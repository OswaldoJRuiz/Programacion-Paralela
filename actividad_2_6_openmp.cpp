/*
Espinoza Hernandez Axel Daniel
Martinez Ibarra Alejandro Isaac
Munoz Perez Jaime
Ruiz Guerrero Oswaldo Josue

Actividad 2.6: Problema practico en OpenMP
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <omp.h>

class BusquedaExhaustiva
{
private:
    char* caracteres;
    int cantidadCaracteres;

public:
    BusquedaExhaustiva()
    {
        cantidadCaracteres = 36;
        caracteres = new char[cantidadCaracteres + 1];
        std::strcpy(caracteres, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    }

    ~BusquedaExhaustiva()
    {
        delete[] caracteres;
    }

    unsigned long long calcularEspacio(int longitud)
    {
        unsigned long long total = 1;

        for (int i = 0; i < longitud; i++)
        {
            total *= (unsigned long long)cantidadCaracteres;
        }

        return total;
    }

    void posicionAClave(
        unsigned long long posicion,
        int longitud,
        char* claveGenerada)
    {
        for (int i = longitud - 1; i >= 0; i--)
        {
            int indice = (int)(posicion % cantidadCaracteres);
            claveGenerada[i] = caracteres[indice];
            posicion /= cantidadCaracteres;
        }

        claveGenerada[longitud] = '\0';
    }

    bool caracterPermitido(char caracter)
    {
        for (int i = 0; i < cantidadCaracteres; i++)
        {
            if (caracteres[i] == caracter)
            {
                return true;
            }
        }

        return false;
    }

    bool validarClave(const char* clave, int longitudSeleccionada)
    {
        int longitudReal = (int)std::strlen(clave);

        if (longitudReal == 0)
        {
            std::cout << "ERROR: La clave no puede estar vacia.\n";
            return false;
        }

        if (longitudReal != longitudSeleccionada)
        {
            std::cout << "ERROR: La clave debe tener exactamente "
                      << longitudSeleccionada << " caracteres.\n";
            return false;
        }

        for (int i = 0; i < longitudReal; i++)
        {
            if (!caracterPermitido(clave[i]))
            {
                std::cout << "ERROR: El caracter '" << clave[i]
                          << "' no esta permitido.\n";
                std::cout << "Utilice solamente A-Z y 0-9, sin espacios.\n";
                return false;
            }
        }

        return true;
    }

    bool buscarSecuencial(
        const char* claveObjetivo,
        int longitud,
        unsigned long long totalCombinaciones,
        unsigned long long* combinacionesRevisadas,
        double* tiempo,
        char* claveEncontrada)
    {
        char* candidata = new char[longitud + 1];
        bool encontrada = false;
        *combinacionesRevisadas = 0;

        std::cout << "\n============================================\n";
        std::cout << "          BUSQUEDA SECUENCIAL\n";
        std::cout << "============================================\n";
        std::cout << "Estado: INICIANDO\n";

        double inicio = omp_get_wtime();

        for (unsigned long long posicion = 0;
             posicion < totalCombinaciones;
             posicion++)
        {
            posicionAClave(posicion, longitud, candidata);
            (*combinacionesRevisadas)++;

            if (std::strcmp(candidata, claveObjetivo) == 0)
            {
                std::strcpy(claveEncontrada, candidata);
                encontrada = true;
                break;
            }
        }

        double fin = omp_get_wtime();
        *tiempo = fin - inicio;

        std::cout << "Estado: FINALIZADA\n";
        std::cout << "Combinaciones revisadas: "
                  << *combinacionesRevisadas << "\n";

        if (encontrada)
        {
            std::cout << "Clave encontrada: " << claveEncontrada << "\n";
        }
        else
        {
            std::cout << "Clave no encontrada.\n";
        }

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Tiempo secuencial: " << *tiempo << " segundos\n";

        delete[] candidata;
        return encontrada;
    }

    bool buscarParalelo(
        const char* claveObjetivo,
        int longitud,
        unsigned long long totalCombinaciones,
        int numeroHilosSolicitado,
        unsigned long long* combinacionesRevisadas,
        double* tiempo,
        int* hiloGanador,
        char* claveEncontrada,
        int* hilosUtilizados)
    {
        volatile int encontrada = 0;
        *hiloGanador = -1;
        *combinacionesRevisadas = 0;
        *hilosUtilizados = 0;

        omp_set_num_threads(numeroHilosSolicitado);

        // Arreglos dinamicos para registrar el trabajo de cada hilo.
        unsigned long long* inicios =
            new unsigned long long[numeroHilosSolicitado];
        unsigned long long* finales =
            new unsigned long long[numeroHilosSolicitado];
        unsigned long long* asignadas =
            new unsigned long long[numeroHilosSolicitado];
        unsigned long long* revisadasPorHilo =
            new unsigned long long[numeroHilosSolicitado];
        int* encontroClave = new int[numeroHilosSolicitado];

        for (int i = 0; i < numeroHilosSolicitado; i++)
        {
            inicios[i] = 0;
            finales[i] = 0;
            asignadas[i] = 0;
            revisadasPorHilo[i] = 0;
            encontroClave[i] = 0;
        }

        std::cout << "\n============================================\n";
        std::cout << "        BUSQUEDA PARALELA CON OPENMP\n";
        std::cout << "============================================\n";

        double inicioTiempo = omp_get_wtime();

#pragma omp parallel shared(encontrada, claveEncontrada, inicios, finales, asignadas, revisadasPorHilo, encontroClave)
        {
            int idHilo = omp_get_thread_num();
            int totalHilos = omp_get_num_threads();

#pragma omp single
            {
                *hilosUtilizados = totalHilos;
            }

            unsigned long long tamanoBase =
                totalCombinaciones / (unsigned long long)totalHilos;
            unsigned long long sobrantes =
                totalCombinaciones % (unsigned long long)totalHilos;

            unsigned long long cantidad = tamanoBase;
            if ((unsigned long long)idHilo < sobrantes)
            {
                cantidad++;
            }

            unsigned long long inicioRango =
                (unsigned long long)idHilo * tamanoBase;

            if ((unsigned long long)idHilo < sobrantes)
            {
                inicioRango += (unsigned long long)idHilo;
            }
            else
            {
                inicioRango += sobrantes;
            }

            unsigned long long finRango = inicioRango + cantidad - 1;

            inicios[idHilo] = inicioRango;
            finales[idHilo] = finRango;
            asignadas[idHilo] = cantidad;

            char* primeraClave = new char[longitud + 1];
            char* ultimaClave = new char[longitud + 1];
            char* candidata = new char[longitud + 1];

            posicionAClave(inicioRango, longitud, primeraClave);
            posicionAClave(finRango, longitud, ultimaClave);

#pragma omp critical(salida_inicio)
            {
                std::cout << "Hilo " << idHilo
                          << " -> Inicio: " << primeraClave
                          << " -> Fin: " << ultimaClave
                          << " -> Cantidad: " << cantidad
                          << " -> Estado: INICIA\n";
            }

            // Todos muestran su rango antes de comenzar la busqueda.
#pragma omp barrier

            for (unsigned long long posicion = inicioRango;
                 posicion <= finRango;
                 posicion++)
            {
#pragma omp flush(encontrada)
                if (encontrada)
                {
                    break;
                }

                posicionAClave(posicion, longitud, candidata);
                revisadasPorHilo[idHilo]++;

                if (std::strcmp(candidata, claveObjetivo) == 0)
                {
#pragma omp critical(registro_ganador)
                    {
                        if (!encontrada)
                        {
                            encontrada = 1;
                            *hiloGanador = idHilo;
                            encontroClave[idHilo] = 1;
                            std::strcpy(claveEncontrada, candidata);
                        }
                    }

#pragma omp flush(encontrada)
                    break;
                }
            }

#pragma omp critical(salida_final)
            {
                std::cout << "Hilo " << idHilo
                          << " -> Revisadas: " << revisadasPorHilo[idHilo]
                          << " -> Estado: FINALIZA";

                if (encontroClave[idHilo])
                {
                    std::cout << " -> ENCONTRO LA CLAVE";
                }
                else if (encontrada &&
                         revisadasPorHilo[idHilo] < asignadas[idHilo])
                {
                    std::cout << " -> DETENIDO POR OTRO HILO";
                }
                else
                {
                    std::cout << " -> NO ENCONTRO LA CLAVE";
                }

                std::cout << "\n";
            }

            delete[] primeraClave;
            delete[] ultimaClave;
            delete[] candidata;
        }

        double finTiempo = omp_get_wtime();
        *tiempo = finTiempo - inicioTiempo;

        for (int i = 0; i < *hilosUtilizados; i++)
        {
            *combinacionesRevisadas += revisadasPorHilo[i];
        }

        std::cout << "\n--- RESULTADO PARALELO ---\n";
        std::cout << "Hilos utilizados: " << *hilosUtilizados << "\n";
        std::cout << "Combinaciones revisadas entre todos los hilos: "
                  << *combinacionesRevisadas << "\n";

        if (encontrada)
        {
            std::cout << "Clave encontrada: " << claveEncontrada << "\n";
            std::cout << "Clave encontrada por el hilo: "
                      << *hiloGanador << "\n";
        }
        else
        {
            std::cout << "Clave no encontrada.\n";
        }

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Tiempo paralelo: " << *tiempo << " segundos\n";

        delete[] inicios;
        delete[] finales;
        delete[] asignadas;
        delete[] revisadasPorHilo;
        delete[] encontroClave;

        return encontrada != 0;
    }
};

int main()
{
    std::cout << "============================================\n";
    std::cout << " ACTIVIDAD 2.6 - BUSQUEDA DE CLAVE DE PRUEBA\n";
    std::cout << "============================================\n";
    std::cout << "Integrantes:\n";
    std::cout << "- Espinoza Hernandez Axel Daniel\n";
    std::cout << "- Martinez Ibarra Alejandro Isaac\n";
    std::cout << "- Munoz Perez Jaime\n";
    std::cout << "- Ruiz Guerrero Oswaldo Josue\n\n";
    std::cout << "ADVERTENCIA: Use solo claves creadas para esta practica.\n";
    std::cout << "No utilice contrasenas ni credenciales reales.\n\n";

    BusquedaExhaustiva buscador;
    int longitud = 0;
    int numeroHilos = 0;
    int maximoHilos = omp_get_max_threads();

    do
    {
        std::cout << "Longitud de la clave de prueba (1 a 10): ";

        if (!(std::cin >> longitud))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            longitud = 0;
        }

        if (longitud < 1 || longitud > 10)
        {
            std::cout << "ERROR: Ingrese una longitud entre 1 y 10.\n";
        }
    }
    while (longitud < 1 || longitud > 10);

    std::cin.ignore(10000, '\n');

    char* entrada = new char[128];
    char* clavePrueba = new char[longitud + 1];
    bool claveValida = false;

    do
    {
        std::cout << "Ingrese una clave academica de " << longitud
                  << " caracteres usando A-Z y 0-9: ";
        std::cin.getline(entrada, 128);

        claveValida = buscador.validarClave(entrada, longitud);

        if (claveValida)
        {
            std::strcpy(clavePrueba, entrada);
        }
    }
    while (!claveValida);

    do
    {
        std::cout << "Numero de hilos (1 a " << maximoHilos << "): ";

        if (!(std::cin >> numeroHilos))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            numeroHilos = 0;
        }

        if (numeroHilos < 1 || numeroHilos > maximoHilos)
        {
            std::cout << "ERROR: Numero de hilos no valido.\n";
        }
    }
    while (numeroHilos < 1 || numeroHilos > maximoHilos);

    unsigned long long totalCombinaciones =
        buscador.calcularEspacio(longitud);

    char* primeraCombinacion = new char[longitud + 1];
    char* ultimaCombinacion = new char[longitud + 1];
    buscador.posicionAClave(0, longitud, primeraCombinacion);
    buscador.posicionAClave(
        totalCombinaciones - 1,
        longitud,
        ultimaCombinacion);

    std::cout << "\n============================================\n";
    std::cout << "          CONFIGURACION DE LA PRUEBA\n";
    std::cout << "============================================\n";
    std::cout << "Clave introducida: " << clavePrueba << "\n";
    std::cout << "Longitud: " << longitud << "\n";
    std::cout << "Conjunto: ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n";
    std::cout << "Cantidad de caracteres: 36\n";
    std::cout << "Numero total de combinaciones (36^"
              << longitud << "): " << totalCombinaciones << "\n";
    std::cout << "Primera combinacion: " << primeraCombinacion << "\n";
    std::cout << "Ultima combinacion: " << ultimaCombinacion << "\n";
    std::cout << "Hilos solicitados: " << numeroHilos << "\n";

    unsigned long long revisadasSecuencial = 0;
    unsigned long long revisadasParalelo = 0;
    double tiempoSecuencial = 0.0;
    double tiempoParalelo = 0.0;
    int hiloGanador = -1;
    int hilosUtilizados = 0;

    char* encontradaSecuencial = new char[longitud + 1];
    char* encontradaParalelo = new char[longitud + 1];
    encontradaSecuencial[0] = '\0';
    encontradaParalelo[0] = '\0';

    bool resultadoSecuencial = buscador.buscarSecuencial(
        clavePrueba,
        longitud,
        totalCombinaciones,
        &revisadasSecuencial,
        &tiempoSecuencial,
        encontradaSecuencial);

    bool resultadoParalelo = buscador.buscarParalelo(
        clavePrueba,
        longitud,
        totalCombinaciones,
        numeroHilos,
        &revisadasParalelo,
        &tiempoParalelo,
        &hiloGanador,
        encontradaParalelo,
        &hilosUtilizados);

    std::cout << "\n============================================\n";
    std::cout << "          COMPARACION DE RESULTADOS\n";
    std::cout << "============================================\n";
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Tiempo secuencial: " << tiempoSecuencial
              << " segundos\n";
    std::cout << "Tiempo paralelo:   " << tiempoParalelo
              << " segundos\n";
    std::cout << "Hilos utilizados:  " << hilosUtilizados << "\n";
    std::cout << "Hilo ganador:       ";

    if (hiloGanador >= 0)
    {
        std::cout << hiloGanador << "\n";
    }
    else
    {
        std::cout << "Ninguno\n";
    }

    if (tiempoParalelo > 0.0)
    {
        double speedup = tiempoSecuencial / tiempoParalelo;
        std::cout << "Speedup:            " << speedup << "\n";

        if (speedup > 1.0)
        {
            std::cout << "Analisis: La version paralela fue mas rapida.\n";
        }
        else
        {
            std::cout << "Analisis: Para esta prueba pequena, el costo de "
                      << "crear y coordinar hilos fue mayor que la mejora.\n";
        }
    }

    bool coincidenciaCorrecta =
        resultadoSecuencial &&
        resultadoParalelo &&
        std::strcmp(encontradaSecuencial, clavePrueba) == 0 &&
        std::strcmp(encontradaParalelo, clavePrueba) == 0;

    std::cout << "Verificacion final: "
              << (coincidenciaCorrecta
                      ? "AMBAS VERSIONES ENCONTRARON LA CLAVE CORRECTA"
                      : "LOS RESULTADOS NO COINCIDEN")
              << "\n";

    delete[] entrada;
    delete[] clavePrueba;
    delete[] primeraCombinacion;
    delete[] ultimaCombinacion;
    delete[] encontradaSecuencial;
    delete[] encontradaParalelo;

    std::cout << "\nPrograma finalizado correctamente.\n";
    std::cout << "Integrantes: Espinoza Hernandez Axel Daniel; "
              << "Martinez Ibarra Alejandro Isaac; Munoz Perez Jaime; "
              << "Ruiz Guerrero Oswaldo Josue.\n";

    return 0;
}
