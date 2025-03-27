#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

#define BLOCK_SIZE 64 // Tamaño del bloque para optimización de caché

// Asigna memoria para una matriz
int **allocate_matrix(int n)
{
    int **matrix = (int **)malloc(n * sizeof(int *));
    if (matrix == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)malloc(n * sizeof(int));
        if (matrix[i] == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

// Libera memoria asignada
void free_matrix(int **matrix, int n)
{
    for (int i = 0; i < n; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

// Llena la matriz con valores aleatorios entre 0 y 9
void fill_matrix(int **matrix, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = rand() % 10;
        }
    }
}

// Multiplicación de matrices con Cache Blocking
void multiply_matrices_blocked(int **A, int **B, int **C, int n)
{
    for (int ii = 0; ii < n; ii += BLOCK_SIZE)
    {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE)
        {
            for (int kk = 0; kk < n; kk += BLOCK_SIZE)
            {
                for (int i = ii; i < ii + BLOCK_SIZE && i < n; i++)
                {
                    for (int k = kk; k < kk + BLOCK_SIZE && k < n; k++)
                    {
                        int temp = A[i][k];
                        for (int j = jj; j < jj + BLOCK_SIZE && j < n; j++)
                        {
                            C[i][j] += temp * B[k][j];
                        }
                    }
                }
            }
        }
    }
}

// Obtiene el tiempo de CPU en segundos
double get_user_time()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <matrix_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        fprintf(stderr, "Invalid matrix size. Must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL)); // Inicializa la semilla para valores aleatorios

    int **A = allocate_matrix(n);
    int **B = allocate_matrix(n);
    int **C = allocate_matrix(n);

    fill_matrix(A, n);
    fill_matrix(B, n);

    struct timespec start, end;

    // --- Medir tiempo antes de la ejecución ---
    clock_gettime(CLOCK_MONOTONIC, &start);
    double cpu_start_time = get_user_time();

    multiply_matrices_blocked(A, B, C, n);

    double cpu_end_time = get_user_time();
    clock_gettime(CLOCK_MONOTONIC, &end);

    // --- Calcular tiempos ---
    double wall_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double cpu_time = cpu_end_time - cpu_start_time;

    // --- Imprimir resultados en la terminal ---
    printf("Matrix size: %d x %d\n", n, n);
    printf("User CPU time: %f seconds\n", cpu_time);
    printf("Total execution time (wall-clock): %f seconds\n", wall_time);

    // --- Guardar el tiempo en un archivo (sin sobrescribir) ---
    FILE *file = fopen("resultados.txt", "a"); // "a" agrega en lugar de sobrescribir
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file for writing.\n");
    }
    else
    {
        fprintf(file, "%f\n", wall_time);
        fclose(file);
    }

    free_matrix(A, n);
    free_matrix(B, n);
    free_matrix(C, n);

    return 0;
}
