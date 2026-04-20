#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define SENSOR_PATH "/dev/hcsr04-driver"
#define BUFFER_SIZE 128


int main() {

    // Create a network socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = { .sin_family = AF_INET, .sin_port = htons(8080), .sin_addr.s_addr = INADDR_ANY };
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 1);

    int new_socket = accept(server_fd, NULL, NULL);
    
    // Open and read the HCSR04 sensor device driver in read-only mode
    int fd = open("/dev/hcsr04-driver", O_RDONLY);
    char data[128];
    read(fd, data, 128);
    send(new_socket, data, strlen(data), 0);
    // Close and exit
    return 0;
}