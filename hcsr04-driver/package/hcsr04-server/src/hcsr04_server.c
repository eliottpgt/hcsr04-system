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

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    
    // Setup Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind and Listen
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server listening on port %d...\n", PORT);

    // Main Loop: keep the server alive for new clients
    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (new_socket >= 0) {
            // Read sensor data
            int fd_sensor = open(SENSOR_PATH, O_RDONLY);
            if (fd_sensor >= 0) {
                memset(buffer, 0, BUFFER_SIZE);
                read(fd_sensor, buffer, BUFFER_SIZE);
                
                // Send data and close connection immediately
                send(new_socket, buffer, strlen(buffer), 0);
                close(fd_sensor);
            }
            close(new_socket); 
        }
    }

    close(server_fd);
    return 0;
}