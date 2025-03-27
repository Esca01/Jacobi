#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>

#define MAX_THREADS 12 // Máximo número de hilos permitidos

typedef struct
{
    int start_row;
    int end_row;
    int n;
    int **A;
    int **B;
    int **C;
} ThreadData;

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

// Función que ejecuta cada hilo
void *multiply_matrices_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int start = data->start_row, end = data->end_row, n = data->n;
    int **A = data->A, **B = data->B, **C = data->C;

    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < n; j++)
        {
            C[i][j] = 0; // Inicializar la celda antes de la suma
            for (int k = 0; k < n; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
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
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <matrix_size> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (n <= 0 || num_threads <= 0 || num_threads > MAX_THREADS)
    {
        fprintf(stderr, "Invalid input. Matrix size and threads must be positive, and threads <= %d.\n", MAX_THREADS);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    int **A = allocate_matrix(n);
    int **B = allocate_matrix(n);
    int **C = allocate_matrix(n);

    fill_matrix(A, n);
    fill_matrix(B, n);

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];

    struct timespec start = {0, 0}, end = {0, 0}; // Inicialización

    // --- Medir tiempo antes de la ejecución ---
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        perror("Error obteniendo el tiempo inicial");
        return EXIT_FAILURE;
    }

    // Crear los hilos
    int rows_per_thread = n / num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? n : (i + 1) * rows_per_thread;
        thread_data[i].n = n;
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;

        pthread_create(&threads[i], NULL, multiply_matrices_thread, (void *)&thread_data[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // --- Medir tiempo después de la ejecución ---
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
    {
        perror("Error obteniendo el tiempo final");
        return EXIT_FAILURE;
    }

    // Calcular tiempo transcurrido
    double wall_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // --- Imprimir resultados en la terminal ---
    printf("Matrix size: %d x %d, Threads: %d\n", n, n, num_threads);
    printf("Total execution time (wall-clock): %f seconds\n", wall_time);

    // --- Guardar solo el número en un archivo ---
    FILE *file = fopen("resultados.txt", "a"); // "a" para no sobrescribir
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
