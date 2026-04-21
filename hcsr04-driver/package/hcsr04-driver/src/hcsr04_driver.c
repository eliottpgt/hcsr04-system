#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>

struct hcsr04_data {
    struct gpio_desc *trigger;
    struct gpio_desc *echo;
    int irq;
    ktime_t start_time;
    int distance;
    struct completion measurement_done;
    struct miscdevice miscdev;
};

static const struct of_device_id hcsr04_of_match[] = {
    { .compatible = "hcsr04", },
    { }
};
MODULE_DEVICE_TABLE(of, hcsr04_of_match);

static irqreturn_t hcsr04_echo_isr(int irq, void *dev_id) {
    struct hcsr04_data *data = dev_id;
    ktime_t now = ktime_get();

    if (gpiod_get_value(data->echo)) {
        data->start_time = now;
    } else {
        s64 duration = ktime_to_us(ktime_sub(now, data->start_time));
        data->distance = (int)duration / 58;
        complete(&data->measurement_done);
    }
    return IRQ_HANDLED;
}

static ssize_t hcsr04_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    struct hcsr04_data *data = container_of(file->private_data, struct hcsr04_data, miscdev);
    char buffer[16];
    int len;

    if (*ppos > 0) return 0;

    reinit_completion(&data->measurement_done);

    gpiod_set_value(data->trigger, 1);
    udelay(10);
    gpiod_set_value(data->trigger, 0);

    if (!wait_for_completion_timeout(&data->measurement_done, msecs_to_jiffies(200))) {
        return -ETIMEDOUT;
    }

    len = snprintf(buffer, sizeof(buffer), "%d\n", data->distance);

    if (copy_to_user(user_buf, buffer, len)) return -EFAULT;

    *ppos += len;
    return len;
}

static const struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE,
    .read = hcsr04_read,
};

static int hcsr04_probe(struct platform_device *pdev) {
    struct hcsr04_data *data;
    struct device *dev = &pdev->dev;
    int ret;

    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data) return -ENOMEM;

    data->trigger = devm_gpiod_get(dev, "trigger", GPIOD_OUT_LOW);
    data->echo = devm_gpiod_get(dev, "echo", GPIOD_IN);

    init_completion(&data->measurement_done);

    data->irq = gpiod_to_irq(data->echo);

    ret = devm_request_irq(dev, data->irq, hcsr04_echo_isr,
                           IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                           "hcsr04_echo", data);
    if (ret) return ret;

    data->miscdev.minor = MISC_DYNAMIC_MINOR;
    data->miscdev.name = "hcsr04";
    data->miscdev.fops = &hcsr04_fops;
    data->miscdev.parent = dev;

    platform_set_drvdata(pdev, data);

    return misc_register(&data->miscdev);
}

static int hcsr04_remove(struct platform_device *pdev) {
    struct hcsr04_data *data = platform_get_drvdata(pdev);
    misc_deregister(&data->miscdev);
    return 0;
}

static struct platform_driver hcsr04_driver = {
    .driver = { 
        .name = "hcsr04",
        .of_match_table = hcsr04_of_match,
    },
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
};

module_platform_driver(hcsr04_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eliott");
MODULE_DESCRIPTION("Loadable kernel module for HCSR04 module");