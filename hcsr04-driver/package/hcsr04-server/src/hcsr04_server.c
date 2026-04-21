#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define SENSOR_PATH "/dev/hcsr04"
#define BUFFER_SIZE 128

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Create a TCP socket 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options (port and type of adress)
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections with a queue limit of 3
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("HC-SR04 Server started on port %d...\n", PORT);

    // Main Server Loop: Keeps the server running to accept new clients
    while(1) {
        printf("Waiting for a connection from client...\n");
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        int flag_connect = 0;
        
        // Persistent Client Loop: Handles multiple requests from the same connected client
        while(1) {
            if(flag_connect == 0){
                printf("Client connected\n");
                flag_connect++;
            }
            
            char req_buffer[1024] = {0};
            // Wait for data or signal from the client
            int valread = read(new_socket, req_buffer, 1024);
            
            // If read returns <= 0, the client has disconnected
            if (valread <= 0) {
                printf("Client disconnected from session\n");
                break;
            }

            // Read data from the HC-SR04 Kernel Module / Device Driver
            int fd_sensor = open(SENSOR_PATH, O_RDONLY);
            if (fd_sensor >= 0) {
                memset(buffer, 0, BUFFER_SIZE);
                read(fd_sensor, buffer, BUFFER_SIZE);
                close(fd_sensor);
                
                // Send the sensor data back to the client
                send(new_socket, buffer, strlen(buffer), 0);
            }
        }

        // Cleanup connection before waiting for the next client
        close(new_socket);
    }
    
    return 0;
}