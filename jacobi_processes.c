#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>

#define MAX_ITERS 1000
#define TOL 1e-6
#define NUM_PROCESSES 12

int N;
double **A, **B;

typedef struct
{
    int start, end;
} ProcessData;

void jacobi_process(ProcessData data)
{
    int i, j;
    for (i = data.start; i < data.end; i++)
    {
        for (j = 1; j < N - 1; j++)
        {
            B[i][j] = 0.25 * (A[i - 1][j] + A[i + 1][j] + A[i][j - 1] + A[i][j + 1]);
        }
    }
    exit(0);
}

void jacobi_parallel()
{
    int iter, i;
    pid_t pids[NUM_PROCESSES];

    for (iter = 0; iter < MAX_ITERS; iter++)
    { // Se ejecutarán siempre 1000 iteraciones
        int chunk_size = (N - 2) / NUM_PROCESSES;

        for (i = 0; i < NUM_PROCESSES; i++)
        {
            ProcessData data = {1 + i * chunk_size, (i == NUM_PROCESSES - 1) ? (N - 1) : (1 + (i + 1) * chunk_size)};
            pids[i] = fork();
            if (pids[i] == 0)
            {
                jacobi_process(data);
            }
        }

        for (i = 0; i < NUM_PROCESSES; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
    }
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
    if (argc < 2)
    {
        printf("Uso: %s <tamaño_matriz>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);

    initialize_matrix();
    jacobi_parallel();

    printf("Jacobi con %d procesos completado en %d iteraciones.\n", NUM_PROCESSES, MAX_ITERS);

    for (int i = 0; i < N; i++)
    {
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    return 0;
}
