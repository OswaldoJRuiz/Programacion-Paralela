#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Incluye la libreria de tiempo para inicializar la semilla aleatoria
#include <omp.h>

// Integrantes:
// Espinoza Hernandez Axel Daniel
// Martinez Ibarra Alejandro Isaac
// Munoz Perez Jaime
// Ruiz Guerrero Oswaldo Josue

class OperacionesArreglos // Define la clase que contiene los metodos de operaciones paralelas
{
public: // Acceso publico para los metodos de la clase
    void sumarArreglosOpenMP(int* A, int* B, int* R, int tam) // Declaracion del metodo para sumar arreglos
    {
#pragma omp parallel for // Directiva para paralelizar el bucle for entre los hilos disponibles
        for (int i = 0; i < tam; i++) // Bucle para recorrer cada elemento del arreglo
        {
            R[i] = A[i] + B[i]; // Almacena la suma de los elementos correspondientes de A y B en R
        }
    }

    void restarArreglosOpenMP(int* A, int* B, int* R, int tam) // Declaracion del metodo para restar arreglos
    {
#pragma omp parallel for // Directiva para paralelizar el bucle for
        for (int i = 0; i < tam; i++) // Bucle para recorrer cada elemento del arreglo
        {
            R[i] = A[i] - B[i]; // Almacena la resta de los elementos de A menos B en R
        }
    }

    void multiplicarArreglosOpenMP(int* A, int* B, int* R, int tam) // Declaracion del metodo para multiplicar arreglos
    {
#pragma omp parallel for // Directiva para paralelizar el bucle for
        for (int i = 0; i < tam; i++) // Bucle para recorrer cada elemento del arreglo
        {
            R[i] = A[i] * B[i]; // Almacena la multiplicacion de los elementos correspondientes de A y B en R
        }
    }

    void cuadradoArregloOpenMP(int* A, int* R, int tam) // Declaracion del metodo para calcular el cuadrado de un arreglo
    {
#pragma omp parallel for // Directiva para paralelizar el bucle for
        for (int i = 0; i < tam; i++) // Bucle para recorrer cada elemento del arreglo
        {
            R[i] = A[i] * A[i]; // Almacena el cuadrado de cada elemento del arreglo A en R
        }
    }

    void sumatoria(int* arreglo, int tam, long long* suma_total)
    {
        long long suma = 0;
#pragma omp parallel for reduction(+:suma)
        for (int i = 0; i < tam; i++)
        {
            suma += arreglo[i];
        }
        *suma_total = suma;
    }

    void promedio(int* arreglo, int tam, double* promedio_res)
    {
        long long suma = 0;
#pragma omp parallel for reduction(+:suma)
        for (int i = 0; i < tam; i++)
        {
            suma += arreglo[i];
        }
        *promedio_res = (double)suma / tam;
    }

    void maximo(int* arreglo, int tam, int* max_val)
    {
        int max_global = arreglo[0];

#pragma omp parallel
        {
            int max_hilo = arreglo[0];

#pragma omp for nowait
            for (int i = 0; i < tam; i++)
            {
                if (arreglo[i] > max_hilo)
                {
                    max_hilo = arreglo[i];
                }
            }

#pragma omp critical
            {
                if (max_hilo > max_global)
                {
                    max_global = max_hilo;
                }
            }
        }

        *max_val = max_global;
    }

    void minimo(int* arreglo, int tam, int* min_val)
    {
        int min_global = arreglo[0];

#pragma omp parallel
        {
            int min_hilo = arreglo[0];

#pragma omp for nowait
            for (int i = 0; i < tam; i++)
            {
                if (arreglo[i] < min_hilo)
                {
                    min_hilo = arreglo[i];
                }
            }

#pragma omp critical
            {
                if (min_hilo < min_global)
                {
                    min_global = min_hilo;
                }
            }
        }

        *min_val = min_global;
    }
};

