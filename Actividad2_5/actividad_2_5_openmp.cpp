#include <iostream>
#include <omp.h>
#include <ctime>

using namespace std;

const int LIMITE_IMPRESION = 1000;

int generarAleatorio(unsigned int& semilla, int maximo) {
    semilla = semilla * 1664525u + 1013904223u;
    return semilla % (maximo + 1);
}

void mostrarArreglo(int arreglo[], int n) {
    for (int i = 0; i < n; i++) {
        cout << arreglo[i] << " ";
    }

    cout << endl;
}

void copiarArreglo(int original[], int copia[], int n) {
    for (int i = 0; i < n; i++) {
        copia[i] = original[i];
    }
}

bool estaOrdenado(int arreglo[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arreglo[i] > arreglo[i + 1]) {
            return false;
        }
    }

    return true;
}

double llenarArreglo(int arreglo[], int n, int maximo) {
    int cantidadHilos = omp_get_max_threads();

    unsigned int* semillas =
        new unsigned int[cantidadHilos];

    unsigned int semillaBase =
        (unsigned int)time(NULL);

    for (int i = 0; i < cantidadHilos; i++) {
        semillas[i] =
            semillaBase + (i + 1) * 12345u;
    }

    double inicio = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int hilo = omp_get_thread_num();

        arreglo[i] = generarAleatorio(
            semillas[hilo],
            maximo
        );
    }

    double fin = omp_get_wtime();

    delete[] semillas;

    return fin - inicio;
}

void countingSortSecuencial(
    int arreglo[],
    int n,
    int maximo
) {
    int* contador =
        new int[maximo + 1];

    for (int i = 0; i <= maximo; i++) {
        contador[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        contador[arreglo[i]]++;
    }

    int posicion = 0;

    for (int numero = 0;
         numero <= maximo;
         numero++) {

        while (contador[numero] > 0) {
            arreglo[posicion] = numero;
            posicion++;
            contador[numero]--;
        }
    }

    delete[] contador;
}

void countingSortParalelo(
    int arreglo[],
    int n,
    int maximo
) {
    int* contador =
        new int[maximo + 1];

    #pragma omp parallel for
    for (int i = 0; i <= maximo; i++) {
        contador[i] = 0;
    }

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int numero = arreglo[i];

        #pragma omp atomic
        contador[numero]++;
    }

    int posicion = 0;

    for (int numero = 0;
         numero <= maximo;
         numero++) {

        while (contador[numero] > 0) {
            arreglo[posicion] = numero;
            posicion++;
            contador[numero]--;
        }
    }

    delete[] contador;
}

void merge(
    int arreglo[],
    int izquierda,
    int medio,
    int derecha
) {
    int cantidad =
        derecha - izquierda + 1;

    int* temporal =
        new int[cantidad];

    int i = izquierda;
    int j = medio + 1;
    int k = 0;

    while (
        i <= medio &&
        j <= derecha
    ) {
        if (arreglo[i] <= arreglo[j]) {
            temporal[k] = arreglo[i];
            i++;
        }
        else {
            temporal[k] = arreglo[j];
            j++;
        }

        k++;
    }

    while (i <= medio) {
        temporal[k] = arreglo[i];
        i++;
        k++;
    }

    while (j <= derecha) {
        temporal[k] = arreglo[j];
        j++;
        k++;
    }

    for (int x = 0;
         x < cantidad;
         x++) {

        arreglo[izquierda + x] =
            temporal[x];
    }

    delete[] temporal;
}

void mergeSortSecuencial(
    int arreglo[],
    int izquierda,
    int derecha
) {
    if (izquierda >= derecha) {
        return;
    }

    int medio =
        (izquierda + derecha) / 2;

    mergeSortSecuencial(
        arreglo,
        izquierda,
        medio
    );

    mergeSortSecuencial(
        arreglo,
        medio + 1,
        derecha
    );

    merge(
        arreglo,
        izquierda,
        medio,
        derecha
    );
}

