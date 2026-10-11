#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;

// Variables globales actualizadas
int* arreglo_global = nullptr;
int* sub_arreglo = nullptr;
int N = 0;
int bloque_size = 0;
string nombre_equipo = "Equipo_8"; // Nuevo nombre de equipo
char hostname[MPI_MAX_PROCESSOR_NAME];
ofstream log_file;

// Función para log seguro
void escribirLog(int rank, int hilo, string bloque, string algoritmo, string estado) {
    #pragma omp critical
    {
        if (log_file.is_open()) {
            log_file << "[Equipo: " << nombre_equipo << "_" << hostname << "] "
                     << "[Nodo MPI: " << rank << "] "
                     << "[Hilo OpenMP: " << hilo << "] "
                     << "[Bloque: " << bloque << "] "
                     << "[Algoritmo: " << algoritmo << "] "
                     << "[Estado: " << estado << "]\n";
            log_file.flush();
        }
    }
}

// Función para imprimir los integrantes (solo lo llamará el Maestro)
void imprimirIntegrantes() {
    cout << "\n=========================================" << endl;
    cout << "        INTEGRANTES DEL EQUIPO 8         " << endl;
    cout << "=========================================" << endl;
    cout << "- Espinoza Hernandez Axel Daniel." << endl;
    cout << "- Martinez Ibarra Alejandro Issac." << endl;
    cout << "- Munoz Perez Jaime." << endl;
    cout << "- Ruiz Guerrero Oswaldo Josue." << endl;
    cout << "=========================================\n" << endl;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, len;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(hostname, &len);

    string filename = "log_" + nombre_equipo + "_" + string(hostname) + "_nodo_" + to_string(rank) + ".txt";
    log_file.open(filename, ios::out | ios::trunc);

    // Impresión inicial de integrantes (solo el maestro)
    if (rank == 0) {
        imprimirIntegrantes();

        cout << "-> Maestro (Nodo 0) inicializado en: " << hostname << endl;
        cout << "   [Log maestro creado: " << filename << "]" << endl;
    } else {
        cout << "-> Esclavo (Nodo " << rank << ") inicializado en: " << hostname << endl;
        cout << "   [Log esclavo creado: " << filename << "]" << endl;
    }

    int max_threads = omp_get_max_threads();
    escribirLog(rank, -1, "N/A", "INICIO", "Sesion iniciada. Hilos OpenMP detectados: " + to_string(max_threads));

    // Ancla inicial segura usando Bcast
    int senal_inicio = 0;
    MPI_Bcast(&senal_inicio, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int opcion = 0;

    while (opcion != 7) {
        if (rank == 0) {
            cout << "\n--- MENU HIBRIDO (MPI + OpenMP) ---" << endl;
            cout << "1. Generar Arreglo Dinamico" << endl;
            cout << "2. Ordenar Arreglo (Local - Pre-requisito Binario)" << endl;
            cout << "3. Busqueda Secuencial (Solo maquina principal)" << endl;
            cout << "4. Busqueda Secuencial (Distribuida en cluster)" << endl;
            cout << "5. Busqueda Binaria (Solo maquina principal)" << endl;
            cout << "6. Busqueda Binaria (Distribuida en cluster)" << endl;
            cout << "7. Salir" << endl;
            cout << "Seleccione: ";
            cin >> opcion;
        }

        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        switch (opcion) {
            case 1: {
                if (rank == 0) {
                    cout << "Ingrese tamano del arreglo (N): ";
                    cin >> N;

                    if (N % size != 0) {
                        cout << "[ADVERTENCIA] " << N << " no es divisible entre " << size
                             << " nodos. Se truncara a " << (N / size) * size << " elementos." << endl;
                    }

                    if (N > 0) {
                        arreglo_global = new int[N];
                        for(int i = 0; i < N; i++) {
                            arreglo_global[i] = rand() % (N * 2);
                        }

                        int indice_ejemplo = N / 2;
                        cout << "\n-> [SUGERENCIA] Numero para buscar: " << arreglo_global[indice_ejemplo]
                             << " (Indice original: " << indice_ejemplo << ")" << endl;
                    }
                }

                MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

                if (N > 0) {
                    bloque_size = N / size;
                    if (sub_arreglo != nullptr) delete[] sub_arreglo;
                    sub_arreglo = new int[bloque_size];
                }
                break;
            }
            case 2: {
                if (rank == 0) {
                    if (arreglo_global == nullptr) { cout << "Genere el arreglo primero." << endl; break; }
                    cout << "Ordenando arreglo..." << endl;
                    double start_time = MPI_Wtime();
                    std::sort(arreglo_global, arreglo_global + N);
                    double end_time = MPI_Wtime();

                    int indice_ejemplo = N / 2;
                    cout << "-> [NUEVA SUGERENCIA POST-ORDENAMIENTO] El numero en la mitad ahora es: "
                         << arreglo_global[indice_ejemplo] << " (Indice " << indice_ejemplo << ")" << endl;
                    cout << "Tiempo de ordenamiento completado: " << (end_time - start_time) << "s" << endl;
                }
                break;
            }
            case 3: {
                if (rank == 0) {
                    if (arreglo_global == nullptr) { cout << "Genere el arreglo primero." << endl; break; }
                    int target, indice_encontrado = -1;
                    cout << "Numero a buscar (Local): ";
                    cin >> target;
                    double start_time = MPI_Wtime();

                    #pragma omp parallel for
                    for (int i = 0; i < N; i++) {
                        if (arreglo_global[i] == target) {
                            indice_encontrado = i;
                            escribirLog(rank, omp_get_thread_num(), "Arreglo Completo", "Secuencial Local", "Indice " + to_string(i));
                        }
                    }

                    double end_time = MPI_Wtime();
                    if (indice_encontrado != -1) cout << "Encontrado en indice: " << indice_encontrado << endl;
                    else cout << "No encontrado." << endl;
                    cout << "Tiempo local multihilo: " << (end_time - start_time) << "s" << endl;
                }
                break;
            }
            case 4: {
                int target;
                if (rank == 0) {
                    if (arreglo_global == nullptr) { cout << "Genere el arreglo primero." << endl; break; }
                    cout << "Numero a buscar (Distribuido): ";
                    cin >> target;
                }

                MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

                if (N > 0) {
                    MPI_Scatter(arreglo_global, bloque_size, MPI_INT, sub_arreglo, bloque_size, MPI_INT, 0, MPI_COMM_WORLD);

                    int indice_local = -1;
                    double start_time = MPI_Wtime();

                    #pragma omp parallel for
                    for (int i = 0; i < bloque_size; i++) {
                        int tid = omp_get_thread_num();

                        if (N <= 100) {
                            #pragma omp critical
                            cout << "[Nodo " << rank << " (" << hostname << ") - Hilo " << tid << "] Revisando indice local " << i
                                 << " (Global: " << (rank * bloque_size) + i << ")..." << endl;
                        }

                        if (sub_arreglo[i] == target) {
                            int indice_global = (rank * bloque_size) + i;
                            indice_local = indice_global;

                            #pragma omp critical
                            cout << "\n>>> [EXITO] ¡El [Nodo " << rank << " (" << hostname << ") - Hilo " << tid
                                 << "] ENCONTRO el numero " << target << " en el indice global " << indice_global << "! <<<\n" << endl;

                            escribirLog(rank, tid, to_string(rank * bloque_size) + "-" + to_string((rank + 1) * bloque_size - 1),
                                        "Secuencial Distribuida", "Encontrado en indice: " + to_string(indice_global));
                        }
                    }

                    int indice_resultado = -1;
                    MPI_Reduce(&indice_local, &indice_resultado, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

                    if (rank == 0) {
                        double end_time = MPI_Wtime();
                        if (indice_resultado == -1) cout << "Elemento no encontrado en la busqueda distribuida." << endl;
                        cout << "Tiempo total: " << (end_time - start_time) << "s" << endl;
                    }
                }
                break;
            }
            case 5: {
                if (rank == 0) {
                    if (arreglo_global == nullptr) { cout << "Genere el arreglo primero." << endl; break; }
                    int target;
                    cout << "Numero a buscar (Binaria Local): ";
                    cin >> target;
                    double start_time = MPI_Wtime();
                    bool encontrado = std::binary_search(arreglo_global, arreglo_global + N, target);
                    double end_time = MPI_Wtime();
                    if (encontrado) cout << "Elemento si existe en el arreglo." << endl;
                    else cout << "Elemento no encontrado." << endl;
                    cout << "Tiempo de busqueda binaria local: " << (end_time - start_time) << "s" << endl;
                }
                break;
            }
            case 6: {
                int target;
                if (rank == 0) {
                    if (arreglo_global == nullptr) { cout << "Genere el arreglo primero." << endl; break; }
                    cout << "Numero a buscar (Binaria Distribuida): ";
                    cin >> target;
                }

                MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

                if (N > 0) {
                    MPI_Scatter(arreglo_global, bloque_size, MPI_INT, sub_arreglo, bloque_size, MPI_INT, 0, MPI_COMM_WORLD);

                    int indice_local = -1;
                    double start_time = MPI_Wtime();

                    if (target >= sub_arreglo[0] && target <= sub_arreglo[bloque_size - 1]) {

                        if (N <= 100) {
                            #pragma omp critical
                            cout << "[Nodo " << rank << " (" << hostname << ")] El numero " << target << " esta en mi rango. Iniciando hilos..." << endl;
                        }

                        #pragma omp parallel
                        {
                            int tid = omp_get_thread_num();
                            int num_threads = omp_get_num_threads();

                            int chunk_size = bloque_size / num_threads;
                            int inicio = tid * chunk_size;
                            int fin = (tid == num_threads - 1) ? bloque_size - 1 : inicio + chunk_size - 1;

                            if (N <= 100) {
                                #pragma omp critical
                                cout << "[Nodo " << rank << " (" << hostname << ") - Hilo " << tid << "] Aplicando binaria en sub-segmento ["
                                     << inicio << " a " << fin << "]" << endl;
                            }

                            int left = inicio;
                            int right = fin;

                            while (left <= right && indice_local == -1) {
                                int mid = left + (right - left) / 2;

                                if (sub_arreglo[mid] == target) {
                                    int indice_global = (rank * bloque_size) + mid;

                                    #pragma omp critical
                                    {
                                        indice_local = indice_global;
                                        cout << "\n>>> [EXITO BINARIO] ¡El [Nodo " << rank << " (" << hostname << ") - Hilo " << tid
                                             << "] ENCONTRO el numero " << target << " en el indice global " << indice_global << "! <<<\n" << endl;
                                    }
                                    break;
                                }
                                else if (sub_arreglo[mid] < target) {
                                    left = mid + 1;
                                }
                                else {
                                    right = mid - 1;
                                }
                            }
                        }
                    }

                    int indice_resultado = -1;
                    MPI_Reduce(&indice_local, &indice_resultado, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

                    if (rank == 0) {
                        double end_time = MPI_Wtime();
                        if (indice_resultado == -1) cout << "Elemento no encontrado en ningun nodo." << endl;
                        cout << "Tiempo total (comunicacion + busqueda binaria): " << (end_time - start_time) << "s" << endl;
                    }
                }
                break;
            }
            case 7:
                if (sub_arreglo != nullptr) {
                    delete[] sub_arreglo;
                    sub_arreglo = nullptr;
                }
                if (rank == 0 && arreglo_global != nullptr) {
                    delete[] arreglo_global;
                    arreglo_global = nullptr;
                }
                escribirLog(rank, -1, "N/A", "FIN", "Liberacion de memoria dinamica y salida.");

                // Impresión final de integrantes (solo el maestro) antes de salir
                if (rank == 0) {
                    cout << "\n--- EJECUCION FINALIZADA ---" << endl;
                    imprimirIntegrantes();
                }
                break;
        }

        // --- SINCRONIZACIÓN SEGURA Y DRENAJE DE BÚFER ---
        cout << flush;

        int ancla_envio = 1;
        int ancla_recepcion = 0;
        MPI_Reduce(&ancla_envio, &ancla_recepcion, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    log_file.close();
    MPI_Finalize();
    return 0;
}