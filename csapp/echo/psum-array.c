/*
* psum-array.c - A simple parallel sum program where each thread sums into its own distinct global array element.
*/

#include "csapp.h"
#define MAXTHREADS 32

void *sum_array(void *vargp);  /* Thread routine */

/* Global shared vairables */
long psum[MAXTHREADS];  /* Partial sum computed by each thread */
long nelems_per_thread; /* Number of elements summed by each thread */

int main(int argc, char **argv)
{
    long i, nelems, log_nelems, nthreads, myid[MAXTHREADS], result = 0;
    pthread_t tid[MAXTHREADS];

    /* Get input arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <nthreads><log_nelems>\n", argv[0]);
        exit(0);
    }
    nthreads = atoi(argv[1]);
    log_nelems = atoi(argv[2]);
    nelems = (1L << log_nelems);

    /* check input arguments */
    if ((nelems % nthreads) != 0 || (log_nelems > 31)) {
        printf("Error: invalid nelems\n");
        exit(0);
    }
    nelems_per_thread = nelems / nthreads;

    /* Create peer threads and wait for them to finish */
    for (i = 0; i < nthreads; i++) {
        myid[i] = i;
        Pthread_create(&tid[i], NULL, sum_array, &myid[i]);
    }
    for (i = 0; i < nthreads; i++) {
        Pthread_join(tid[i], NULL);
    }
    /* Add up the partial sums computed by each thread */
    for (i = 0; i < nthreads; i++) {
        result += psum[i];
    }

    /* check final answer */
    if (result != (nelems * (nelems-1))/2) 
        printf("Error: result=%ld\n", result);

    exit(0);
}

// worker thread 0 : 0-10
// worker thread 1 : 11-20
// ...
void *sum_array(void *vargp)
{
    long myid = *((long *)vargp);
    long start = myid * nelems_per_thread;
    long end = start + nelems_per_thread;
    long i;

    for (i=start; i < end; i++) {
        psum[myid] += i;
    }
    return NULL;
}