void iniciarMergeParalelo(
    int arreglo[],
    int n
) {
    // Merge Sort iterativo compatible con OpenMP 2.0 de MSVC 2019.
    // En cada pasada se combinan segmentos independientes en paralelo.
    for (int ancho = 1; ancho < n; ancho *= 2) {
        int tamanoBloque = ancho * 2;
        int cantidadBloques =
            (n + tamanoBloque - 1) / tamanoBloque;

        #pragma omp parallel for schedule(static)
        for (int bloque = 0;
             bloque < cantidadBloques;
             bloque++) {

            int izquierda = bloque * tamanoBloque;
            int medio = izquierda + ancho - 1;
            int derecha = izquierda + tamanoBloque - 1;

            if (medio >= n - 1) {
                continue;
            }

            if (derecha >= n) {
                derecha = n - 1;
            }

            merge(
                arreglo,
                izquierda,
                medio,
                derecha
            );
        }
    }
}

int main() {
    int n = 100;
    int maximo = 200;

    int* original =
        new int[n];

    int* copia =
        new int[n];

    int opcion;

    bool arregloLleno = false;

    bool countingSecEjecutado = false;
    bool countingParEjecutado = false;

    bool mergeSecEjecutado = false;
    bool mergeParEjecutado = false;

    double tiempoLlenado = 0;

    double tiempoCountingSec = 0;
    double tiempoCountingPar = 0;

    double tiempoMergeSec = 0;
    double tiempoMergePar = 0;

    do {
        cout << "\n";
        cout << "==========================================\n";
        cout << "        ORDENAMIENTOS CON OPENMP\n";
        cout << "==========================================\n";

        cout << "Elementos: "
             << n
             << endl;

        cout << "Rango: 0 - "
             << maximo
             << endl;

        cout << "Hilos disponibles: "
             << omp_get_max_threads()
             << endl;

        cout << "==========================================\n";

        cout << "1. Llenar arreglo\n";
        cout << "2. Mostrar arreglo original\n";
        cout << "3. Counting Sort secuencial\n";
        cout << "4. Counting Sort paralelo\n";
        cout << "5. Merge Sort secuencial\n";
        cout << "6. Merge Sort paralelo\n";
        cout << "7. Comparar tiempos\n";
        cout << "8. Salir\n";

        cout << "==========================================\n";

        cout << "Opcion: ";

        cin >> opcion;

        switch (opcion) {

        case 1:
            tiempoLlenado =
                llenarArreglo(
                    original,
                    n,
                    maximo
                );

            arregloLleno = true;

            countingSecEjecutado = false;
            countingParEjecutado = false;

            mergeSecEjecutado = false;
            mergeParEjecutado = false;

            tiempoCountingSec = 0;
            tiempoCountingPar = 0;

            tiempoMergeSec = 0;
            tiempoMergePar = 0;

            cout << "\nArreglo llenado correctamente.\n";

            cout << "Tiempo de llenado: "
                 << tiempoLlenado
                 << " segundos\n";

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo desordenado:\n";

                mostrarArreglo(
                    original,
                    n
                );
            }

            break;

        case 2:
            if (!arregloLleno) {
                cout << "\nPrimero debes llenar el arreglo.\n";
                break;
            }

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo original:\n";

                mostrarArreglo(
                    original,
                    n
                );
            }
            else {
                cout << "\nEl arreglo es demasiado grande "
                     << "para mostrarlo.\n";
            }

            break;

        case 3:
            if (!arregloLleno) {
                cout << "\nPrimero debes llenar el arreglo.\n";
                break;
            }

            copiarArreglo(
                original,
                copia,
                n
            );

            {
                double inicio =
                    omp_get_wtime();

                countingSortSecuencial(
                    copia,
                    n,
                    maximo
                );

                double fin =
                    omp_get_wtime();

                tiempoCountingSec =
                    fin - inicio;

                countingSecEjecutado = true;
            }

            cout << "\nCOUNTING SORT SECUENCIAL\n";

            cout << "Tiempo: "
                 << tiempoCountingSec
                 << " segundos\n";

            cout << "Resultado correcto: ";

            if (estaOrdenado(copia, n)) {
                cout << "SI\n";
            }
            else {
                cout << "NO\n";
            }

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo ordenado:\n";

                mostrarArreglo(
                    copia,
                    n
                );
            }

            break;

        case 4:
            if (!arregloLleno) {
                cout << "\nPrimero debes llenar el arreglo.\n";
                break;
            }

            copiarArreglo(
                original,
                copia,
                n
            );

            {
                double inicio =
                    omp_get_wtime();

                countingSortParalelo(
                    copia,
                    n,
                    maximo
                );

                double fin =
                    omp_get_wtime();

                tiempoCountingPar =
                    fin - inicio;

                countingParEjecutado = true;
            }

            cout << "\nCOUNTING SORT PARALELO\n";

            cout << "Tiempo: "
                 << tiempoCountingPar
                 << " segundos\n";

            cout << "Resultado correcto: ";

            if (estaOrdenado(copia, n)) {
                cout << "SI\n";
            }
            else {
                cout << "NO\n";
            }

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo ordenado:\n";

                mostrarArreglo(
                    copia,
                    n
                );
            }

            break;

        case 5:
            if (!arregloLleno) {
                cout << "\nPrimero debes llenar el arreglo.\n";
                break;
            }

            copiarArreglo(
                original,
                copia,
                n
            );

            {
                double inicio =
                    omp_get_wtime();

                mergeSortSecuencial(
                    copia,
                    0,
                    n - 1
                );

                double fin =
                    omp_get_wtime();

                tiempoMergeSec =
                    fin - inicio;

                mergeSecEjecutado = true;
            }

            cout << "\nMERGE SORT SECUENCIAL\n";

            cout << "Tiempo: "
                 << tiempoMergeSec
                 << " segundos\n";

            cout << "Resultado correcto: ";

            if (estaOrdenado(copia, n)) {
                cout << "SI\n";
            }
            else {
                cout << "NO\n";
            }

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo ordenado:\n";

                mostrarArreglo(
                    copia,
                    n
                );
            }

            break;

        case 6:
            if (!arregloLleno) {
                cout << "\nPrimero debes llenar el arreglo.\n";
                break;
            }

            copiarArreglo(
                original,
                copia,
                n
            );

            {
                double inicio =
                    omp_get_wtime();

                iniciarMergeParalelo(
                    copia,
                    n
                );

                double fin =
                    omp_get_wtime();

                tiempoMergePar =
                    fin - inicio;

                mergeParEjecutado = true;
            }

            cout << "\nMERGE SORT PARALELO\n";

            cout << "Tiempo: "
                 << tiempoMergePar
                 << " segundos\n";

            cout << "Resultado correcto: ";

            if (estaOrdenado(copia, n)) {
                cout << "SI\n";
            }
            else {
                cout << "NO\n";
            }

            if (n <= LIMITE_IMPRESION) {
                cout << "\nArreglo ordenado:\n";

                mostrarArreglo(
                    copia,
                    n
                );
            }

            break;

        case 7:
            cout << "\n";
            cout << "==========================================\n";
            cout << "          COMPARACION DE TIEMPOS\n";
            cout << "==========================================\n";

            if (arregloLleno) {
                cout << "\nTiempo de llenado: "
                     << tiempoLlenado
                     << " segundos\n";
            }
            else {
                cout << "\nEl arreglo aun no ha sido llenado.\n";
            }

            cout << "\n---------- COUNTING SORT ----------\n";

            if (
                countingSecEjecutado &&
                countingParEjecutado
            ) {
                cout << "Secuencial: "
                     << tiempoCountingSec
                     << " segundos\n";

                cout << "Paralelo:   "
                     << tiempoCountingPar
                     << " segundos\n";

                if (tiempoCountingPar > 0) {
                    cout << "Speedup:    "
                         << tiempoCountingSec /
                            tiempoCountingPar
                         << "x\n";
                }
            }
            else {
                cout << "Debes ejecutar primero las dos "
                     << "versiones de Counting Sort.\n";
            }

            cout << "\n------------ MERGE SORT ------------\n";

            if (
                mergeSecEjecutado &&
                mergeParEjecutado
            ) {
                cout << "Secuencial: "
                     << tiempoMergeSec
                     << " segundos\n";

                cout << "Paralelo:   "
                     << tiempoMergePar
                     << " segundos\n";

                if (tiempoMergePar > 0) {
                    cout << "Speedup:    "
                         << tiempoMergeSec /
                            tiempoMergePar
                         << "x\n";
                }
            }
            else {
                cout << "Debes ejecutar primero las dos "
                     << "versiones de Merge Sort.\n";
            }

            cout << "==========================================\n";

            break;

        case 8:
            cout << "\nPrograma terminado.\n";
            break;

        default:
            cout << "\nOpcion incorrecta.\n";
        }

    } while (opcion != 8);

    delete[] original;
    delete[] copia;

    return 0;
}
