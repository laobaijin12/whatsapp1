#import <UIKit/UIKit.h>
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

    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(7890);
    inet_pton(AF_INET, "192.168.3.16", &proxy_addr.sin_addr);

    printf("[HOOK] Redirecting to Proxy => 192.168.3.16:7890\n");

    return original_connect(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr));
}

__attribute__((constructor))
static void init_hook_with_alert() {
    printf("[HOOK] Installing connect() hook...\n");

    // install hook
    struct rebinding bind = {
        .name = "connect",
        .replacement = my_connect,
        .replaced = (void *)&original_connect,
    };
    rebind_symbols(&bind, 1);

    // show alert on main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"✅ Hook 成功"
            message:@"已劫持 connect() 到 192.168.3.16:7890"
            preferredStyle:UIAlertControllerStyleAlert];

        UIAlertAction *ok = [UIAlertAction actionWithTitle:@"好的" style:UIAlertActionStyleDefault handler:nil];
        [alert addAction:ok];

        UIWindow *keyWindow = [UIApplication sharedApplication].keyWindow;
        [keyWindow.rootViewController presentViewController:alert animated:YES completion:nil];
    });
}
