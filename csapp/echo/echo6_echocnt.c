#include "csapp.h"

static int byte_cnt; /* byte counter */
static sem_t mutex; /* and the mutex that protects it */

static void init_echo_cnt(void)
{
    Sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void echo_cnt(int connfd)
{
    int n;
    char buf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    Pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        P(&mutex);          // 加互斥锁
        byte_cnt += n;      // 临界区数据更新
        printf("server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
        V(&mutex); 
        Rio_writen(connfd, buf, n);
    }
}