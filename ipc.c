#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/wait.h>

int iterations;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

void test_pipes() {
    int fd1[2];
    int fd2[2];

    pipe(fd1);
    pipe(fd2);

    pid_t pid = fork();
    if (pid == 0) {
        char msg;
        for (int i = 0; i < iterations; i++) {
            read(fd1[0], &msg, 1);
            write(fd2[1], "A", 1);
        }
        exit(0);
    } else {
        char msg;
        double start = get_time();
        for (int i = 0; i < iterations; i++) {
            write(fd1[1], "A", 1);
            read(fd2[0], &msg, 1);
        }
        double end = get_time();
        wait(NULL);
        printf("PIPES=%.6f\n", end - start);
    }
}

pthread_mutex_t shm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_t1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_t2 = PTHREAD_COND_INITIALIZER;
int shared_flag = 0;

void* thread2_func(void* arg) {
    for (int i = 0; i < iterations; i++) {
        pthread_mutex_lock(&shm_mutex);

        while (shared_flag != 1)
            pthread_cond_wait(&cond_t2, &shm_mutex);

        shared_flag = 0;

        pthread_cond_signal(&cond_t1);

        pthread_mutex_unlock(&shm_mutex);
    }
    return NULL;
}

void test_threads_shm() {
    pthread_t t2;
    pthread_create(&t2, NULL, thread2_func, NULL);

    double start = get_time();

    for (int i = 0; i < iterations; i++) {
        pthread_mutex_lock(&shm_mutex);
        shared_flag = 1;
        pthread_cond_signal(&cond_t2);
        while (shared_flag != 0) pthread_cond_wait(&cond_t1, &shm_mutex);
        pthread_mutex_unlock(&shm_mutex);
    }

    double end = get_time();

    pthread_join(t2, NULL);
    printf("SHM=%.6f\n", end - start);
}

int main(int argc, char* argv[]) {
    iterations = (argc > 1) ? atoi(argv[1]) : 100000;
    test_pipes();
    test_threads_shm();
    return 0;
}
