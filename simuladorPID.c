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


//removi o que estava antes
#define SHARED_MEMORY_NAME "/mem_sensores"
#define SHARED_MEMORY_SIZE sizeof(DadosSensores)
#define MESSAGE_QUEUE_NAME "/fila_comandos"
#define MESSAGE_QUEUE_SIZE sizeof(PainelMsg)

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

// Estrutura para estados internos
typedef struct {
    float estado_velocidade;
    float estado_rpm;
    float estado_temperatura;
} EstadosInternos;

EstadosInternos estados = {0};

// Estrutura para controlador PID
typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float integral;
    float previous_error;
} PIDController;

PIDController pid_velocidade = {0.1, 0.01, 0.05, 0, 0, 0};

// Função para calcular a saída do controlador PID
float calcular_pid(PIDController *pid, float medida_atual) {
    float erro = pid->setpoint - medida_atual;
    pid->integral += erro;
    float derivativo = erro - pid->previous_error;
    pid->previous_error = erro;
    return pid->kp * erro + pid->ki * pid->integral + pid->kd * derivativo;
}

// Função para simular inércia física
void atualizar_estados(int comando) {
    if (comando == 1) { // Acelerar
        pid_velocidade.setpoint += 5.0; // Incremento de velocidade
    } else if (comando == 2) { // Frear
        pid_velocidade.setpoint -= 5.0; // Decremento de velocidade
        if (pid_velocidade.setpoint < 0) pid_velocidade.setpoint = 0;
    }

    float ajuste_velocidade = calcular_pid(&pid_velocidade, estados.estado_velocidade);
    estados.estado_velocidade += ajuste_velocidade;
    if (estados.estado_velocidade < 0) estados.estado_velocidade = 0;

    estados.estado_rpm = estados.estado_velocidade * 100;
    estados.estado_temperatura = 20 + (estados.estado_rpm / 1000);
}

void *simulador(void *arg) {
    DadosSensores *sensores = (DadosSensores *)arg;

    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        atualizar_estados(sensores->comando);
        sensores->velocidade = estados.estado_velocidade;
        sensores->rpm = estados.estado_rpm;
        sensores->temperatura = estados.estado_temperatura;
        pthread_mutex_unlock(&sensores->mutex);

        sleep(1);
    }

    return NULL;
}

int main() {
    int fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Erro ao abrir memória compartilhada");
        exit(1);
    }

    if (ftruncate(fd, SHARED_MEMORY_SIZE) == -1) {
        perror("Erro ao definir tamanho da memória compartilhada");
        exit(1);
    }

    DadosSensores *sensores = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sensores == MAP_FAILED) {
        perror("Erro ao mapear memória compartilhada");
        exit(1);
    }

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sensores->mutex, &mutex_attr);

    pthread_t thread_simulador;
    if (pthread_create(&thread_simulador, NULL, simulador, (void *)sensores) != 0) {
        perror("Erro ao criar thread do simulador");
        exit(1);
    }

    pthread_join(thread_simulador, NULL);

    munmap(sensores, SHARED_MEMORY_SIZE);
    close(fd);
    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}