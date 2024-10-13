#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

double cuentaBancaria = 100000.00; 
double* cantidadRetirada; 
pthread_mutex_t mutex;               
sem_t semaforo;                      

// Función que simula el retiro de dinero por un cliente
void* retirarDinero(void* arg) {
    int id = *(int*)arg;  
    double cantidadCuenta = cantidadRetirada[id - 1]; 

    sem_wait(&semaforo);  

    pthread_mutex_lock(&mutex);  

    cout << "Cliente " << id << " esta intentando retirar " << cantidadCuenta << " de la cuenta " << endl;

    // Verifica si hay suficiente saldo para el retiro
    if (cuentaBancaria >= cantidadCuenta) {
        cuentaBancaria -= cantidadCuenta;
        cout << "Cliente " << id << " retiro " << cantidadCuenta 
             << " Saldo restante: " << cuentaBancaria << endl;
    } else {
        cout << "Cliente " << id << " no pudo retirar dinero. Saldo insuficiente." << endl;
    }

    pthread_mutex_unlock(&mutex);  
    sem_post(&semaforo);  

    return NULL;
}

int main() {
    int numClientes;

    cout << "Ingrese la cantidad de clientes: ";
    cin >> numClientes;

    //Arreglo para almacenar los hilos, ID y saldo de los clientes
    pthread_t clientes[numClientes];  
    int idClientes[numClientes];      
    cantidadRetirada = new double[numClientes]; 

    //Inicializar mutex y semáforo
    pthread_mutex_init(&mutex, NULL);
    sem_init(&semaforo, 0, 1);  

    //Solicitar la cantidad a retirar
    for (int i = 0; i < numClientes; ++i) {
        cout << "Ingrese la cantidad que el cliente " << (i + 1) << " desee retirar: ";
        cin >> cantidadRetirada[i];
        idClientes[i] = i + 1;  
    }

    //Inicia la simulación de los retiros
    for (int i = 0; i < numClientes; ++i) {
        // Crear un hilo para cada cliente, pasando el ID del cliente como argumento
        pthread_create(&clientes[i], NULL, retirarDinero, &idClientes[i]);
    }

    //Esperar a que todos los hilos terminen
    for (int i = 0; i < numClientes; ++i) {
        pthread_join(clientes[i], NULL);
    }

    //Mostrar el saldo final 
    cout << "Saldo actual de la cuenta : " << cuentaBancaria << endl;

    // Destruir el mutex y el semáforo
    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaforo);

    delete[] cantidadRetirada;  

    return 0;
}
