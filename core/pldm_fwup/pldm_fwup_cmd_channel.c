#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include "mctp/mctp_interface.h"
#include "pldm_fwup_cmd_channel.h"

#define PORT 5000

void print_packet_data(const uint8_t *data, size_t len) {
    printf("Packet data:");
    for (size_t i = 0; i < len; ++i) {
        printf(" %02x", data[i]);
    }
    printf("\n");
}

int set_socket_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (set_socket_non_blocking(server_fd) < 0) {
        perror("set non-blocking failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port 5000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);
    long elapsed_ms;

    do {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        gettimeofday(&now, NULL);
        elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
    } while (new_socket < 0 && errno == EWOULDBLOCK && elapsed_ms < ms_timeout);

    if (new_socket < 0) {
        if (errno == EWOULDBLOCK) {
            printf("Timeout occurred\n");
        } else {
            perror("accept");
        }
        close(server_fd);
        return -1;
    }

    // Read data from the socket
    size_t valread = read(new_socket, packet->data, MCTP_BASE_PROTOCOL_MAX_PACKET_LEN);

    packet->pkt_size = valread;
    packet->dest_addr = 0xDA;

    //printf("Received Packet \n");
    //print_packet_data(packet->data, packet->pkt_size);

    close(new_socket);
    close(server_fd);

    return 0;
}

int send_packet(struct cmd_channel *channel, struct cmd_packet *packet) {
    struct sockaddr_in serv_addr;
    int sock = 0;
    int ms_timeout = 60000;

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);
    long elapsed_ms = 0;
    int connected = 0;

    while (elapsed_ms < ms_timeout && !connected) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
            connected = 1;
        } else {
            close(sock);
        }

        gettimeofday(&now, NULL);
        elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
    }

    if (!connected) {
        printf("Connection timeout\n");
        return -1;
    }

    send(sock, packet->data, packet->pkt_size, 0);
    //printf("Sent Packet \n");
    //print_packet_data(packet->data, packet->pkt_size);
    close(sock);

    return 0;
}



int generate_error_packet(struct cmd_interface *intf, struct cmd_interface_msg *request,
		uint8_t error_code, uint32_t error_data, uint8_t cmd_set)
{
    uint8_t error[2] = {0xDE, 0xAD};
    request->data = (uint8_t *) error;
    request->length = 16;

    printf("MCTP ERROR\n");
    return 0;
}