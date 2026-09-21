/*
 * Actividad 1.1: Modelo de programacion y memoria en MPI
 * Equipo: 8
 * Integrantes:
 * Espinoza Hernandez, Axel Daniel
 * Martinez Ibarra, Alejandro Isaac
 * Munoz Perez, Jaime
 * Ruiz Guerrero, Oswaldo Josue
 */

#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <new>
#include <cstring>

class ArregloDinamicoHibrido
{
private:
    int* datos;
    long long tamanio;
    int proceso;
    int totalProcesos;
    std::string nombreNodo;

    static int generarValor(unsigned int& semilla)
    {
        // Generador independiente para evitar compartir el estado de rand
        // entre los hilos OpenMP.
        semilla = semilla * 1664525u + 1013904223u;
        return static_cast<int>(semilla % 1000u);
    }

public:
    ArregloDinamicoHibrido(
        long long cantidad,
        int numeroProceso,
        int cantidadProcesos,
        const char* nodo)
        : datos(NULL),
          tamanio(cantidad),
          proceso(numeroProceso),
          totalProcesos(cantidadProcesos),
          nombreNodo(nodo)
    {
        datos = new (std::nothrow) int[tamanio];
    }

    ~ArregloDinamicoHibrido()
    {
        delete[] datos;
        datos = NULL;
    }

    bool memoriaReservada() const
    {
        return datos != NULL;
    }

    double llenarConOpenMP()
    {
        const int cantidadHilos = omp_get_max_threads();
        unsigned int* semillas =
            new (std::nothrow) unsigned int[cantidadHilos];

        if (semillas == NULL)
        {
            return -1.0;
        }

        // srand y rand crean una semilla inicial diferente para cada hilo
        // y para cada proceso MPI.
        std::srand(
            static_cast<unsigned int>(std::time(NULL)) +
            static_cast<unsigned int>((proceso + 1) * 100003));

        for (int hilo = 0; hilo < cantidadHilos; ++hilo)
        {
            semillas[hilo] =
                static_cast<unsigned int>(std::rand()) ^
                static_cast<unsigned int>((hilo + 1) * 2654435761u);
        }

        const double inicio = MPI_Wtime();

        #pragma omp parallel shared(semillas)
        {
            const int hilo = omp_get_thread_num();
            unsigned int semillaLocal = semillas[hilo];

            // Diez etapas con barrera implicita. Cuando se imprime un
            // porcentaje, toda esa porcion del arreglo ya fue llenada.
            for (int etapa = 1; etapa <= 10; ++etapa)
            {
                const long long inicioEtapa =
                    (tamanio * (etapa - 1)) / 10;
                const long long finEtapa =
                    (tamanio * etapa) / 10;

                #pragma omp for schedule(static)
                for (long long i = inicioEtapa; i < finEtapa; ++i)
                {
                    datos[i] = generarValor(semillaLocal);
                }

                #pragma omp single
                {
                    std::ostringstream mensaje;
                    mensaje
                        << "[Equipo 8] "
                        << "Nodo: " << nombreNodo
                        << " | Proceso: " << proceso
                        << " de " << totalProcesos
                        << " | Hilo OpenMP: "
                        << omp_get_thread_num()
                        << " | Avance: " << (etapa * 10) << "%\n";

                    std::cout << mensaje.str() << std::flush;
                }
            }

            semillas[hilo] = semillaLocal;
        }

        const double fin = MPI_Wtime();
        delete[] semillas;

        return fin - inicio;
    }

    unsigned long long calcularSumaVerificacion() const
    {
        unsigned long long suma = 0;

        #pragma omp parallel for reduction(+:suma)
        for (long long i = 0; i < tamanio; ++i)
        {
            suma += static_cast<unsigned long long>(datos[i]);
        }

        return suma;
    }

    void mostrarContenido() const
    {
        std::cout
            << "\n============================================\n"
            << " RESULTADO DEL PROCESO " << proceso << "\n"
            << "============================================\n"
            << "Equipo: 8\n"
            << "Nodo: " << nombreNodo << "\n"
            << "Proceso MPI: " << proceso
            << " de " << totalProcesos << "\n"
            << "Tamanio del arreglo dinamico: " << tamanio << "\n";

        if (tamanio <= 100)
        {
            std::cout << "Contenido completo:\n";

            for (long long i = 0; i < tamanio; ++i)
            {
                std::cout << datos[i] << ' ';
            }

            std::cout << "\n";
        }
        else
        {
            // Imprimir diez millones de valores haria ilegible la terminal.
            // Se muestran los extremos y una suma para comprobar los datos.
            std::cout << "Primeros 20 elementos:\n";

            for (int i = 0; i < 20; ++i)
            {
                std::cout << datos[i] << ' ';
            }

            std::cout << "\nUltimos 20 elementos:\n";

            for (long long i = tamanio - 20; i < tamanio; ++i)
            {
                std::cout << datos[i] << ' ';
            }

            std::cout << "\n";
        }

        std::cout
            << "Suma de verificacion: "
            << calcularSumaVerificacion()
            << "\n============================================\n";
    }
};

void mostrarIntegrantes()
{
    std::cout
        << "Integrantes:\n"
        << "- Espinoza Hernandez Axel Daniel\n"
        << "- Martinez Ibarra Alejandro Isaac\n"
        << "- Munoz Perez Jaime\n"
        << "- Ruiz Guerrero Oswaldo Josue\n";
}

