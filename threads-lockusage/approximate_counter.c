// Counter designed for concurrency with approximation

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>
#define NUMCPUS 5
#define NUMTHREADS 50

typedef struct __counter_t {
    int global;
    pthread_mutex_t glock;
    int local[NUMCPUS];
    pthread_mutex_t llock[NUMCPUS];
    int threshold;
} counter_t;

void init(counter_t *c, int threshold) {
    c->global = 0;
    c->threshold = threshold;
    assert(pthread_mutex_init(&c->glock,NULL) == 0);

    for (int i=0;i<NUMCPUS;i++) {
        c->local[i] = 0;
        assert(pthread_mutex_init(&c->llock[i],NULL) == 0);
    }
}

void incre(counter_t *c, int threadID) {
    int cpu = threadID % NUMCPUS;
    pthread_mutex_lock(&c->llock[cpu]);
    c->local[cpu]++;
    if (c->local[cpu] >= c->threshold) {
        pthread_mutex_lock(&c->glock);
        c->global += c->local[cpu];
        pthread_mutex_unlock(&c->glock);
        c->local[cpu] = 0;
    }
    pthread_mutex_unlock(&c->llock[cpu]);
}

int get(counter_t *c) {
    return c->global;
}

typedef struct __thread_arg_t {
    counter_t *c;
    int threadID;
} thread_arg_t;

void* thread_func(void *arg)
{
    thread_arg_t* myarg = arg;
    counter_t *c = myarg->c;
    int threadID = myarg->threadID;

    int i;
    for (i = 0; i < 1000000; i++) incre(c, threadID);
    return NULL;
}

int main(int argc, char *argv[])
{
    struct timeval start,end;
    counter_t *c = (counter_t *) malloc(sizeof(counter_t));
    init(c,1000);
    pthread_t p[NUMTHREADS];

    assert(gettimeofday(&start, NULL) == 0);

    for (int i=0;i<NUMTHREADS;i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->c = c;
        arg->threadID = i;
        pthread_create(p + i, NULL, thread_func, arg);
    }

    for (int i=0;i<NUMTHREADS;i++) {
        pthread_join(p[i], NULL);
    }

    assert(gettimeofday(&end, NULL) == 0);

    double millis = (end.tv_sec - start.tv_sec) * 1000 +
                    (end.tv_usec - start.tv_usec) / 1000.0;
    printf("%.3f milliseconds\n", millis);
    printf("value : %d", get(c));

    return 0;
}