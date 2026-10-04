#include <iostream> // Incluye la libreria para entrada y salida de datos en consola
#include <omp.h> // Incluye la libreria de OpenMP para manejar hilos y paralelismo
#include <cstdlib> // Incluye la libreria estandar de C para funciones generales
#include <ctime> // Incluye la libreria de tiempo para obtener la hora del sistema
#include <random> // Incluye la libreria para generacion de numeros aleatorios


// Integrantes del equipo:
// 1. Espinoza Hernandez Axel Daniel
// 2. Martinez Ibarra Alejandro Isaac
// 3. Munoz Perez Jaime
// 4. Ruiz Guerrero Oswaldo Josue

// VERSIÓN 1: Sincronización con CRITICAL

class BancoCritical { // Declara la clase BancoCritical para sincronizar mediante bloques critical
private: // Define la seccion de miembros privados de la clase
    int saldo; // Declara la variable entera privada para almacenar el saldo de la cuenta
public: // Define la seccion de miembros publicos de la clase
    BancoCritical(int saldo_inicial) : saldo(saldo_inicial) {} // Constructor que inicializa la variable saldo con el valor recibido

    void depositar(int cantidad, int hilo) { // Metodo para realizar un deposito en la cuenta
        #pragma omp critical // Directiva para asegurar que solo un hilo ejecute el bloque a la vez
        { // Inicio del bloque de ejecucion exclusiva
            saldo += cantidad; // Incrementa el saldo sumandole la cantidad depositada
            std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad << " | Saldo actual: $" << saldo << "\n"; // Imprime los datos de la transaccion
        } // Fin del bloque de ejecucion exclusiva
    } // Fin del metodo depositar

    void retirar(int cantidad, int hilo) { // Metodo para realizar un retiro de la cuenta
        #pragma omp critical // Directiva para asegurar que solo un hilo ejecute el bloque a la vez
        { // Inicio del bloque de ejecucion exclusiva
            if (saldo >= cantidad) { // Comprueba si el saldo es suficiente para realizar el retiro
                saldo -= cantidad; // Resta la cantidad al saldo actual
                std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad << " | Saldo actual: $" << saldo << "\n"; // Imprime el retiro exitoso
            } else { // Bloque a ejecutar si no hay fondos suficientes
                std::cout << "[Hilo " << hilo << "] Intento de retiro: $" << cantidad << " (FONDOS INSUFICIENTES)\n"; // Imprime el mensaje de fondos insuficientes
            } // Fin de la validacion de fondos
        } // Fin del bloque de ejecucion exclusiva
    } // Fin del metodo retirar

    int consultarSaldo() { // Metodo para obtener el saldo actual de la cuenta
        int saldo_actual; // Declara la variable local para almacenar la copia del saldo
        #pragma omp critical // Directiva para asegurar acceso exclusivo a la lectura de la variable saldo
        { // Inicio del bloque de ejecucion exclusiva
            saldo_actual = saldo; // Copia el valor de saldo en la variable local
        } // Fin del bloque de ejecucion exclusiva
        return saldo_actual; // Devuelve el saldo consultado al llamador
    } // Fin del metodo consultarSaldo
}; // Fin de la clase BancoCritical


// Integrantes del equipo:
// 1. Espinoza Hernandez Axel Daniel
// 2. Martin Ibarra Alejandro Isaac
// 3. Munoz Perez Jaime
// 4. Ruiz Guerrero Oswaldo Josue

// VERSIÓN 2: Sincronización con LOCK

class BancoLock { // Declara la clase BancoLock para sincronizar mediante candados de OpenMP
private: // Define la seccion de miembros privados de la clase
    int saldo; // Declara la variable entera para almacenar el saldo
    omp_lock_t candado; // Declara la variable del candado tipo OpenMP
public: // Define la seccion de miembros publicos de la clase
    BancoLock(int saldo_inicial) : saldo(saldo_inicial) { // Constructor de la clase
        omp_init_lock(&candado); // Inicializa la variable de candado antes de usarla
    } // Fin del constructor

    ~BancoLock() { // Destructor de la clase
        omp_destroy_lock(&candado); // Libera los recursos del candado al destruir el objeto
    } // Fin del destructor

    void depositar(int cantidad, int hilo) { // Metodo para depositar dinero en la cuenta
        omp_set_lock(&candado); // Bloquea el candado para que otros hilos esperen
        saldo += cantidad; // Incrementa el saldo con la cantidad depositada
        #pragma omp critical // Directiva para evitar que la salida en consola se altere
        std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad << " | Saldo actual: $" << saldo << "\n"; // Imprime la informacion del deposito
        omp_unset_lock(&candado); // Libera el candado para permitir acceso a otros hilos
    } // Fin del metodo depositar

