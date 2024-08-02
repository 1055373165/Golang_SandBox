#include "csapp.h"

/*
    基于进程的并发 echo 服务器，父进程派生一个子进程来处理每个新的连接请求
*/

void sigchld_handler(int sig)
{
    while( waitpid(-1, 0, WNOHANG)  > 0)
        ;
    return;
}
/*
    通常服务器会运行很长时间，必须有一个 SIGCHLD处理程序来回收僵尸子进程的资源；
    因为当 SIGCHLD 处理程序执行时，SIGCHLD 信号是阻塞的，而 Linux 是是不排队的，所以 SIGCHLD 处理程序需要准备好回收多个僵尸子进程的资源。
*/

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    Signal(SIGCHLD, sigchld_handler);
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (Fork() == 0) {
            Close(listenfd); /* Child closes its listening socket */
            echo(connfd);    /* child services client */
            Close(connfd);   /* child closes connection with client */
            exit(0);         /* child exits*/
        }
        Close(connfd);       /* Parent closes connected socket (important) */
    }
} 