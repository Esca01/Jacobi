#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>

// Asigna memoria compartida para una matriz
int **allocate_shared_matrix(int n)
{
    int fd = shm_open("/matrix_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, n * n * sizeof(int));
    int **matrix = mmap(NULL, n * n * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return matrix;
}

// Libera memoria compartida
void free_shared_matrix(int **matrix, int n)
{
    munmap(matrix, n * n * sizeof(int));
    shm_unlink("/matrix_shm");
}

// Asigna memoria normal para una matriz (no compartida)
int **allocate_matrix(int n)
{
    int **matrix = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)malloc(n * sizeof(int));
    }
    return matrix;
}

// Libera memoria normal
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

// Multiplicación de matrices en cada proceso
void multiply_matrices_process(int start_row, int end_row, int n, int **A, int **B, int **C)
{
    for (int i = start_row; i < end_row; i++)
    {
        for (int j = 0; j < n; j++)
        {
            C[i][j] = 0; // Inicializar
            for (int k = 0; k < n; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
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
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <matrix_size> [num_processes]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);                                                           // Tamaño de la matriz
    int num_processes = (argc >= 3) ? atoi(argv[2]) : sysconf(_SC_NPROCESSORS_ONLN); // Usa núcleos disponibles si no se especifica

    if (n <= 0 || num_processes <= 0)
    {
        fprintf(stderr, "Invalid input. Matrix size must be positive.\n");
        return EXIT_FAILURE;
    }

    printf("Using %d processes for a %dx%d matrix.\n", num_processes, n, n);

    srand(time(NULL));

    int **A = allocate_matrix(n);
    int **B = allocate_matrix(n);
    int **C = allocate_shared_matrix(n);

    fill_matrix(A, n);
    fill_matrix(B, n);

    struct timespec start = {0, 0}, end = {0, 0};

    // --- Medir tiempo antes de la ejecución ---
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        perror("Error obteniendo el tiempo inicial");
        return EXIT_FAILURE;
    }

    // Crear procesos
    pid_t *pids = malloc(num_processes * sizeof(pid_t)); // Se usa memoria dinámica para cualquier número de procesos
    int rows_per_process = n / num_processes;
    int extra_rows = n % num_processes; // Si la división no es exacta, repartir las filas restantes

    int start_row = 0;
    for (int i = 0; i < num_processes; i++)
    {
        int end_row = start_row + rows_per_process + (i < extra_rows ? 1 : 0); // Distribuye filas extra entre los primeros procesos

        if ((pids[i] = fork()) == 0)
        {
            multiply_matrices_process(start_row, end_row, n, A, B, C);
            exit(0);
        }
        start_row = end_row;
    }

    // Esperar a que todos los procesos terminen
    for (int i = 0; i < num_processes; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    free(pids); // Liberar memoria dinámica de los PIDs

    // --- Medir tiempo después de la ejecución ---
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
    {
        perror("Error obteniendo el tiempo final");
        return EXIT_FAILURE;
    }

    // Calcular tiempo transcurrido
    double wall_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // --- Imprimir resultados en la terminal ---
    printf("Matrix size: %d x %d, Processes: %d\n", n, n, num_processes);
    printf("Total execution time (wall-clock): %f seconds\n", wall_time);

    // --- Guardar solo el número en un archivo ---
    FILE *file = fopen("resultados_procesos.txt", "a");
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
    free_shared_matrix(C, n);

    return 0;
}
