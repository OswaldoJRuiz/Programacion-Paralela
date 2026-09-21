// Nombres del equipo:
// Espinoza Hernandez, Axel Daniel
// Martinez Ibarra, Alejandro Isaac
// Munoz Perez, Jaime
// Ruiz Guerrero, Oswaldo Josue

#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Clase exigida por la practica
class OperacionesArreglos {
public:
    void crearArregloMPI(long long* A, long long* B, int tam, int N_total, const char* equipo, int nodo, bool detallado, int modoComm) {
        #pragma omp parallel for
        for (int i = 0; i < tam; i++) {
            // Calculo correcto de la posicion global segun el modo de comunicacion
            int posGlobal = (modoComm == 1) ? ((nodo - 1) * tam + i) : (nodo * tam + i);
            
            if (N_total == 40) {
                A[i] = posGlobal + 1;
                B[i] = N_total - posGlobal;
            } else {
                A[i] = rand() % 1000000 + 1;
                B[i] = rand() % 1000000 + 1;
            }

            if (detallado) {
                #pragma omp critical
                cout << "[Equipo: " << equipo << "] [Nodo MPI: " << nodo 
                     << "] [Hilo OpenMP: " << omp_get_thread_num() 
                     << "] [Posicion: " << posGlobal << "] [A: " << A[i] << "] [B: " << B[i] 
                     << "] [Operacion: Crear Arreglo]\n";
            }
        }
    }

    void sumar(long long* A, long long* B, long long* R, int tam, const char* equipo, int nodo, bool detallado, int modoComm) {
        #pragma omp parallel for
        for (int i = 0; i < tam; i++) {
            R[i] = A[i] + B[i];
            int posGlobal = (modoComm == 1) ? ((nodo - 1) * tam + i) : (nodo * tam + i);
            if (detallado) {
                #pragma omp critical
                cout << "[Equipo: " << equipo << "] [Nodo MPI: " << nodo 
                     << "] [Hilo OpenMP: " << omp_get_thread_num() 
                     << "] [Posicion: " << posGlobal << "] [Valor: " << R[i] << "] [Operacion: Suma]\n";
            }
        }
    }

    void restar(long long* A, long long* B, long long* R, int tam, const char* equipo, int nodo, bool detallado, int modoComm) {
        #pragma omp parallel for
        for (int i = 0; i < tam; i++) {
            R[i] = A[i] - B[i];
            int posGlobal = (modoComm == 1) ? ((nodo - 1) * tam + i) : (nodo * tam + i);
            if (detallado) {
                #pragma omp critical
                cout << "[Equipo: " << equipo << "] [Nodo MPI: " << nodo 
                     << "] [Hilo OpenMP: " << omp_get_thread_num() 
                     << "] [Posicion: " << posGlobal << "] [Valor: " << R[i] << "] [Operacion: Resta]\n";
            }
        }
    }

    void multiplicar(long long* A, long long* B, long long* R, int tam, const char* equipo, int nodo, bool detallado, int modoComm) {
        #pragma omp parallel for
        for (int i = 0; i < tam; i++) {
            R[i] = A[i] * B[i];
            int posGlobal = (modoComm == 1) ? ((nodo - 1) * tam + i) : (nodo * tam + i);
            if (detallado) {
                #pragma omp critical
                cout << "[Equipo: " << equipo << "] [Nodo MPI: " << nodo 
                     << "] [Hilo OpenMP: " << omp_get_thread_num() 
                     << "] [Posicion: " << posGlobal << "] [Valor: " << R[i] << "] [Operacion: Multiplicacion]\n";
            }
        }
    }

    void cuadrado(long long* A, long long* R, int tam, const char* equipo, int nodo, bool detallado, int modoComm) {
        #pragma omp parallel for
        for (int i = 0; i < tam; i++) {
            R[i] = A[i] * A[i];
            int posGlobal = (modoComm == 1) ? ((nodo - 1) * tam + i) : (nodo * tam + i);
            if (detallado) {
                #pragma omp critical
                cout << "[Equipo: " << equipo << "] [Nodo MPI: " << nodo 
                     << "] [Hilo OpenMP: " << omp_get_thread_num() 
                     << "] [Posicion: " << posGlobal << "] [Valor: " << R[i] << "] [Operacion: Cuadrado]\n";
            }
        }
    }
};

