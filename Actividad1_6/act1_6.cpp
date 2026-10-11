#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <omp.h>
#include <mpi.h>

#define ALPHABET_SIZE 36
#define TAG_RANGE 1
#define TAG_SUCCESS 2
#define TAG_STOP 3
#define TAG_KEY_LEN 4

// Alfabeto estricto de 36 caracteres: A-Z (26) + 0-9 (10)
const char ALPHABET[ALPHABET_SIZE + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// Imprime el número de equipo y sus integrantes al inicio y fin (Orden Alfabético por Primer Apellido)
void imprimirNombresEquipo() {
    std::cout << "====================================================================\n";
    std::cout << "                         EQUIPO 8\n";
    std::cout << "INTEGRANTES DEL EQUIPO DE TRABAJO:\n";
    std::cout << "1. Espinoza Hernandez Axel Daniel\n";
    std::cout << "2. Martinez Ibarra Alejandro Isaac\n";
    std::cout << "3. Muñoz Perez Jaime\n";
    std::cout << "4. Ruiz Guerrero Oswaldo Josue\n";
    std::cout << "====================================================================\n" << std::endl;
}

// Convierte un índice numérico de base 36 a su correspondiente cadena alfanumérica
void convertirIndiceATexto(unsigned long long index, char* out_str, int length) {
    out_str[length] = '\0';
    for (int i = length - 1; i >= 0; i--) {
        out_str[i] = ALPHABET[index % ALPHABET_SIZE];
        index /= ALPHABET_SIZE;
    }
}

// Valida si la clave contiene únicamente caracteres válidos (A-Z, 0-9)
bool validarClave(const char* key) {
    int len = std::strlen(key);
    for (int i = 0; i < len; i++) {
        char c = key[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

// Búsqueda Secuencial (Línea Base para medición de Speedup)
bool busquedaSecuencial(const char* target_key, int key_len, unsigned long long total_comb, double& tiempo_ejecucion) {
    double t_inicio = MPI_Wtime();
    char* candidato = new char[key_len + 1];
    bool hallado = false;

    for (unsigned long long i = 0; i < total_comb; i++) {
        convertirIndiceATexto(i, candidato, key_len);
        if (std::strcmp(candidato, target_key) == 0) {
            hallado = true;
            break;
        }
    }

    delete[] candidato;
    tiempo_ejecucion = MPI_Wtime() - t_inicio;
    return hallado;
}

int main(int argc, char** argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Primera instrucción: Impresión de Equipo 8 e Integrantes desde el Nodo Maestro
    if (world_rank == 0) {
        imprimirNombresEquipo();
    }

    // Nomenclatura exacta solicitada para los archivos de Log
    char log_filename[256];
    std::snprintf(log_filename, sizeof(log_filename), "logs_pract1.6_equipo_8_%s_nodo_%d.txt", processor_name, world_rank);
    std::ofstream log_file(log_filename, std::ios::out);

    int max_threads = omp_get_max_threads();
    log_file << "========================================================\n";
    log_file << "EQUIPO 8 - LOG DE AUDITORIA (PRACTICA 1.6)\n";
    log_file << "NODO MPI: " << world_rank << " | NOMBRE PC (HOST): " << processor_name << "\n";
    log_file << "HILOS OPENMP ACTIVOS: " << max_threads << "\n";
    log_file << "========================================================\n\n";
    log_file.flush();

    int opcion = 0;
    char* target_key = nullptr;
    int key_len = 0;
    unsigned long long total_comb = 0;

    do {
        if (world_rank == 0) {
            std::cout << "\n--- MENU PRINCIPAL - EQUIPO 8 (MPI + OPENMP) ---\n";
            std::cout << "1. Ingresar clave personalizada en mayusculas (A-Z, 0-9)\n";
            std::cout << "2. Ejecutar busqueda en Modo Secuencial (Línea Base)\n";
            std::cout << "3. Ejecutar busqueda en Modo Paralelo Distribuido\n";
            std::cout << "4. Salir\n";
            std::cout << "Seleccione una opcion: ";
            std::cin >> opcion;

            if (opcion == 1) {
                char temp_buffer[128];
                std::cout << "Ingrese la clave (ej. X7B o KEY7890): ";
                std::cin >> temp_buffer;

                if (!validarClave(temp_buffer)) {
                    std::cout << "[ERROR] La clave contiene caracteres invalidos. Solo A-Z y 0-9.\n";
                    opcion = -1;
                } else {
                    key_len = std::strlen(temp_buffer);
                    if (target_key != nullptr) delete[] target_key;
                    target_key = new char[key_len + 1];
                    std::strcpy(target_key, temp_buffer);

                    total_comb = (unsigned long long)std::pow(ALPHABET_SIZE, key_len);
                    std::cout << "[OK] Clave cargada exitosamente. Longitud: " << key_len
                              << " | Total de Combinaciones (36^" << key_len << "): " << total_comb << "\n";
                }
            }
        }

        // Transmitir opción a todos los nodos
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (opcion == 1) {
            MPI_Bcast(&key_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if (world_rank != 0) {
                if (target_key != nullptr) delete[] target_key;
                target_key = new char[key_len + 1];
            }
            MPI_Bcast(target_key, key_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
            total_comb = (unsigned long long)std::pow(ALPHABET_SIZE, key_len);
        }
        else if (opcion == 2) { // Búsqueda Secuencial (Solo Maestro)
            if (world_rank == 0) {
                if (target_key == nullptr) {
                    std::cout << "[!] Primero debe ingresar una clave (Opcion 1).\n";
                } else {
                    std::cout << "\n[+] Ejecutando busqueda secuencial...\n";
                    double t_sec = 0.0;
                    bool encontrado = busquedaSecuencial(target_key, key_len, total_comb, t_sec);
                    std::cout << "    Resultado: " << (encontrado ? "ENCONTRADA" : "NO ENCONTRADA") << "\n";
                    std::cout << "    Tiempo Secuencial: " << t_sec << " segundos.\n";
                }
            }
        }
        else if (opcion == 3) { // Búsqueda Paralela Distribuida
            if (target_key == nullptr) {
                if (world_rank == 0) std::cout << "[!] Primero debe ingresar una clave (Opcion 1).\n";
                continue;
            }

            // Recopilación de nombres de PC de todos los nodos para el reporte centralizado
            char all_processor_names[world_size][MPI_MAX_PROCESSOR_NAME];
            MPI_Gather(processor_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       all_processor_names, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

            MPI_Barrier(MPI_COMM_WORLD);
            double t_inicio_paralelo = MPI_Wtime();

            int num_esclavos = (world_size > 1) ? (world_size - 1) : 1;
            unsigned long long base_range = total_comb / num_esclavos;
            unsigned long long remainder = total_comb % num_esclavos;

            unsigned long long mi_inicio = 0, mi_fin = 0;
            bool es_trabajador = false;

            if (world_size == 1) {
                mi_inicio = 0;
                mi_fin = total_comb - 1;
                es_trabajador = true;
            } else if (world_rank > 0) {
                int slave_id = world_rank - 1;
                mi_inicio = slave_id * base_range + std::min((unsigned long long)slave_id, remainder);
                mi_fin = mi_inicio + base_range + (slave_id < remainder ? 1 : 0) - 1;
                es_trabajador = true;
            }

            log_file << "[INICIO DE BUSQUEDA PARALELA - PRACTICA 1.6 - EQUIPO 8]\n";
            if (es_trabajador) {
                log_file << "Rango Asignado: [" << mi_inicio << " - " << mi_fin << "] ("
                         << (mi_fin - mi_inicio + 1) << " combinaciones)\n";
            } else {
                log_file << "Rol: Maestro Coordinador.\n";
            }
            log_file.flush();

            bool bandera_parada_local = false;
            bool clave_encontrada_global = false;
            int nodo_ganador = -1;
            int hilo_ganador = -1;
            double t_hallazgo = 0.0;

            if (es_trabajador) {
                bool encontrado_local = false;
                bool modo_detallado = (key_len <= 3);

                #pragma omp parallel shared(encontrado_local, bandera_parada_local)
                {
                    int tid = omp_get_thread_num();
                    char* candidato = new char[key_len + 1];

                    #pragma omp for schedule(dynamic, 1000)
                    for (unsigned long long idx = mi_inicio; idx <= mi_fin; idx++) {
                        if (encontrado_local || bandera_parada_local) {
                            continue;
                        }

                        convertirIndiceATexto(idx, candidato, key_len);

                        if (modo_detallado && (idx % 5000 == 0)) {
                            #pragma omp critical
                            {
                                log_file << "[Equipo: Equipo_8] [PC: " << processor_name << "] [Nodo MPI: " << world_rank
                                         << "] [Hilo OpenMP: " << tid << "] [Candidato: " << candidato << "] [Estado: Evaluando]\n";
                            }
                        }

                        if (std::strcmp(candidato, target_key) == 0) {
                            #pragma omp critical
                            {
                                if (!encontrado_local) {
                                    encontrado_local = true;
                                    t_hallazgo = MPI_Wtime() - t_inicio_paralelo;
                                    nodo_ganador = world_rank;
                                    hilo_ganador = tid;

                                    log_file << "\n[ÉXITO] [Equipo: Equipo_8] [PC: " << processor_name << "] [Nodo MPI: "
                                             << world_rank << "] [Hilo OpenMP: " << tid << "] [Clave Encontrada: \""
                                             << candidato << "\"] [Tiempo Parcial: " << t_hallazgo << "s]\n\n";
                                    log_file.flush();

                                    if (world_size > 1) {
                                        unsigned long long buffer_exito[2] = {(unsigned long long)tid, idx};
                                        MPI_Send(buffer_exito, 2, MPI_UNSIGNED_LONG_LONG, 0, TAG_SUCCESS, MPI_COMM_WORLD);
                                    }
                                }
                            }
                        }
                    }
                    delete[] candidato;
                }
            }

            if (world_rank == 0 && world_size > 1) {
                bool termino = false;
                while (!termino) {
                    int flag = 0;
                    MPI_Status status;
                    MPI_Iprobe(MPI_ANY_SOURCE, TAG_SUCCESS, MPI_COMM_WORLD, &flag, &status);

                    if (flag) {
                        unsigned long long buffer_exito[2];
                        MPI_Recv(buffer_exito, 2, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, TAG_SUCCESS, MPI_COMM_WORLD, &status);

                        nodo_ganador = status.MPI_SOURCE;
                        hilo_ganador = (int)buffer_exito[0];
                        clave_encontrada_global = true;
                        t_hallazgo = MPI_Wtime() - t_inicio_paralelo;

                        for (int i = 1; i < world_size; i++) {
                            int cancel_msg = 1;
                            MPI_Send(&cancel_msg, 1, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
                        }
                        termino = true;
                    }

                    if (MPI_Wtime() - t_inicio_paralelo > 600.0) {
                        termino = true;
                    }
                }
            } else if (world_size > 1) {
                int flag_stop = 0;
                MPI_Status status_stop;
                MPI_Iprobe(0, TAG_STOP, MPI_COMM_WORLD, &flag_stop, &status_stop);
                if (flag_stop) {
                    int cancel_dummy;
                    MPI_Recv(&cancel_dummy, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, &status_stop);
                    bandera_parada_local = true;
                    log_file << "[NOTIFICACIÓN DE PARADA TEMPRANA RECIBIDA DEL MAESTRO]\n";
                }
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double t_total_paralelo = MPI_Wtime() - t_inicio_paralelo;

            if (world_rank == 0) {
                std::cout << "\n====================================================\n";
                std::cout << "    REPORTE DE EJECUCIÓN PARALELA DISTRIBUIDA (EQUIPO 8)\n";
                std::cout << "====================================================\n";
                std::cout << "Clave Buscada:         " << target_key << "\n";
                std::cout << "Tiempo Total MPI:      " << t_total_paralelo << " segundos.\n\n";
                std::cout << "--- DESGLOSE DE NODOS Y MAQUINAS PARTICIPANTES ---\n";
                for (int r = 0; r < world_size; r++) {
                    std::cout << "  [Nodo MPI " << r << "] -> Host PC: " << all_processor_names[r]
                              << (r == 0 ? " (Maestro)" : " (Esclavo)") << "\n";
                }
                std::cout << "----------------------------------------------------\n";
                if (clave_encontrada_global || world_size == 1) {
                    std::cout << "Estado:                ¡CLAVE ENCONTRADA!\n";
                    std::cout << "Nodo Ganador (Rank):   " << nodo_ganador << " (Host: " << all_processor_names[nodo_ganador] << ")\n";
                    std::cout << "Hilo OpenMP Ganador:   " << hilo_ganador << "\n";
                } else {
                    std::cout << "Estado:                FINALIZADO SIN COINCIDENCIAS\n";
                }
                std::cout << "====================================================\n";
            }

            log_file << "[FIN DE PROCESO] Tiempo Total: " << t_total_paralelo << "s\n\n";
            log_file.flush();
        }

    } while (opcion != 4);

    if (target_key != nullptr) {
        delete[] target_key;
        target_key = nullptr;
    }

    log_file << "Cierre del archivo de log de auditoria.\n";
    log_file.close();

    // Última instrucción: Nombres del equipo desde el Nodo Maestro
    if (world_rank == 0) {
        std::cout << "\nFinalizando ejecucion en el clúster.\n";
        imprimirNombresEquipo();
    }

    MPI_Finalize();
    return 0;
}