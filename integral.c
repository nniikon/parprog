#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_THREADS 8
#define CHUNK_SIZE 0.001
#define EPSILON 1e-7

double global_a = 0.01;
double global_b = 4.0;
double current_x;
double total_integral = 0.0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int thread_id;
    double execution_time;
    int chunks_processed;
} ThreadStats;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

double f(double x) {
    return sin(1.0 / x);
}

double adaptive_trapezoid(double a, double b, double eps) {
    double h = b - a;
    double mid = (a + b) / 2.0;
    double area_whole = (f(a) + f(b)) * h / 2.0;
    double area_split = (f(a) + f(mid)) * (h / 2.0) / 2.0 + (f(mid) + f(b)) * (h / 2.0) / 2.0;

    if (fabs(area_whole - area_split) <= 3.0 * eps) {
        return area_split;
    }
    return adaptive_trapezoid(a, mid, eps / 2.0) + adaptive_trapezoid(mid, b, eps / 2.0);
}

void* worker(void* arg) {
    ThreadStats* stats = (ThreadStats*)arg;
    double local_sum = 0.0;

    double start_time = get_time();

    while (1) {
        double start, end;

        pthread_mutex_lock(&mutex);
        if (current_x >= global_b) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        start = current_x;
        current_x += CHUNK_SIZE;
        end = (current_x > global_b) ? global_b : current_x;
        pthread_mutex_unlock(&mutex);

        local_sum += adaptive_trapezoid(start, end, EPSILON);
        stats->chunks_processed++;
    }

    stats->execution_time = get_time() - start_time;

    pthread_mutex_lock(&mutex);
    total_integral += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadStats stats[NUM_THREADS];
    current_x = global_a;

    for (int i = 0; i < NUM_THREADS; i++) {
        stats[i].thread_id = i;
        stats[i].chunks_processed = 0;
        pthread_create(&threads[i], NULL, worker, &stats[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Интеграл sin(1/x) от %.3f до %.3f равен: %.8f\n\n", global_a, global_b, total_integral);

    printf("Статистика балансировки потоков (Work Stealing)\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Поток [%d]: Время работы = %.5f сек | Обработано кусков = %d\n",
               stats[i].thread_id,
               stats[i].execution_time,
               stats[i].chunks_processed);
    }

    return 0;
}
