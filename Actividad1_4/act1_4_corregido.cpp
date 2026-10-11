#include <iostream>
#include <omp.h>
#include <ctime>
#include <random>

class BancoCritical
{
private:
    int saldo;

public:
    BancoCritical(int saldo_inicial) : saldo(saldo_inicial) {}

    void depositar(int cantidad, int hilo)
    {
#pragma omp critical
        {
            saldo += cantidad;
            std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad
                      << " | Saldo actual: $" << saldo << "\n";
        }
    }

    void retirar(int cantidad, int hilo)
    {
#pragma omp critical
        {
            if (saldo >= cantidad)
            {
                saldo -= cantidad;
                std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad
                          << " | Saldo actual: $" << saldo << "\n";
            }
            else
            {
                std::cout << "[Hilo " << hilo << "] Intento de retiro: $"
                          << cantidad << " (FONDOS INSUFICIENTES)\n";
            }
        }
    }

    int consultarSaldo()
    {
        int saldo_actual;

#pragma omp critical
        {
            saldo_actual = saldo;
        }

        return saldo_actual;
    }
};

class BancoLock
{
private:
    int saldo;
    omp_lock_t candado;

public:
    BancoLock(int saldo_inicial) : saldo(saldo_inicial)
    {
        omp_init_lock(&candado);
    }

    ~BancoLock()
    {
        omp_destroy_lock(&candado);
    }

    void depositar(int cantidad, int hilo)
    {
        omp_set_lock(&candado);
        saldo += cantidad;

#pragma omp critical(consola)
        {
            std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad
                      << " | Saldo actual: $" << saldo << "\n";
        }

        omp_unset_lock(&candado);
    }

    void retirar(int cantidad, int hilo)
    {
        omp_set_lock(&candado);

        if (saldo >= cantidad)
        {
            saldo -= cantidad;

#pragma omp critical(consola)
            {
                std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad
                          << " | Saldo actual: $" << saldo << "\n";
            }
        }
        else
        {
#pragma omp critical(consola)
            {
                std::cout << "[Hilo " << hilo << "] Intento de retiro: $"
                          << cantidad << " (FONDOS INSUFIENTES)\n";
            }
        }

        omp_unset_lock(&candado);
    }

    int consultarSaldo()
    {
        int saldo_actual;
        omp_set_lock(&candado);
        saldo_actual = saldo;
        omp_unset_lock(&candado);
        return saldo_actual;
    }
};

class BancoAtomic
{
private:
    int saldo;

public:
    BancoAtomic(int saldo_inicial) : saldo(saldo_inicial) {}

    void depositar(int cantidad, int hilo)
    {
#pragma omp atomic
        saldo += cantidad;

#pragma omp critical(consola)
        {
            std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad << "\n";
        }
    }

    void retirar(int cantidad, int hilo)
    {
        bool exito = false;

#pragma omp critical(consola)
        {
            if (saldo >= cantidad)
            {
                saldo -= cantidad;
                exito = true;
            }
        }

#pragma omp critical
        {
            if (exito)
            {
                std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad << "\n";
            }
            else
            {
                std::cout << "[Hilo " << hilo << "] Intento de retiro: $"
                          << cantidad << " (FONDOS INSUFICIENTES)\n";
            }
        }
    }

    int consultarSaldo()
    {
        int saldo_actual;

#pragma omp critical
        {
            saldo_actual = saldo;
        }

        return saldo_actual;
    }
};

int main()
{
    std::cout << "INTEGRANTES DEL EQUIPO\n";
    std::cout << "1. Espinoza Hernandez Axel Daniel\n";
    std::cout << "2. Martinez Ibarra Alejandro Isaac\n";
    std::cout << "3. Munoz Perez Jaime\n";
    std::cout << "4. Ruiz Guerrero Oswaldo Josue\n\n";

    int saldo_inicial;

    std::cout << "Ingrese el saldo inicial de la cuenta: $";
    std::cin >> saldo_inicial;

    if (!std::cin)
    {
        std::cout << "Entrada invalida.\n";
        return 1;
    }

    if (saldo_inicial < 0)
    {
        std::cout << "El saldo inicial no puede ser negativo. Se establecera en $0.\n";
        saldo_inicial = 0;
    }

    BancoLock banco(saldo_inicial);

    std::cout << "\nINICIO DE SIMULACION BANCARIA\n";
    std::cout << "Saldo inicial: $" << saldo_inicial << "\n\n";

    double tiempo_inicio = omp_get_wtime();

#pragma omp parallel
    {
        int id_hilo = omp_get_thread_num();
        unsigned int semilla = static_cast<unsigned int>(std::time(nullptr))
                             ^ static_cast<unsigned int>(id_hilo * 2654435761U);
        std::mt19937 generador(semilla);
        std::uniform_int_distribution<int> dist_operacion(0, 2);
        std::uniform_int_distribution<int> dist_cantidad(100, 599);

        for (int i = 0; i < 5; i++)
        {
            int operacion = dist_operacion(generador);
            int cantidad = dist_cantidad(generador);

            if (operacion == 0)
            {
                banco.depositar(cantidad, id_hilo);
            }
            else if (operacion == 1)
            {
                banco.retirar(cantidad, id_hilo);
            }
            else
            {
                int saldo_consultado = banco.consultarSaldo();

#pragma omp critical(consola)
                {
                    std::cout << "[Hilo " << id_hilo
                              << "] Consulto saldo. Saldo consultado: $"
                              << saldo_consultado << "\n";
                }
            }
        }
    }

    double tiempo_fin = omp_get_wtime();

    std::cout << "\nFIN DE SIMULACION\n";
    std::cout << "Saldo final exacto: $" << banco.consultarSaldo() << "\n";
    std::cout << "Tiempo de ejecucion: " << tiempo_fin - tiempo_inicio
              << " segundos.\n";

    std::cout << "\nSIMULACION COMPLETADA POR\n";
    std::cout << "1. Espinoza Hernandez Axel Daniel\n";
    std::cout << "2. Martinez Ibarra Alejandro Isaac\n";
    std::cout << "3. Munoz Perez Jaime\n";
    std::cout << "4. Ruiz Guerrero Oswaldo Josue\n";

    return 0;
}
