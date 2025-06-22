#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "fishhook.h"

#define LOCAL_SOCKS5_PORT 1083

static int (*original_connect)(int, const struct sockaddr *, socklen_t) = NULL;

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    struct sockaddr_in *target = (struct sockaddr_in *)addr;
    char target_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(target->sin_addr), target_ip, INET_ADDRSTRLEN);
    int target_port = ntohs(target->sin_port);

    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(LOCAL_SOCKS5_PORT);
    inet_pton(AF_INET, "127.0.0.1", &proxy_addr.sin_addr);

    int ret = original_connect(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr));
    if (ret != 0) {
        printf("[hook] Failed to connect to socks5 proxy: %s\n", strerror(errno));
        return ret;
    }

    unsigned char handshake_req[] = {0x05, 0x01, 0x00};
    send(sockfd, handshake_req, sizeof(handshake_req), 0);

    unsigned char handshake_res[2];
    recv(sockfd, handshake_res, 2, 0);
    if (handshake_res[1] != 0x00) {
        printf("[hook] socks5 handshake failed\n");
        return -1;
    }

    unsigned char connect_req[10];
    connect_req[0] = 0x05;
    connect_req[1] = 0x01;
    connect_req[2] = 0x00;
    connect_req[3] = 0x01;

    memcpy(connect_req + 4, &(target->sin_addr), 4);
    connect_req[8] = (target->sin_port >> 8) & 0xff;
    connect_req[9] = target->sin_port & 0xff;

    send(sockfd, connect_req, 10, 0);

    unsigned char connect_res[10];
    recv(sockfd, connect_res, 10, 0);
    if (connect_res[1] != 0x00) {
        printf("[hook] socks5 connect failed, code: %02x\n", connect_res[1]);
        return -1;
    }

    printf("[hook] CONNECT OK -> %s:%d via 127.0.0.1:%d\n", target_ip, target_port, LOCAL_SOCKS5_PORT);
    return 0;
}

__attribute__((constructor))
void install_hook() {
    printf("[hook] Installing connect() hook...\n");
    struct rebinding bind = {
        .name = "connect",
        .replacement = my_connect,
        .replaced = (void *)&original_connect,
    };
    rebind_symbols(&bind, 1);
}
