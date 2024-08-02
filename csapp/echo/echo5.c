#include "csapp.h"

/*
    Posix 线程（Pthreads）是在 c 程序中处理线程的一个标准接口。
    Pthreads 定义了大约 60 个函数，允许程序创建、杀死和回收线程与对等线程（peer thread）安全的共享数据，还可以通知对等线程系统状态的变化。
    基于线程的并发服务器；
    主线程不断等待连接请求，然后创建一个对等线程处理该请求。
*/
void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = Open_listenfd(argv[1]);

    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        // 为新连接创建一个新的已连接描述符
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* Thread routine */
void *thread(void *vargp)
{
    // connfd 申请完成后赋值给了局部变量 vargp
    int connfd = *((int *)vargp);
    // 从 joinable -> detach，用以回收 thread 资源
    Pthread_detach(pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}

/*
    线程是 joinable 或者是分离的（detached）；
    一个可结合的线程能够被其他线程回收和杀死，在被其他线程回收之前，它的内存资源是不释放的。
    一个分离的线程是不能被其他线程回收或者杀死的，它的内存资源在它终止时默认由操作系统内核自动释放。
    在基于线程的并发服务器场景下，为每一个请求创建一个线程（单个逻辑流）处理，而创建出的线程默认为 joinable，也就是需要主线程负责回收；
    一个高性能 web 服务器可能在每次收到 web 浏览器的连接请求时都创建一个新的对等线程。因为每个连接都是由一个单独的线程独立处理的，所以对于服务器而言，
    很没有必要显示地等待每个对等线程终止。在这种情况下，每个对等线程都应该在它开始处理请求之前分离它自身，这样就能在它终止后回收它的内存资源了。
*/

/*
    当调用 pthread_create 时，如何将已连接描述符传递给对等线程呢？
    最明显的方法就是传递一个指向这个描述符的指针；
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    Pthread_create(&tid, NULL, thread, &connfd);
    然后让对等线程间接引用这个指针，并将它赋值给一个局部变量
    void *thread(void *vargp) {
        int connfd = *((int *)vargp);
        ...
    }
    然而这样可能会出现 race，如果赋值语句在下一个 accept 之前完成，那么对等线程中的局部变量 connfd 就得到正确的描述符值；
    然而如果赋值语句在 accept 之后完成，那么对等线程中的局部变量 connfd 就得到下一次连接的描述符值。
    这意味着两个线程在同一个描述符上执行输入和输出，为了避免这种潜在的致命竞争，必须将 accept 返回的每个已连接描述符分配到它自己的动态分配的内存块。
*/

/*

*/