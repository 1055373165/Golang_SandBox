/*
    基于 io 多路复用的并发编程
*/

#include "csapp.h"

void echo(int connfd);
void command(void);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    // read event fd set
    // ready fd set into read fd set
    fd_set read_set, ready_set;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = Open_listenfd(argv[1]);

    FD_ZERO(&read_set);               /* clear read set */
    FD_SET(STDIN_FILENO, &read_set);  /* add stdin to read set */
    FD_SET(listenfd, &read_set);      /* add listenfd to read set */

    while(1) {
        ready_set = read_set;
        Select(listenfd+1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set))
            command();                /* Read command line from stdin */
        if (FD_ISSET(listenfd, &ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            echo(connfd);             /* echo client input until EOF */
            Close(connfd);
        }
    }
}

void command(void) {
    char buf[MAXLINE];
    if (!Fgets(buf, MAXLINE, stdin))
        exit(0);       /* EOF */
    printf("%s", buf); /* Process the input command */
}

/*
    使用 select 函数，要求内核挂起进程，只有在一个或者多个 IO 事件发生后，才将控制返回给应用程序。
    select 函数处理类型为 fd_set 集合，也叫做描述符集合。逻辑上可以将描述符集合看做一个大小为 n 的位向量。
    可以使用 FD_ZERO、FD_SET、FD_CLR 和 FD_ISSET  宏来修改和检查它们。
    在设计时，select 函数有两个输入，一个时读集合的描述符集合（fdset）和该读集合中的描述符最大的序号。
    select 会一直阻塞直到读集合中至少有一个描述符住备好可以读（当且仅当从该描述符读取一个字节的请求不会阻塞时）。

    select 函数有一个副作用：它修改参数 fdset 指向的 fd_set，指明读集合的一个子集，称为准备好集合（ready set），这个集合是由读集合中准备好可以读的描述符组成的。

    一旦 select 返回，可以使用 FD_ISSET 宏指令来确定哪个描述符准备好可以读了。
    - 如果是标准输入准备好了，就调用 command 函数，该函数在返回到主程序前，会读、解析和响应命令。
    - 如果是监听描述符准备好了，就调用 accept 来得到一个已连接描述符，然后调用 echo 函数，它会将来自客户端的一行内容回送回去，直到客户端主动关闭这个连接才将所有内容返回给客户端。
*/