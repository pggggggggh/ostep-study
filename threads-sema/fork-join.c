#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

// Using semaphore to enforce the execution order between the parent and child threads

sem_t s;

void *child(void *arg) {
    printf("child\n");
    sem_post(&s);
    sleep(1); // has no effect
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    printf("parent: begin\n");
    sem_init(&s, 0, 0);
    Pthread_create(&p, NULL, child, NULL);
    sem_wait(&s);
    printf("parent: end\n");
    return 0;
}

