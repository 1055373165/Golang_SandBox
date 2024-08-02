/*
* psum-local.c - A simple parallel sum program where each thread sums into a local variable which it then copies to a distinct array element.
*/

#include "csapp.h"
#define MAXTHREADS 32

void *sum_local(void *vargp); /* Thread routine */

/* Global shared variables */
long psum[MAXTHREADS];      /* Partial sum computed by each thread */
long nelems_per_thread;     /* Number of elements summed by each thread */  

int main(int argc, char **argv)
{
    long i, nelems, log_nelems, nthreads, myid[MAXTHREADS], result = 0;
    pthread_t tid[MAXTHREADS];

    /* Get input argument */
    if (argc != 3) {
        printf("Usage: %dd <nthreads> <log_nelems>\n", argv[0]);
        exit(0);
    }
    nthreads = atoi(argv[1]);
    log_nelems = atio(argv[2]);
    nelems = (1L << log_nelems);

    /* check peer threads and wait for them to finish */
    if ((nelems % nthreads) != 0 || (log_nelems > 31)) {
        printf("Error: invalid nelems\n");
        exit(0);
    }
    nelems_per_thread = nelems / nthreads;

    /* Create peer threads and wait for them to finish */
    for (i = 0; i < nthreads; i++) {
        myid[i] = i;
        Pthread_create(&tid[i], NULL, sum_local, &myid[i]);
    }
    for (i = 0; i < nthreads; i++) {
        Pthread_join(tid[i], NULL);
    }

    /* Added up the partial sums computed by each thread */
    for (i = 0; i < nthreads; i++) 
        result += psum[i];
    
    /* check final answer */
    if (result != (nelems * (nelems-1))/2)
        printf("Error: result=%ld\n", result);

    exit(0);
}

void *sum_local(void *vargp) {
    long myid = *((long *)vargp);
    long start = myid * nelems_per_thread;
    long end = start + nelems_per_thread;
    long i, sum = 0;

    for (i = start; i < end; i++) {
        sum += i;
    }
    psum[myid] = sum;
    return NULL;
}