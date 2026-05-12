#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define THRESHOLD 10000

int* array;
int* temp_array;

typedef struct {
    int left;
    int right;
} SortParams;

int compare(const void* a, const void* b) {
    return (*(int*) a - *(int*) b);
}

void merge(int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (array[i] <= array[j])
            temp_array[k++] = array[i++];
        else
            temp_array[k++] = array[j++];
    }

    while (i <= mid)
        temp_array[k++] = array[i++];

    while (j <= right)
        temp_array[k++] = array[j++];

    for (i = left; i <= right; i++)
        array[i] = temp_array[i];
}

void* parallel_merge_sort(void* arg) {
    SortParams* params = (SortParams*)arg;
    int left = params->left;
    int right = params->right;

    if (right - left <= THRESHOLD) {
        qsort(&array[left], right - left + 1, sizeof(int), compare);
        return NULL;
    }
    int mid = left + (right - left) / 2;
    pthread_t thread1, thread2;
    SortParams params1 = {left, mid};
    SortParams params2 = {mid + 1, right};

    pthread_create(&thread1, NULL, parallel_merge_sort, &params1);
    pthread_create(&thread2, NULL, parallel_merge_sort, &params2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    merge(left, mid, right);
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char* argv[]) {
    int array_size = (argc > 1) ? atoi(argv[1]) : 1000000;

    array = malloc(array_size * sizeof(int));
    temp_array = malloc(array_size * sizeof(int));
    int* array_copy = malloc(array_size * sizeof(int));

    srand(42);

    for (int i = 0; i < array_size; i++) {
        array[i] = rand() % 100000;
        array_copy[i] = array[i];
    }

    double start = get_time();
    SortParams params = {0, array_size - 1};
    parallel_merge_sort(&params);
    double end = get_time();

    double time_parallel = end - start;

    start = get_time();
    qsort(array_copy, array_size, sizeof(int), compare);
    end = get_time();
    double time_qsort = end - start;

    printf("PARALLEL=%.6f\n", time_parallel);
    printf("QSORT=%.6f\n", time_qsort);

    free(array);
    free(temp_array);
    free(array_copy);
    return 0;
}
