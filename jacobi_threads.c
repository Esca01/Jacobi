#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define MAX_ITERS 1000
#define TOL 1e-6

int N, NUM_THREADS;
double **A, **B;
int num_iters = 0;
pthread_mutex_t mutex;

typedef struct
{
    int start, end;
} ThreadData;

void *jacobi_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int i, j;
    double local_diff = 0.0;

    for (i = data->start; i < data->end; i++)
    {
        for (j = 1; j < N - 1; j++)
        {
            B[i][j] = 0.25 * (A[i - 1][j] + A[i + 1][j] + A[i][j - 1] + A[i][j + 1]);
        }
    }

    for (i = data->start; i < data->end; i++)
    {
        for (j = 1; j < N - 1; j++)
        {
            local_diff += fabs(A[i][j] - B[i][j]);
            A[i][j] = B[i][j];
        }
    }

    pthread_mutex_lock(&mutex);
    num_iters += local_diff;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void jacobi_parallel()
{
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int i, iter;

    for (iter = 0; iter < MAX_ITERS; iter++)
    { // Se ejecutarán siempre 1000 iteraciones
        int chunk_size = (N - 2) / NUM_THREADS;

        for (i = 0; i < NUM_THREADS; i++)
        {
            thread_data[i].start = 1 + i * chunk_size;
            thread_data[i].end = (i == NUM_THREADS - 1) ? (N - 1) : (thread_data[i].start + chunk_size);
            pthread_create(&threads[i], NULL, jacobi_thread, &thread_data[i]);
        }

        for (i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }

    num_iters = MAX_ITERS; // Asegurar que siempre sean 1000 iteraciones
}

void initialize_matrix()
{
    A = (double **)malloc(N * sizeof(double *));
    B = (double **)malloc(N * sizeof(double *));
    for (int i = 0; i < N; i++)
    {
        A[i] = (double *)malloc(N * sizeof(double));
        B[i] = (double *)malloc(N * sizeof(double));
    }

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            A[i][j] = (i == 0 || i == N - 1 || j == 0 || j == N - 1) ? 1.0 : 0.0;
            B[i][j] = A[i][j];
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Uso: %s <tamaño_matriz> <num_hilos>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    NUM_THREADS = atoi(argv[2]);

    pthread_mutex_init(&mutex, NULL);
    initialize_matrix();
    jacobi_parallel();
    pthread_mutex_destroy(&mutex);

    printf("Jacobi con %d hilos completado en %d iteraciones.\n", NUM_THREADS, num_iters);

    for (int i = 0; i < N; i++)
    {
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    return 0;
}