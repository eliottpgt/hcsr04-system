#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

/**
 * struct hcsr04_data - Private data structure for the HC-SR04 sensor
 * @trigger: GPIO descriptor for the trigger pin
 * @echo: GPIO descriptor for the echo pin
 * @irq: Interrupt line associated with the echo pin
 * @start_time: Timestamp of the rising edge of the echo signal
 * @distance: Calculated distance in centimeters
 * @measurement_done: Completion signal to synchronize read() and ISR
 * @miscdev: Misc device structure for user-space access
 * @lock: Mutex to prevent concurrent sensor triggers
 */
struct hcsr04_data {
    struct gpio_desc *trigger;
    struct gpio_desc *echo;
    int irq;
    ktime_t start_time;
    int distance;
    struct completion measurement_done;
    struct miscdevice miscdev;
    struct mutex lock;
};

/* Device Tree matching table */
static const struct of_device_id hcsr04_of_match[] = {
    { .compatible = "hcsr04", },
    { } /* Sentinel */
};
MODULE_DEVICE_TABLE(of, hcsr04_of_match);

/**
 * hcsr04_echo_isr() - Interrupt Service Routine for the echo pin
 * @irq: The interrupt number
 * @dev_id: Pointer to our private hcsr04_data structure
 * * Triggered on both rising and falling edges. Measures the pulse width
 * of the echo signal to calculate distance.
 */
static irqreturn_t hcsr04_echo_isr(int irq, void *dev_id) {
    struct hcsr04_data *data = dev_id;
    ktime_t now = ktime_get();

    if (gpiod_get_value(data->echo)) {
        /* Rising edge: start of the pulse */
        data->start_time = now;
    } else {
        /* Falling edge: end of the pulse, calculate distance */
        s64 duration = ktime_to_us(ktime_sub(now, data->start_time));
        /* Distance (cm) = (time [us] * speed of sound [0.034 cm/us]) / 2 */
        data->distance = (int)duration / 58;
        complete(&data->measurement_done);
    }
    return IRQ_HANDLED;
}

/**
 * hcsr04_read() - User-space read interface
 * * Triggers a 10us pulse, waits for the ISR to complete the measurement,
 * and returns the distance as a string.
 */
static ssize_t hcsr04_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    struct hcsr04_data *data = container_of(file->private_data, struct hcsr04_data, miscdev);
    char buffer[16];
    int len;
    long timeout;

    /* Handle EOF */
    if (*ppos > 0) return 0;

    /* Ensure only one process uses the sensor at a time */
    if (mutex_lock_interruptible(&data->lock))
        return -ERESTARTSYS;

    reinit_completion(&data->measurement_done);

    /* Generate 10us trigger pulse */
    gpiod_set_value(data->trigger, 1);
    udelay(10);
    gpiod_set_value(data->trigger, 0);

    /* Wait for ISR to finish (timeout after 50ms) */
    timeout = wait_for_completion_timeout(&data->measurement_done, msecs_to_jiffies(50));

    if (timeout == 0) {
        mutex_unlock(&data->lock);
        return -ETIMEDOUT;
    }

    len = snprintf(buffer, sizeof(buffer), "%d\n", data->distance);

    /* Transfer result to user space */
    if (copy_to_user(user_buf, buffer, len)) {
        mutex_unlock(&data->lock);
        return -EFAULT;
    }

    *ppos += len;
    mutex_unlock(&data->lock);
    return len;
}

/* Standard VFS interface mapping system calls to driver functions */
static const struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE, /* Track module usage count for safe unloading */
    .read = hcsr04_read,   /* Link read() syscall to sensor logic */
};


/**
 * hcsr04_probe() - Initializes the sensor when a match is found
 * @pdev: Platform device pointer
 */
static int hcsr04_probe(struct platform_device *pdev) {
    struct hcsr04_data *data;
    struct device *dev = &pdev->dev;
    int ret;

    /* Allocate managed memory for the device state */
    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data) return -ENOMEM;

    /* Request GPIOs defined in the Device Tree */
    data->trigger = devm_gpiod_get(dev, "trigger", GPIOD_OUT_LOW);
    if (IS_ERR(data->trigger)) return PTR_ERR(data->trigger);

    data->echo = devm_gpiod_get(dev, "echo", GPIOD_IN);
    if (IS_ERR(data->echo)) return PTR_ERR(data->echo);

    /* Initialize synchronization primitives */
    init_completion(&data->measurement_done);
    mutex_init(&data->lock);

    /* Map GPIO to IRQ and request the interrupt */
    data->irq = gpiod_to_irq(data->echo);
    ret = devm_request_irq(dev, data->irq, hcsr04_echo_isr,
                           IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                           "hcsr04_echo", data);
    if (ret) {
        dev_err(dev, "Failed to request IRQ %d\n", data->irq);
        return ret;
    }

    /* Configure the misc device interface */
    data->miscdev.minor = MISC_DYNAMIC_MINOR;
    data->miscdev.name = "hcsr04";
    data->miscdev.fops = &hcsr04_fops;
    data->miscdev.parent = dev;

    /* Store private data for use in other functions (like remove) */
    platform_set_drvdata(pdev, data);

    /* Final step: Register the character device */
    ret = misc_register(&data->miscdev);
    if (ret) {
        dev_err(dev, "Failed to register misc device\n");
        return ret;
    }

    /* Success message! */
    dev_info(dev, "HC-SR04 Driver loaded. Access via /dev/%s\n", data->miscdev.name);

    return 0;
}

/**
 * hcsr04_remove() - Cleanup when the driver is removed
 */
static int hcsr04_remove(struct platform_device *pdev) {
    struct hcsr04_data *data = platform_get_drvdata(pdev);
    misc_deregister(&data->miscdev);
    return 0;
}

/* Definition of the platform driver structure */
static struct platform_driver hcsr04_driver = {
    .driver = { 
        .name = "hcsr04",                   /* Driver name used for matching without Device Tree */
        .of_match_table = hcsr04_of_match,
    },
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
};

/* Register the platform driver */
module_platform_driver(hcsr04_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eliott");
MODULE_DESCRIPTION("Loadable kernel module for HC-SR04 ultrasonic sensor");