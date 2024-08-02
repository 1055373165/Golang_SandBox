#include "csapp.h"
#define MAXTHREADS 32

void *sum_mutex(void *vargp); /* thread routine */
/* Global shared variables */
long gsum = 0;                /* Global sum */
long nelems_per_thread;       /* Number of elements to sum */
sem_t mutex;                  /* Mutex to protect global sum */

int main(int argc, char **argv)
{
    // myid 将为每个 worker thread 分配一个唯一的 id
    // 每个唯一 id 分一部分任务序列进行计算（加锁更新全局变量)
    long i, nelems, log_nelems, nthreads, myid[MAXTHREADS];
    pthread_t tid[MAXTHREADS];

    /* Get input arguments */
    if (argc != 3) {
        printf("Usage: %s <nthreads> <log_nelems>\n", argv[0]);
        exit(0);
    }
    nthreads = atoi(argv[1]);
    log_nelems = atio(argv[2]);
    // 1L << log_nelems 需要计算数字的总量
    nelems = (1L << log_nelems);
    // 平均每个 worker 分多少个数字进行计算
    nelems_per_thread = nelems / nthreads;
    sem_init(&mutex, 0, 1);

    /* Create peer threads and wait for them to finish */
    /* 共享同一个互斥锁、每个 posix thread 分得一个唯一的 id */
    for (i = 0; i < nthreads; i++) {
        myid[i] = i;
        Pthread_create(&tid[i], NULL, sum_mutex, &myid[i]);
    }

    /* 为了保证正确性，主线程等待所有线程的完成后返回结果，因此将所有子线程变为 joinable */
    for (i = 0; i < nthreads; i++)
        Pthread_join(tid[i], NULL);
    
    /* Check final answer */
    if (gsum != (nelems * (nelems-1))/2)
        printf("Error: result=%ld\n", gsum);
    
    exit(0);
}

/* 
    * Thread routine for psum-mutex.c 
    * 同步操作代价太大：同步开销巨大，要尽可能避免；（所有的线程去抢同一个互斥锁）
*/
void *sum_mutex(void *vargp)
{
    long myid = *((long *)vargp);           /* Extract the thread ID */
    long start = myid * nelems_per_thread;  /* Start element index */
    long end = start + nelems_per_thread;   /* End element index */
    long i;

    for (i = start; i < end; i++) {
        P(&mutex);
        gsum += i;
        V(&mutex);
    }
    return NULL;
}

