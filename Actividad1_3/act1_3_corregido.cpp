#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

class OperacionesArreglos 
{
public:
    void sumarArreglosOpenMP(int* A, int* B, int* R, int tam) 
    {
#pragma omp parallel for 
        for (int i = 0; i < tam; i++) 
        {
            R[i] = A[i] + B[i]; 
        }
    }

    void restarArreglosOpenMP(int* A, int* B, int* R, int tam) 
    {
#pragma omp parallel for 
        for (int i = 0; i < tam; i++) 
        {
            R[i] = A[i] - B[i]; 
        }
    }

    void multiplicarArreglosOpenMP(int* A, int* B, int* R, int tam) 
    {
#pragma omp parallel for 
        for (int i = 0; i < tam; i++) 
        {
            R[i] = A[i] * B[i]; 
        }
    }

    void cuadradoArregloOpenMP(int* A, int* R, int tam) 
    {
#pragma omp parallel for 
        for (int i = 0; i < tam; i++) 
        {
            R[i] = A[i] * A[i]; 
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

int main()
{
    printf("Integrantes:\n");
    printf("- Espinoza Hernandez Axel Daniel\n");
    printf("- Martinez Ibarra Alejandro Isaac\n");
    printf("- Munoz Perez Jaime\n");
    printf("- Ruiz Guerrero Oswaldo Josue\n\n");

    srand((unsigned int)time(NULL));
    OperacionesArreglos operaciones;

    int tam = 100;
    int opcion;

    printf("Ingresa el tamano de los arreglos (Ej. 100 o 10000000): ");
    scanf("%d", &tam);

    int* A = (int*)malloc(tam * sizeof(int)); 
    int* B = (int*)malloc(tam * sizeof(int)); 
    int* R = (int*)malloc(tam * sizeof(int)); 

    for (int i = 0; i < tam; i++)
    {
        A[i] = i + 1;
        B[i] = tam - i;
    }

    do
    {
        printf("\n--- MENU ---\n");
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

        switch (opcion)
        {
        case 1:
            for (int i = 0; i < tam; i++)
            {
                A[i] = rand() % 100;
                B[i] = rand() % 100;
            }
            printf("\nArreglos rellenados con valores aleatorios exito.\n");
            break;

        case 2:
        {
            if (tam <= 500)
            {
                printf("\nArreglo A:\n");
                for (int i = 0; i < tam; i++) printf("%d ", A[i]);
                printf("\n\nArreglo B:\n");
                for (int i = 0; i < tam; i++) printf("%d ", B[i]);
            }

            double inicio = omp_get_wtime();
            operaciones.sumarArreglosOpenMP(A, B, R, tam);
            double fin = omp_get_wtime();

            if (tam <= 500)
            {
                printf("\n\nResultado R:\n");
                for (int i = 0; i < tam; i++) printf("%d ", R[i]);
            }

            printf("\n\nTiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 3:
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

        case 4:
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

        case 5:
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

        case 6:
        {
            free(A);
            free(B);
            free(R);

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

        case 7:
        {
            long long suma_total = 0;
            double inicio = omp_get_wtime();
            operaciones.sumatoria(A, tam, &suma_total);
            double fin = omp_get_wtime();

            printf("\nSumatoria del arreglo A: %lld\n", suma_total);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 8:
        {
            double promedio_res = 0.0;
            double inicio = omp_get_wtime();
            operaciones.promedio(A, tam, &promedio_res);
            double fin = omp_get_wtime();

            printf("\nPromedio del arreglo A: %f\n", promedio_res);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 9:
        {
            int max_val = 0;
            double inicio = omp_get_wtime();
            operaciones.maximo(A, tam, &max_val);
            double fin = omp_get_wtime();

            printf("\nMaximo del arreglo A: %d\n", max_val);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 10:
        {
            int min_val = 0;
            double inicio = omp_get_wtime();
            operaciones.minimo(A, tam, &min_val);
            double fin = omp_get_wtime();

            printf("\nMinimo del arreglo A: %d\n", min_val);
            printf("Tiempo de ejecucion: %f segundos\n", fin - inicio);
            break;
        }

        case 0:
            printf("\nPrograma terminado.\n");
            break;

        default:
            printf("\nOpcion invalida.\n");
        }

    } while (opcion != 0);

    free(A);
    free(B);
    free(R);

    printf("- Espinoza Hernandez Axel Daniel\n");
    printf("- Martinez Ibarra Alejandro Isaac\n");
    printf("- Munoz Perez Jaime\n");
    printf("- Ruiz Guerrero Oswaldo Josue\n");

    return 0;
}