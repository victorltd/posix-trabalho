#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <mqueue.h>

#define SHARED_MEMORY_NAME "/mem_sensores" // Nome da memória compartilhada
#define SHARED_MEMORY_SIZE sizeof(DadosSensores) // Tamanho da memória compartilhada
#define MESSAGE_QUEUE_NAME "/fila_comandos" // Nome da fila de mensagens
#define MESSAGE_QUEUE_SIZE sizeof(PainelMsg) // Tamanho da fila de mensagens

// Estrutura para armazenar os dados dos sensores
typedef struct {
    float velocidade;
    float rpm;
    float temperatura;
    float proximidade;
    int comando; // Novo campo para comando
    pthread_mutex_t mutex; // Proteção para acesso sincronizado
} DadosSensores;

// Estrutura para mensagens do painel
typedef struct {
    int comando;
    char descricao[100];
} PainelMsg;

// Estados internos
typedef struct {
    float estado_velocidade;
    float estado_rpm;
    float estado_temperatura;
} EstadosInternos;

EstadosInternos estados = {0}; // Inicializa os estados internos

// Função para simular inércia física
void atualizar_estados(int comando) {
    float aceleracao = 0.0;
    
    if (comando == 1) { // Acelerar
        aceleracao = 5.0; // Incremento de velocidade
    } else if (comando == 2) { // Frear
        aceleracao = -7.0; // Redução de velocidade
    } else {
        aceleracao = -0.5; // Resistência natural (desaceleração leve)
    }

    // Atualizar velocidade com limite inferior de 0
    estados.estado_velocidade += aceleracao;
    if (estados.estado_velocidade < 0) {
        estados.estado_velocidade = 0;
    }

    // Atualizar RPM proporcional à velocidade
    estados.estado_rpm = estados.estado_velocidade * 40; // Exemplo de relação RPM/velocidade

    // Atualizar temperatura baseada no RPM (quanto maior, mais aquece)
    estados.estado_temperatura += (estados.estado_rpm / 1000.0) * 0.2;
    estados.estado_temperatura -= 0.1; // Resfriamento passivo
    if (estados.estado_temperatura < 20.0) {
        estados.estado_temperatura = 20.0; // Temperatura mínima
    }
}

// Função para ler comandos da memória compartilhada
void *ler_comandos(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        int comando = sensores->comando; // Lê o comando da memória compartilhada
        sensores->comando = 0; // Resetar comando após leitura
        pthread_mutex_unlock(&sensores->mutex);

        if (comando != 0) { // Se houver um comando, atualiza os estados
            atualizar_estados(comando);
        }

        sleep(1); // Intervalo entre leituras
    }
    return NULL;
}

// Funções dos sensores

// Função para simular o sensor de velocidade
void *sensor_velocidade(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->velocidade = estados.estado_velocidade; // Atualiza a velocidade na memória compartilhada
        printf("[Sensor Velocidade] Nova leitura: %.2f km/h\n", sensores->velocidade);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1); // Simula intervalo entre leituras
    }
    return NULL;
}

// Função para simular o sensor de RPM
void *sensor_rpm(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->rpm = estados.estado_rpm; // Atualiza o RPM na memória compartilhada
        //printf("[Sensor RPM] Nova leitura: %.2f RPM\n", sensores->rpm);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1); // Simula intervalo entre leituras
    }
    return NULL;
}

// Função para simular o sensor de temperatura
void *sensor_temperatura(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->temperatura = estados.estado_temperatura; // Atualiza a temperatura na memória compartilhada
        //printf("[Sensor Temperatura] Nova leitura: %.2f ºC\n", sensores->temperatura);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1); // Simula intervalo entre leituras
    }
    return NULL;
}

// Função para simular o sensor de proximidade
void *sensor_proximidade(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->proximidade = (rand() % 100) + ((float)rand() / RAND_MAX); // Gera um valor aleatório para proximidade
        //printf("[Sensor Proximidade] Nova leitura: %.2f m\n", sensores->proximidade);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1); // Simula intervalo entre leituras
    }
    return NULL;
}

int main() {
    pthread_t thread_velocidade, thread_rpm, thread_temperatura, thread_proximidade, thread_comandos;
    DadosSensores sensores = {0}; // Inicializa a estrutura de sensores

    pthread_mutex_init(&sensores.mutex, NULL); // Inicializa o mutex

    // Criar memória compartilhada
    int fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("[Simulador] Erro ao criar memória compartilhada");
        exit(1);
    }
    ftruncate(fd, SHARED_MEMORY_SIZE); // Define o tamanho da memória compartilhada
    DadosSensores *shared_mem = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("[Simulador] Erro ao mapear memória compartilhada");
        exit(1);
    }
    memcpy(shared_mem, &sensores, sizeof(DadosSensores)); // Copia os dados iniciais para a memória compartilhada

    // Criar threads para os sensores e leitura de comandos
    pthread_create(&thread_velocidade, NULL, sensor_velocidade, (void *)shared_mem);
    pthread_create(&thread_rpm, NULL, sensor_rpm, (void *)shared_mem);
    pthread_create(&thread_temperatura, NULL, sensor_temperatura, (void *)shared_mem);
    pthread_create(&thread_proximidade, NULL, sensor_proximidade, (void *)shared_mem);
    pthread_create(&thread_comandos, NULL, ler_comandos, (void *)shared_mem);

    // Aguardar threads
    pthread_join(thread_velocidade, NULL);
    pthread_join(thread_rpm, NULL);
    pthread_join(thread_temperatura, NULL);
    pthread_join(thread_proximidade, NULL);
    pthread_join(thread_comandos, NULL);

    pthread_mutex_destroy(&sensores.mutex); // Destrói o mutex
    munmap(shared_mem, SHARED_MEMORY_SIZE); // Desmapeia a memória compartilhada
    close(fd); // Fecha o descritor de arquivo
    shm_unlink(SHARED_MEMORY_NAME); // Remove a memória compartilhada

    return 0;
}