void mostrarNodos(
    int proceso,
    int totalProcesos,
    const char* nombreNodo,
    bool validarTresNodos)
{
    char* nombres = NULL;

    if (proceso == 0)
    {
        nombres = new char[totalProcesos * MPI_MAX_PROCESSOR_NAME];
    }

    char nombreFijo[MPI_MAX_PROCESSOR_NAME];
    std::memset(nombreFijo, 0, sizeof(nombreFijo));
    std::strncpy(
        nombreFijo,
        nombreNodo,
        MPI_MAX_PROCESSOR_NAME - 1);

    MPI_Gather(
        nombreFijo,
        MPI_MAX_PROCESSOR_NAME,
        MPI_CHAR,
        nombres,
        MPI_MAX_PROCESSOR_NAME,
        MPI_CHAR,
        0,
        MPI_COMM_WORLD);

    if (proceso == 0)
    {
        int nodosDiferentes = 0;

        std::cout << "\nDistribucion MPI:\n";

        for (int i = 0; i < totalProcesos; ++i)
        {
            const char* actual =
                nombres + i * MPI_MAX_PROCESSOR_NAME;

            std::cout
                << "Proceso " << i
                << " ejecutandose en nodo " << actual << "\n";

            bool encontradoAntes = false;

            for (int j = 0; j < i; ++j)
            {
                const char* anterior =
                    nombres + j * MPI_MAX_PROCESSOR_NAME;

                if (std::strcmp(actual, anterior) == 0)
                {
                    encontradoAntes = true;
                    break;
                }
            }

            if (!encontradoAntes)
            {
                ++nodosDiferentes;
            }
        }

        std::cout
            << "Nodos diferentes detectados: "
            << nodosDiferentes << "\n";

        if (validarTresNodos && nodosDiferentes < 3)
        {
            std::cout
                << "ADVERTENCIA: La tercera prueba requiere "
                << "al menos 3 computadoras.\n";
        }

        delete[] nombres;
    }
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int proceso = 0;
    int totalProcesos = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &proceso);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos);

    char nombreNodo[MPI_MAX_PROCESSOR_NAME];
    int longitudNombre = 0;

    MPI_Get_processor_name(nombreNodo, &longitudNombre);
    nombreNodo[longitudNombre] = '\0';

    int opcion = 0;
    long long tamanio = 0;

    if (proceso == 0)
    {
        std::cout
            << "============================================\n"
            << " ACTIVIDAD 1.1 - MODELO HIBRIDO MPI + OPENMP\n"
            << "============================================\n"
            << "Equipo: 8\n";

        mostrarIntegrantes();

        std::cout
            << "\n1. Primera ejecucion: arreglo aleatorio de 20 a 50\n"
            << "2. Segunda ejecucion: 10,000,000 de elementos\n"
            << "3. Tercera ejecucion distribuida: 10,000,000\n"
            << "Seleccione una opcion: ";

        std::cin >> opcion;

        while (opcion < 1 || opcion > 3)
        {
            std::cout << "Opcion no valida. Intente nuevamente: ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cin >> opcion;
        }

        if (opcion == 1)
        {
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            tamanio = 20 + (std::rand() % 31);
        }
        else
        {
            tamanio = 10000000LL;
        }
    }

    MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(
        &tamanio,
        1,
        MPI_LONG_LONG_INT,
        0,
        MPI_COMM_WORLD);

    mostrarNodos(
        proceso,
        totalProcesos,
        nombreNodo,
        opcion == 3);

    if (proceso == 0 && totalProcesos < 4)
    {
        std::cout
            << "ADVERTENCIA: La practica solicita por lo menos "
            << "4 procesos MPI.\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    ArregloDinamicoHibrido arreglo(
        tamanio,
        proceso,
        totalProcesos,
        nombreNodo);

    int memoriaLocal = arreglo.memoriaReservada() ? 1 : 0;
    int memoriaTodos = 0;

    MPI_Allreduce(
        &memoriaLocal,
        &memoriaTodos,
        1,
        MPI_INT,
        MPI_MIN,
        MPI_COMM_WORLD);

    if (memoriaTodos == 0)
    {
        if (proceso == 0)
        {
            std::cerr
                << "ERROR: Al menos un proceso no pudo reservar memoria.\n";
        }

        MPI_Finalize();
        return 1;
    }

    if (proceso == 0)
    {
        std::cout
            << "\nComenzando llenado de " << tamanio
            << " elementos por cada proceso...\n\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);
    const double tiempoLocal = arreglo.llenarConOpenMP();

    double tiempoMaximo = 0.0;

    MPI_Reduce(
        &tiempoLocal,
        &tiempoMaximo,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD);

    // Ordena la salida final por proceso para evitar que los arreglos
    // aparezcan mezclados en la terminal.
    for (int turno = 0; turno < totalProcesos; ++turno)
    {
        MPI_Barrier(MPI_COMM_WORLD);

        if (proceso == turno)
        {
            arreglo.mostrarContenido();
            std::cout << std::flush;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (proceso == 0)
    {
        std::cout
            << "\nTiempo total de la operacion (proceso mas lento): "
            << std::fixed << std::setprecision(6)
            << tiempoMaximo << " segundos\n"
            << "Todos los procesos finalizaron correctamente.\n";

        mostrarIntegrantes();
    }

    MPI_Finalize();
    return 0;
}
