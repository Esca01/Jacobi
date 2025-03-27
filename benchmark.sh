#!/bin/bash

# Compilar con flags para gprof
gcc -o jacobi_threads jacobi_threads.c -lpthread -pg
gcc -o jacobi_processes jacobi_processes.c -pg

# Crear carpeta para almacenar resultados
mkdir -p resultados_gprof

# Ejecutar Jacobi con diferentes tamaños de matriz y cantidades de hilos
for size in 10 100 200 400 800 1600 3200; do
    echo "Ejecutando Jacobi con tamaño $size y diferentes hilos..."
    for threads in 2 4 6 8 10 12; do
        output_file="resultados_gprof/threads_${threads}_${size}.txt"
        
        # Si el archivo ya existe, omitir la prueba
        if [ -f "$output_file" ]; then
            echo "Prueba con $threads hilos y matriz $size ya realizada. Omitiendo..."
            continue
        fi
        
        > $output_file  # Limpiar archivo antes de escribir
        for i in {1..10}; do
            ( time ./jacobi_threads $size $threads ) 2>&1 | awk '/real/ {split($2,a,"m"); print a[1]*60 + a[2]}' >> $output_file
        done
    done
    
    echo "Ejecutando Jacobi con tamaño $size y procesos..."
    output_file="resultados_gprof/processes_${size}.txt"
    
    # Si el archivo ya existe, omitir la prueba
    if [ -f "$output_file" ]; then
        echo "Prueba con procesos y matriz $size ya realizada. Omitiendo..."
        continue
    fi
    
    > $output_file  # Limpiar archivo antes de escribir
    for i in {1..10}; do
        ( time ./jacobi_processes $size ) 2>&1 | awk '/real/ {split($2,a,"m"); print a[1]*60 + a[2]}' >> $output_file
    done
done

# Generar reportes consolidados
echo "Generando reporte consolidado para hilos..."
gprof jacobi_threads resultados_gprof/gmon_threads_*.out > resultados_gprof/profile_threads_total.txt
echo "Reporte guardado en resultados_gprof/profile_threads_total.txt"

echo "Generando reporte consolidado para procesos..."
gprof jacobi_processes resultados_gprof/gmon_processes_*.out > resultados_gprof/profile_processes_total.txt
echo "Reporte guardado en resultados_gprof/profile_processes_total.txt"

echo "Benchmark completado."
