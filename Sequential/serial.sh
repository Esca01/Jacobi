#!/bin/bash

# Lista de dimensiones a probar
DIMENSIONS=(10 100 200 400 800 1600 3200)
NSTEPS=1000  # Número de iteraciones

# Verifica si el ejecutable existe
if [ ! -f "./jacobi1d" ]; then
    echo "Error: No se encontró el ejecutable ./jacobi1d"
    exit 1
fi

# Crear carpeta para resultados
mkdir -p resultados_serial

# Ejecutar Jacobi para cada dimensión
for N in "${DIMENSIONS[@]}"; do
    echo "Ejecutando Jacobi con tamaño de matriz $N..."
    output_file="resultados_serial/serial_${N}.txt"
    
    > $output_file  # Limpiar archivo antes de escribir
    
    for i in {1..10}; do
        ./jacobi1d $N $NSTEPS u_serial_$N.out | grep -Eo "[0-9]+(\.[0-9]+)?" >> $output_file
    done
done

echo "Todas las pruebas han finalizado. Resultados guardados en 'resultados_serial/'"