    void retirar(int cantidad, int hilo) { // Metodo para retirar dinero de la cuenta
        omp_set_lock(&candado); // Bloquea el candado antes de evaluar o modificar el saldo
        if (saldo >= cantidad) { // Verifica si se cuenta con el saldo necesario
            saldo -= cantidad; // Resta la cantidad depositada al saldo
            #pragma omp critical // Directiva para imprimir de forma ordenada en consola
            std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad << " | Saldo actual: $" << saldo << "\n"; // Imprime el retiro realizado
        } else { // Si no hay fondos suficientes
            #pragma omp critical // Directiva para imprimir de forma ordenada en consola
            std::cout << "[Hilo " << hilo << "] Intento de retiro: $" << cantidad << " (FONDOS INSUFICIENTES)\n"; // Imprime el mensaje de rechazo
        } // Fin de la condicion
        omp_unset_lock(&candado); // Libera el candado para otros hilos
    } // Fin del metodo retirar

    int consultarSaldo() { // Metodo para obtener el saldo actual
        int saldo_actual; // Declara una variable local para guardar la copia
        omp_set_lock(&candado); // Bloquea el candado antes de leer la variable
        saldo_actual = saldo; // Asigna el valor del saldo a la variable local
        omp_unset_lock(&candado); // Libera el candado despues de la lectura
        return saldo_actual; // Retorna el valor obtenido
    } // Fin del metodo consultarSaldo
}; // Fin de la clase BancoLock


// Integrantes del equipo:
// 1. [Espinoza Hernandez Axel Daniel]
// 2. [Martin Ibarra Alejandro Isaac]
// 3. [Munoz Perez Jaime]
// 4. [Ruiz Guerrero Oswaldo Josue]

// VERSIÓN 3: Sincronización con ATOMIC

class BancoAtomic { // Declara la clase BancoAtomic para sincronizar mediante operaciones atomicas
private: // Define la seccion de miembros privados de la clase
    int saldo; // Declara la variable entera privada para el saldo
public: // Define la seccion de miembros publicos de la clase
    BancoAtomic(int saldo_inicial) : saldo(saldo_inicial) {} // Constructor que asigna el saldo inicial

    void depositar(int cantidad, int hilo) { // Metodo para realizar depositos
        #pragma omp atomic // Garantiza que la operacion de suma en memoria sea atomica
        saldo += cantidad; // Asigna y suma la cantidad al saldo de forma segura

        #pragma omp critical // Garantiza impresion limpia en consola sin interferencia
        std::cout << "[Hilo " << hilo << "] Deposito: $" << cantidad << "\n"; // Imprime la operacion
    } // Fin del metodo depositar

    void retirar(int cantidad, int hilo) { // Metodo para retirar dinero
        bool exito = false; // Declara e inicializa la bandera para saber si se concreto el retiro

        #pragma omp critical // Inicia bloque para validar el saldo de forma atómica respecto a otros retiros
        { // Inicio de bloque
            if (saldo >= cantidad) { // Evalua si hay dinero suficiente
                #pragma omp atomic // Ejecuta de forma atomica la resta
                saldo -= cantidad; // Resta la cantidad al saldo actual
                exito = true; // Cambia el estado del retiro a exitoso
            } // Fin de la evaluacion
        } // Fin de bloque

        #pragma omp critical // Sincroniza la salida en pantalla
        { // Inicio de bloque
            if (exito) std::cout << "[Hilo " << hilo << "] Retiro: $" << cantidad << "\n"; // Imprime confirmed cuando el retiro fue hecho
            else std::cout << "[Hilo " << hilo << "] Intento de retiro: $" << cantidad << " (FONDOS INSUFICIENTES)\n"; // Imprime rechazo si fallo
        } // Fin de bloque
    } // Fin del metodo retirar

    int consultarSaldo() { // Metodo para consultar el saldo
        int saldo_actual; // Variable local para guardar el resultado de lectura
        #pragma omp atomic read // Lee el saldo directamente de memoria en una sola instruccion atomica
        saldo_actual = saldo; // Almacena la lectura atomica en la variable local
        return saldo_actual; // Devuelve la lectura realizada
    } // Fin del metodo consultarSaldo
}; // Fin de la clase BancoAtomic

