#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_CARROS 8
#define CAPACIDADE_FILA 4

pthread_mutex_t mutex_bomba_gasolina;
pthread_mutex_t mutex_bomba_alcool;
pthread_mutex_t mutex_fila;
pthread_cond_t cond_fila;

int fila[CAPACIDADE_FILA];
int proximo_carro = 1;

int carro_gasolina = 0;
int carro_alcool = 0;

void imprimirEstadoPosto() {
    printf("  - Fila: ");
    for (int i = 0; i < CAPACIDADE_FILA; ++i) {
        if (fila[i] == 0) {
            printf("Vazio ");
        } else {
            printf("Carro %d ", fila[i]);
        }
    }
  printf("\n");


  printf("|GAS-STATION----ALC-GAS-|\n");
  printf("|                |   |  |\n");
  printf("|               c%02d c%02d |\n", carro_alcool, carro_gasolina);
  printf("|                       |\n");
  printf("|-----------------------|\n");
}

void* threadCarro(void* arg) {
    int carro_id = *(int*)arg;

    // Chegada ao posto de gasolina
    //printf("Carro %d chegou ao posto de gasolina.\n", carro_id);

    // Verifica a disponibilidade da bomba de gasolina ou álcool
    if (carro_id % 2 == 0) {
        pthread_mutex_lock(&mutex_bomba_alcool);
        //printf("Carro %d está usando a bomba de álcool.\n", carro_id);
        carro_alcool = carro_id;
    } else {
        pthread_mutex_lock(&mutex_bomba_gasolina);
        //printf("Carro %d está usando a bomba de gasolina.\n", carro_id);
        carro_gasolina = carro_id;
    }

    pthread_mutex_lock(&mutex_fila);

    // Verifica se a fila está cheia
    int posicao_fila = -1;
    for (int i = 0; i < CAPACIDADE_FILA; ++i) {
        if (fila[i] == 0) {
            posicao_fila = i;
            break;
        }
    }

    if (posicao_fila == -1) {
        // Fila cheia, carro vai embora
        printf("Carro %d foi embora pois não havia lugar na fila.\n", carro_id);
        if (carro_id % 2 == 0) {
            pthread_mutex_unlock(&mutex_bomba_alcool);
            carro_alcool = 0;
        } else {
            pthread_mutex_unlock(&mutex_bomba_gasolina);
            carro_gasolina = 0;
        }
        pthread_mutex_unlock(&mutex_fila);
        return NULL;
    }

    fila[posicao_fila] = carro_id; // Entra na fila

    pthread_cond_broadcast(&cond_fila); // Notifica que há carros na fila

    pthread_mutex_unlock(&mutex_fila);

    // Simula o tempo de abastecimento
    sleep(6);

    pthread_mutex_lock(&mutex_fila);

    fila[posicao_fila] = 0; // Remove da fila

    proximo_carro++;
    pthread_cond_broadcast(&cond_fila);

    pthread_mutex_unlock(&mutex_fila);

    // Libera a bomba de gasolina ou álcool
    if (carro_id % 2 == 0) {
        pthread_mutex_unlock(&mutex_bomba_alcool);
        carro_alcool = 0;
    } else {
        pthread_mutex_unlock(&mutex_bomba_gasolina);
        carro_gasolina = 0;
    }

    // Finaliza o uso da bomba de gasolina ou álcool e sai do posto de gasolina
    //printf("Carro %d finalizou o abastecimento e saiu do posto de gasolina.\n", carro_id);

    return NULL;
}

int main() {
    pthread_t carros[NUM_CARROS];
    int carro_ids[NUM_CARROS];

    pthread_mutex_init(&mutex_bomba_gasolina, NULL);
    pthread_mutex_init(&mutex_bomba_alcool, NULL);
    pthread_mutex_init(&mutex_fila, NULL);
    pthread_cond_init(&cond_fila, NULL);

    for (int i = 0; i < CAPACIDADE_FILA; ++i) {
        fila[i] = 0; // Inicializa a fila como vazia
    }

    // Cria as threads dos carros
    for (int i = 0; i < NUM_CARROS; ++i) {
        carro_ids[i] = i + 1;
        pthread_create(&carros[i], NULL, threadCarro, (void*)&carro_ids[i]);
        sleep(2);
      imprimirEstadoPosto();
    }

    // Aguarda a finalização das threads
    for (int i = 0; i < NUM_CARROS; ++i) {
        pthread_join(carros[i], NULL);
    }

    pthread_mutex_destroy(&mutex_bomba_gasolina);
    pthread_mutex_destroy(&mutex_bomba_alcool);
    pthread_mutex_destroy(&mutex_fila);
    pthread_cond_destroy(&cond_fila);

    return 0;
}