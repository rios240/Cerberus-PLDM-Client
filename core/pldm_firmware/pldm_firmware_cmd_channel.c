#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "pldm_firmware_cmd_channel.h"

int send_packet(struct cmd_channel *channel, struct cmd_packet *packet) {
    const int server_port = 12354;
    const char *address = "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return CMD_CHANNEL_SOC_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    if (inet_pton(AF_INET, address, &(server.sin_addr)) <= 0) {
        close(sock);
        return CMD_CHANNEL_SOC_NET_ADDRESS_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_CONNECT_FAILURE;
    }


    if (send(sock, packet->data, packet->pkt_size, 0) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }
    
    close(sock);

    return 0;
}

int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    const int server_port = 12354;
    const char *address = "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return CMD_CHANNEL_SOC_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    if (inet_pton(AF_INET, address, &(server.sin_addr)) <= 0) {
        close(sock);
        return CMD_CHANNEL_SOC_NET_ADDRESS_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_CONNECT_FAILURE;
    }

    ssize_t bytes = recv(sock, packet->data, MCTP_BASE_PROTOCOL_MAX_PACKET_LEN, 0);

    if (bytes < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }

    packet->pkt_size = bytes;
    packet->dest_addr = 0x1E;

    close(sock);

    return 0;
}