void mostrarArreglo(long long* arr, int N) {
    for (int i = 0; i < N; i++) cout << arr[i] << " ";
    cout << "\n";
}

int main(int argc, char** argv) {
    int rank, numProcs, lenNombre;
    char equipo[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Get_processor_name(equipo, &lenNombre);

    OperacionesArreglos op;
    int N = 0, opcion = 0, modoComm = 1; // 1: Send/Recv, 2: Scatter/Gather
    bool datosCreados = false;

    if (rank == 0) {
        cout << "============================================\n";
        cout << " INTEGRANTES DEL EQUIPO:\n";
        cout << " - Espinoza Hernandez, Axel Daniel\n";
        cout << " - Martinez Ibarra, Alejandro Isaac\n";
        cout << " - Munoz Perez, Jaime\n";
        cout << " - Ruiz Guerrero, Oswaldo Josue\n";
        cout << "============================================\n";
        cout << "   PRACTICA MPI + OPENMP (MODO LOCAL 100%)\n";
        cout << "============================================\n";
        cout << "1. Prueba pequena (40 elementos - Muestra detalles)\n";
        cout << "2. Prueba masiva (4,000,000 elementos - Solo tiempo)\n";
        cout << "Seleccione modo: ";
        cin >> opcion;
        N = (opcion == 1) ? 40 : 4000000;

        cout << "\nMecanismo de comunicacion:\n1. Send / Recv\n2. Scatter / Gather\nSeleccione: ";
        cin >> modoComm;
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&modoComm, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int trabajadores = numProcs - 1;
    int tamLocal = (modoComm == 1) ? (N / trabajadores) : (N / numProcs);
    bool detallado = (N == 40);

    // Arreglos dinamicos mediante punteros
    long long *A_local = new long long[tamLocal];
    long long *B_local = new long long[tamLocal];
    long long *R_local = new long long[tamLocal];

    long long *A = nullptr, *B = nullptr, *R = nullptr;
    if (rank == 0) {
        A = new long long[N];
        B = new long long[N];
        R = new long long[N];
    }

    srand(time(NULL) + rank);

    do {
        if (rank == 0) {
            cout << "\n================ MENU DE OPERACIONES ================\n";
            cout << "1. Crear arreglos\n2. Sumar arreglos\n3. Restar arreglos\n";
            cout << "4. Multiplicar arreglos\n5. Calcular cuadrado\n6. Salir\n";
            cout << "Ingrese una opcion: ";
            cin >> opcion;
        }

        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (opcion >= 1 && opcion <= 5) {
            MPI_Barrier(MPI_COMM_WORLD);
            double inicio = MPI_Wtime();

            // --- 1. CREACION DE DATOS ---
            if (opcion == 1) {
                if (modoComm == 1) { // Send / Recv
                    if (rank != 0) {
                        op.crearArregloMPI(A_local, B_local, tamLocal, N, equipo, rank, detallado, modoComm);
                        MPI_Send(A_local, tamLocal, MPI_LONG_LONG, 0, 1, MPI_COMM_WORLD);
                        MPI_Send(B_local, tamLocal, MPI_LONG_LONG, 0, 2, MPI_COMM_WORLD);
                    } else {
                        for (int i = 1; i < numProcs; i++) {
                            MPI_Recv(&A[(i-1)*tamLocal], tamLocal, MPI_LONG_LONG, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(&B[(i-1)*tamLocal], tamLocal, MPI_LONG_LONG, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        }
                    }
                } else { // Scatter / Gather
                    op.crearArregloMPI(A_local, B_local, tamLocal, N, equipo, rank, detallado, modoComm);
                    MPI_Gather(A_local, tamLocal, MPI_LONG_LONG, A, tamLocal, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                    MPI_Gather(B_local, tamLocal, MPI_LONG_LONG, B, tamLocal, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                }

                if (rank == 0) {
                    double fin = MPI_Wtime();
                    if (detallado) {
                        cout << "\nArreglo A creado:\n"; mostrarArreglo(A, N);
                        cout << "Arreglo B creado:\n"; mostrarArreglo(B, N);
                    }
                    cout << "Tiempo de creacion: " << (fin - inicio) << " segundos.\n";
                }
                datosCreados = true;
            } 
            // --- 2. OPERACIONES ARITMETICAS ---
            else {
                if (!datosCreados && rank == 0) {
                    cout << "Error: Primero debe crear los arreglos usando la Opcion 1.\n";
                    continue;
                }

                if (modoComm == 1) { // Send / Recv
                    if (rank == 0) {
                        for (int i = 1; i < numProcs; i++) {
                            MPI_Send(&A[(i-1)*tamLocal], tamLocal, MPI_LONG_LONG, i, 10, MPI_COMM_WORLD);
                            if (opcion != 5) MPI_Send(&B[(i-1)*tamLocal], tamLocal, MPI_LONG_LONG, i, 20, MPI_COMM_WORLD);
                        }
                        for (int i = 1; i < numProcs; i++) {
                            MPI_Recv(&R[(i-1)*tamLocal], tamLocal, MPI_LONG_LONG, i, 30, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        }
                    } else {
                        MPI_Recv(A_local, tamLocal, MPI_LONG_LONG, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        if (opcion != 5) MPI_Recv(B_local, tamLocal, MPI_LONG_LONG, 0, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                        if (opcion == 2) op.sumar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                        else if (opcion == 3) op.restar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                        else if (opcion == 4) op.multiplicar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                        else if (opcion == 5) op.cuadrado(A_local, R_local, tamLocal, equipo, rank, detallado, modoComm);

                        MPI_Send(R_local, tamLocal, MPI_LONG_LONG, 0, 30, MPI_COMM_WORLD);
                    }
                } else { // Scatter / Gather
                    MPI_Scatter(A, tamLocal, MPI_LONG_LONG, A_local, tamLocal, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                    if (opcion != 5) MPI_Scatter(B, tamLocal, MPI_LONG_LONG, B_local, tamLocal, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

                    if (opcion == 2) op.sumar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                    else if (opcion == 3) op.restar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                    else if (opcion == 4) op.multiplicar(A_local, B_local, R_local, tamLocal, equipo, rank, detallado, modoComm);
                    else if (opcion == 5) op.cuadrado(A_local, R_local, tamLocal, equipo, rank, detallado, modoComm);

                    MPI_Gather(R_local, tamLocal, MPI_LONG_LONG, R, tamLocal, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                }

                if (rank == 0) {
                    double fin = MPI_Wtime();
                    if (detallado) {
                        cout << "\nArreglo Resultado:\n";
                        mostrarArreglo(R, N);
                    }
                    cout << "Tiempo de ejecucion: " << (fin - inicio) << " segundos.\n";
                }
            }
        }
    } while (opcion != 6);

    if (rank == 0) {
        cout << "\n============================================\n";
        cout << " PROGRAMA FINALIZADO - INTEGRANTES:\n";
        cout << " - Espinoza Hernandez, Axel Daniel\n";
        cout << " - Martinez Ibarra, Alejandro Isaac\n";
        cout << " - Munoz Perez, Jaime\n";
        cout << " - Ruiz Guerrero, Oswaldo Josue\n";
        cout << "============================================\n";
    }

    // Memoria dinamica liberada obligatoria
    delete[] A_local; delete[] B_local; delete[] R_local;
    if (rank == 0) { delete[] A; delete[] B; delete[] R; }

    MPI_Finalize();
    return 0;
}

// Nombres del equipo:
// Espinoza Hernandez, Axel Daniel
// Martinez Ibarra, Alejandro Isaac
// Munoz Perez, Jaime
// Ruiz Guerrero, Oswaldo Josue