// ==========================================
// CLASE TestBanco (Función Principal)
// ==========================================
int main() { // Funcion principal del programa
    // 1. Impresión de nombres al inicio
    std::cout << "INTEGRANTES DEL EQUIPO \n"; // Muestra el encabezado de integrantes
    std::cout << "1. [Espinoza Hernandez Axel Daniel]\n"; // Imprime integrante 1
    std::cout << "2. [Martin Ibarra Alejandro Isaac]\n"; // Imprime integrante 2
    std::cout << "3. [Munoz Perez Jaime]\n"; // Imprime integrante 3
    std::cout << "4. [Ruiz Guerrero Oswaldo Josue]\n"; // Imprime integrante 4


    int saldo_inicial; // Declara la variable para recibir el saldo de entrada

    std::cout << "Ingrese el saldo inicial de la cuenta: $"; // Pide al usuario el saldo inicial
    std::cin >> saldo_inicial; // Lee el dato ingresado por el usuario

    if (saldo_inicial < 0) { // Evalua si el saldo dado es negativo
        std::cout << "El saldo inicial no puede ser negativo. Se establecera en $0.\n"; // Muestra advertencia
        saldo_inicial = 0; // Corrige el valor asignandole cero
    } // Fin de validacion del saldo

    BancoLock banco(saldo_inicial); // Instancia un objeto de BancoLock usando el saldo inicial

    std::cout << "\nINICIO DE SIMULACION BANCARIA \n"; // Muestra encabezado de inicio de simulacion
    std::cout << "Saldo inicial: $" << saldo_inicial << "\n\n"; // Muestra el saldo registrado para iniciar

    double tiempo_inicio = omp_get_wtime(); // Registra el tiempo inicial usando el reloj de OpenMP

    #pragma omp parallel // Inicia la region paralela usando el numero de hilos por defecto
    { // Inicio de la region paralela
        int id_hilo = omp_get_thread_num(); // Obtiene el identificador unico del hilo ejecutor

        std::mt19937 generador(time(NULL) ^ id_hilo); // Crea generador de numeros aleatorios con semilla unica por hilo
        std::uniform_int_distribution<int> dist_operacion(0, 2); // Define distribucion uniforme para elegir operacion (0, 1 o 2)
        std::uniform_int_distribution<int> dist_cantidad(100, 599); // Define distribucion uniforme para la cantidad de 100 a 599

        for (int i = 0; i < 5; i++) { // Bucle que ejecuta 5 operaciones por cada hilo
            int operacion = dist_operacion(generador); // Genera la operacion a realizar
            int cantidad = dist_cantidad(generador); // Genera el monto de la operacion

            if (operacion == 0) { // Si el numero es 0, la operacion es un deposito
                banco.depositar(cantidad, id_hilo); // Llama al metodo depositar
            } // Fin de opcion deposito
            else if (operacion == 1) { // Si el numero es 1, la operacion es un retiro
                banco.retirar(cantidad, id_hilo); // Llama al metodo retirar
            } // Fin de opcion retiro
            else { // Si el numero es 2, la operacion es una consulta
                int s = banco.consultarSaldo(); // Obtiene el saldo llamando a consultarSaldo
                #pragma omp critical // Sincroniza la consola para que el texto salga en orden
                std::cout << "[Hilo " << id_hilo << "] Consulto saldo. Saldo consultado: $" << s << "\n"; // Imprime la consulta realizada
            } // Fin de opcion consulta
        } // Fin del bucle for
    } // Fin de la region paralela

    double tiempo_fin = omp_get_wtime(); // Registra el tiempo de fin del procesamiento paralelo

    std::cout << "\n FIN DE SIMULACION \n"; // Muestra texto de cierre de simulacion
    std::cout << "Saldo final exacto: $" << banco.consultarSaldo() << "\n"; // Muestra el saldo definitivo tras las transacciones
    std::cout << "Tiempo de ejecucion: " << (tiempo_fin - tiempo_inicio) << " segundos.\n"; // Calcula y muestra la duracion del proceso

    // 2. Impresión de nombres al final
    std::cout << "\nSIMULACION COMPLETADA POR \n"; // Muestra encabezado final
    std::cout << "1. [Espinoza Hernandez Axel Daniel]\n"; // Imprime integrante 1
    std::cout << "2. [Martin Ibarra Alejandro Isaac]\n"; // Imprime integrante 2
    std::cout << "3. [Munoz Perez Jaime]\n"; // Imprime integrante 3
    std::cout << "4. [Ruiz Guerrero Oswaldo Josue]\n"; // Imprime integrante 4
    

    return 0; // Finaliza la ejecucion del programa retornando cero
} // Fin de la funcion main