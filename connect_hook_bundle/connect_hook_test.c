
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "fishhook.h"

int (*original_connect)(int, const struct sockaddr *, socklen_t);

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printf("[+] my_connect called\n");

    struct sockaddr_in *new_addr = (struct sockaddr_in *)addr;
    new_addr->sin_family = AF_INET;
    new_addr->sin_port = htons(7890);
    inet_pton(AF_INET, "192.168.3.16", &(new_addr->sin_addr));

    return original_connect(sockfd, (struct sockaddr *)new_addr, addrlen);
}

__attribute__((constructor))
void init_hook() {
    struct rebinding rb;
    rb.name = "connect";
    rb.replacement = my_connect;
    rb.replaced = (void *)&original_connect;
    rebind_symbols(&rb, 1);
}
