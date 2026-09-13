 Programación Paralela

Actividad 2.6 Problema práctico en OpenMP

Programa académico en C++ que implementa una búsqueda exhaustiva de claves de prueba mediante una versión secuencial y una versión paralela con OpenMP. El espacio de combinaciones se divide en rangos y cada hilo examina únicamente el segmento que le corresponde.

Este programa se utiliza únicamente con claves creadas para la práctica. No está diseñado para acceder a cuentas, archivos protegidos, credenciales ni sistemas externos.

Integrantes

Espinoza Hernández Axel Daniel

Martínez Ibarra Alejandro Isaac

Muñoz Pérez Jaime

Ruiz Guerrero Oswaldo Josué

Caracteres utilizados

El espacio de búsqueda contiene 36 caracteres:

ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789

Para una clave de longitud n, el número total de combinaciones es 36^n.

Funcionalidades

Solicitud y validación de una clave académica.

Validación de longitud entre 1 y 10 caracteres.

Validación de letras mayúsculas A-Z y números 0-9.

Generación de combinaciones mediante conversión en base 36.

Búsqueda exhaustiva secuencial.

Búsqueda exhaustiva paralela con OpenMP.

Distribución explícita del espacio entre los hilos.

Identificación del hilo ganador.

Detención de los demás hilos cuando se encuentra la clave.

Uso de arreglos dinámicos.

Medición de tiempos y cálculo del speedup.

Algoritmos implementados

Versión secuencial

Recorre las combinaciones desde la primera posición hasta encontrar la clave introducida. Registra las combinaciones revisadas y el tiempo de ejecución.

Versión paralela

Divide el espacio completo en rangos de tamaño equivalente. Cada hilo recorre su propio rango y consulta un indicador compartido para saber si otro hilo ya encontró la clave.

OpenMP y sincronización

La solución utiliza:

#pragma omp parallel para crear la región paralela.

omp_get_thread_num() para identificar cada hilo.

omp_get_num_threads() para conocer la cantidad real de hilos.

#pragma omp critical para proteger el registro de la clave y del hilo ganador.

#pragma omp flush para hacer visible el indicador compartido.

#pragma omp barrier para coordinar el inicio de la búsqueda.

omp_get_wtime() para medir los tiempos.

Estructura del repositorio

Programacion-Paralela/
├── actividad_2_6_openmp.cpp
└── README.md

Compilación en Windows

Abrir x64 Native Tools Command Prompt for VS 2019, entrar en la carpeta del proyecto y ejecutar:

cl /EHsc /openmp /O2 actividad_2_6_openmp.cpp

Ejecución

actividad_2_6_openmp.exe

También se puede ejecutar desde PowerShell:

.\actividad_2_6_openmp.exe

Pruebas realizadas

Ejecución

Clave

Longitud

Hilos

Tiempo secuencial

Tiempo paralelo

Speedup

1

999

3

4

0.003652 s

0.009305 s

0.392449

2

ZZZZZ

5

4

4.229125 s

2.123064 s

1.991992

En la primera prueba, la sobrecarga de crear y sincronizar hilos hizo que la versión secuencial fuera más rápida. En la segunda, la versión paralela fue aproximadamente 1.99 veces más rápida debido al mayor espacio de búsqueda.