int main() // Funcion principal del programa
{
    printf("Integrantes:\n"); // Imprime el encabezado de integrantes
    printf("- Espinoza Hernandez Axel Daniel\n");
    printf("- Martinez Ibarra Alejandro Isaac\n");
    printf("- Munoz Perez Jaime\n");
    printf("- Ruiz Guerrero Oswaldo Josue\n\n");

    srand((unsigned int)time(NULL)); // Inicializa la semilla del generador de numeros aleatorios usando la hora actual
    OperacionesArreglos operaciones; // Crea un objeto de la clase OperacionesArreglos

    int tam = 100; // Declara e inicializa la variable del tamaño de arreglos
    int opcion; // Declara la variable para almacenar la opcion del menu

    printf("Ingresa el tamano de los arreglos (Ej. 100 o 10000000): "); // Solicita al usuario el tamaño inicial
    scanf("%d", &tam); // Lee el tamaño ingresado por el usuario

    int* A = (int*)malloc(tam * sizeof(int)); // Reserva memoria dinamica para el arreglo A
    int* B = (int*)malloc(tam * sizeof(int)); // Reserva memoria dinamica para el arreglo B
    int* R = (int*)malloc(tam * sizeof(int)); // Reserva memoria dinamica para el arreglo de resultados R

    // Llenado inicial: A ascendente y B descendente
    for (int i = 0; i < tam; i++) // Bucle para inicializar los datos de los arreglos
    {
        A[i] = i + 1; // Asigna valores secuenciales ascendentes al arreglo A
        B[i] = tam - i; // Asigna valores secuenciales descendentes al arreglo B
    }

    do // Inicio del bucle para repetir el menu
    {
        printf("\n--- MENU ---\n"); // Imprime la cabecera del menu
        printf("1. Llenar arreglos aleatorios\n");
        printf("2. Sumar arreglos\n");
        printf("3. Restar arreglos\n");
        printf("4. Multiplicar arreglos\n");
        printf("5. Cuadrado del arreglo A\n");
        printf("6. Cambiar tamano de los arreglos\n");
        printf("7. Sumatoria del arreglo A\n");
        printf("8. Promedio del arreglo A\n");
        printf("9. Maximo del arreglo A\n");
        printf("10. Minimo del arreglo A\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        switch (opcion) // Evalua la opcion seleccionada
        {
        case 1: // Bloque para la opcion de llenar con numeros aleatorios
            for (int i = 0; i < tam; i++) // Bucle para iterar en todas las posiciones
            {
                A[i] = rand() % 100;
                B[i] = rand() % 100;
            }
            printf("\nArreglos rellenados con valores aleatorios exito.\n"); // Notifica la correcta asignacion
            break; // Finaliza el caso 1

        case 2: // Bloque para la opcion de suma
        {
            if (tam <= 500) // Verifica si el tamaño es apto para impresion en consola
            {
                printf("\nArreglo A:\n"); // Imprime el titulo del arreglo A
                for (int i = 0; i < tam; i++) printf("%d ", A[i]);
                printf("\n\nArreglo B:\n");
                for (int i = 0; i < tam; i++) printf("%d ", B[i]);
            }

            double inicio = omp_get_wtime(); // Obtiene la marca de tiempo inicial
            operaciones.sumarArreglosOpenMP(A, B, R, tam); // Llama al metodo de suma paralela
            double fin = omp_get_wtime(); // Obtiene la marca de tiempo final

            if (tam <= 500)
            {
                printf("\n\nResultado R:\n");
                for (int i = 0; i < tam; i++) printf("%d ", R[i]);
            }

            printf("\n\nTiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 3: // Bloque para la opcion de resta
        {
            if (tam <= 500)
            {
                printf("\nArreglo A:\n");
                for (int i = 0; i < tam; i++) printf("%d ", A[i]);
                printf("\n\nArreglo B:\n");
                for (int i = 0; i < tam; i++) printf("%d ", B[i]);
            }

            double inicio = omp_get_wtime();
            operaciones.restarArreglosOpenMP(A, B, R, tam);
            double fin = omp_get_wtime();

            if (tam <= 500)
            {
                printf("\n\nResultado R:\n");
                for (int i = 0; i < tam; i++) printf("%d ", R[i]);
            }

            printf("\n\nTiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 4: // Bloque para la opcion de multiplicacion
        {
            if (tam <= 500)
            {
                printf("\nArreglo A:\n");
                for (int i = 0; i < tam; i++) printf("%d ", A[i]);
                printf("\n\nArreglo B:\n");
                for (int i = 0; i < tam; i++) printf("%d ", B[i]);
            }

            double inicio = omp_get_wtime();
            operaciones.multiplicarArreglosOpenMP(A, B, R, tam);
            double fin = omp_get_wtime();

            if (tam <= 500)
            {
                printf("\n\nResultado R:\n");
                for (int i = 0; i < tam; i++) printf("%d ", R[i]);
            }

            printf("\n\nTiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 5: // Bloque para la opcion de cuadrado del arreglo A
        {
            if (tam <= 500)
            {
                printf("\nArreglo A:\n");
                for (int i = 0; i < tam; i++) printf("%d ", A[i]);
            }

            double inicio = omp_get_wtime();
            operaciones.cuadradoArregloOpenMP(A, R, tam);
            double fin = omp_get_wtime();

            if (tam <= 500)
            {
                printf("\n\nResultado R:\n");
                for (int i = 0; i < tam; i++) printf("%d ", R[i]);
            }

            printf("\n\nTiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 6: // Bloque para la opcion de cambiar el tamaño de los arreglos
        {
            free(A); // Libera la memoria previamente asignada a A
            free(B); // Libera la memoria previamente asignada a B
            free(R); // Libera la memoria previamente asignada a R

            printf("\nIngresa el nuevo tamano de los arreglos: ");
            scanf("%d", &tam);

            A = (int*)malloc(tam * sizeof(int));
            B = (int*)malloc(tam * sizeof(int));
            R = (int*)malloc(tam * sizeof(int));

            for (int i = 0; i < tam; i++)
            {
                A[i] = i + 1;
                B[i] = tam - i;
            }

            printf("\nTamano actualizado a %d con exito.\n", tam);
            break;
        }

        case 7: // Bloque para la sumatoria del arreglo A
        {
            long long suma_total = 0;
            double inicio = omp_get_wtime();
            operaciones.sumatoria(A, tam, &suma_total);
            double fin = omp_get_wtime();

            printf("\nSumatoria del arreglo A: %lld\n", suma_total);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 8: // Bloque para el promedio del arreglo A
        {
            double promedio_res = 0.0;
            double inicio = omp_get_wtime();
            operaciones.promedio(A, tam, &promedio_res);
            double fin = omp_get_wtime();

            printf("\nPromedio del arreglo A: %f\n", promedio_res);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 9: // Bloque para el maximo del arreglo A
        {
            int max_val = 0;
            double inicio = omp_get_wtime();
            operaciones.maximo(A, tam, &max_val);
            double fin = omp_get_wtime();

            printf("\nMaximo del arreglo A: %d\n", max_val);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 10: // Bloque para el minimo del arreglo A
        {
            int min_val = 0;
            double inicio = omp_get_wtime();
            operaciones.minimo(A, tam, &min_val);
            double fin = omp_get_wtime();

            printf("\nMinimo del arreglo A: %d\n", min_val);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 0: // Bloque para salir del programa
            printf("\nPrograma terminado.\n");
            break;

        default: // Caso cuando la opcion ingresada no existe
            printf("\nOpcion invalida.\n");
        }

    } while (opcion != 0); // Termina el bucle al seleccionar 0

    free(A); // Libera la memoria asignada al arreglo A antes de terminar
    free(B); // Libera la memoria asignada al arreglo B antes de terminar
    free(R); // Libera la memoria asignada al arreglo R antes de terminar

    printf("- Espinoza Hernandez Axel Daniel\n");
    printf("- Martinez Ibarra Alejandro Isaac\n");
    printf("- Munoz Perez Jaime\n");
    printf("- Ruiz Guerrero Oswaldo Josue\n");

    return 0; // Finaliza el programa retornando cero
}
