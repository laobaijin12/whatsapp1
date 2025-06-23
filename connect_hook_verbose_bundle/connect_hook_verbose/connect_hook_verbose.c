#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "fishhook.h"

int (*original_connect)(int, const struct sockaddr *, socklen_t);

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    struct sockaddr_in *target = (struct sockaddr_in *)addr;
    char target_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(target->sin_addr), target_ip, INET_ADDRSTRLEN);
    int target_port = ntohs(target->sin_port);

    printf("[HOOK] Intercepted connect() to %s:%d\n", target_ip, target_port);
    printf("[HOOK] Original Target => %s:%d\n", target_ip, target_port);

    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(7890);
    inet_pton(AF_INET, "192.168.3.16", &proxy_addr.sin_addr);

    printf("[HOOK] Redirecting to Proxy => 192.168.3.16:7890\n");

    int result = original_connect(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr));
    if (result == 0) {
        printf("[HOOK] Proxy connect() success.\n");
    } else {
        printf("[HOOK] Proxy connect() failed!\n");
    }

    return result;
}

__attribute__((constructor))
void init_hook() {
    printf("[HOOK] Installing connect() hook...\n");
    struct rebinding bind = {
        .name = "connect",
        .replacement = my_connect,
        .replaced = (void *)&original_connect,
    };
    rebind_symbols(&bind, 1);
}
