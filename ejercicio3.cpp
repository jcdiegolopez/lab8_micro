#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};
const int numProductos = 5;  
const int MAX_BUFFER = 5;    
const int MAX_SILLAS = 3;    

int buffer[MAX_BUFFER];       
int in = 0;                   
int out = 0;                  
int sillasProducidas = 0;    
int piezasEnBuffer[numProductos] = {0};  // Arreglo para contar piezas sobrantes en el buffer

// Semáforos y mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

// Función simulación de un productor (fabrica una pieza de silla)
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    
    while (true) {
        // Verificar si ya se alcanzó el número máximo de sillas
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        piezaId = rand() % numProductos;  	// Seleccionar una pieza al azar

        sem_wait(&vacios);  				// Espera hasta que hay espacio en el buffer
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        buffer[in] = piezaId;				// Añade la pieza al buffer
        piezasEnBuffer[piezaId]++;          // Incrementa el conteo de la pieza en el buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;  		// Avanza el índice circular del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  				// Incrementa el número de productos disponibles
        
        sleep(1);  							// Simula el tiempo de fabricación
    }
    
    return NULL;
}

// Función simulación de un consumidor (ensambla una silla)
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    int piezas[3] = {0, 0, 0};  // Contador de las piezas necesarias (1 respaldo, 1 asiento, 4 patas)

    while (true) {
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        sem_wait(&llenos);  				// Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        // Retirar una pieza del buffer
        piezaId = buffer[out];
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        piezasEnBuffer[piezaId]--;  // Decrementa el conteo de piezas en el buffer
        out = (out + 1) % MAX_BUFFER;  		// Avanza en el índice circular del buffer

        // Contar las piezas necesarias para una silla
        if (productos[piezaId] == "Pata") {
            piezas[2]++;
        } else if (productos[piezaId] == "Respaldo") {
            piezas[0]++;
        } else if (productos[piezaId] == "Asiento") {
            piezas[1]++;
        }

        // Si tenemos las piezas completas (1 respaldo, 1 asiento y 4 patas), ensamblamos una silla
        if (piezas[0] >= 1 && piezas[1] >= 1 && piezas[2] >= 4) {
            sillasProducidas++;
            piezas[0]--; piezas[1]--; piezas[2] -= 4;  // Usamos las piezas para ensamblar una silla
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  				// Incrementa el número de espacios vacíos
        
        sleep(2);  							// Simula el tiempo de ensamblaje
    }

    return NULL;
}

int main() {
    int numProductores, numConsumidores;

    // Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  
    int idProductores[100], idConsumidores[100];    

    // Inicializa semáforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);  
    sem_init(&llenos, 0, 0);           
    pthread_mutex_init(&mutex, NULL);

    // Crea hilos productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Crea hilos consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Espera a que los hilos terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    // Reporte final
    cout << "\n--- Reporte Final ---" << endl;
    cout << "Sillas ensambladas: " << sillasProducidas << endl;
    cout << "Piezas sobrantes en el buffer: " << endl;
    for (int i = 0; i < numProductos; ++i) {
        cout << productos[i] << ": " << piezasEnBuffer[i] << endl;
    }

    return 0;
}
