#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10
#define PORT 8080

/* 
    服务端使用 socket、bind、listen、accept 系统调用与客户端建立连接，并使用 epoll 来处理 IO 事件
*/
int main() {
    int server_fd, client_fd;
    struct socketaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET
    address.sin_addr.s_addr = INADDR_ANY
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event)

    while(1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) { // 传入连接请求
                client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)
                // 设置客户端 socket 为非阻塞并添加到 epoll 监听列表中
                event.events = EPOLLIN | EPOLLET; // 边缘触发每次就绪就通知
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event)
            } else { // fd 上用户请求数据到达
                read(events[i].data.fd, buffer, 1024);
                printf("Message from client: %s\n", buffer);
                send(events[i].data.fd, "Hello from server", 17, 0)
                close(events[i].data.fd)
            }
        }
    }

    return 0
}