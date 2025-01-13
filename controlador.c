#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <mqueue.h>

typedef struct {
    float velocidade;
    float rpm;
    float temperatura;
    float proximidade;
    int comando; // Novo campo para comando
    pthread_mutex_t mutex;
} DadosSensores;

typedef struct {
    int comando;
    char descricao[100];
} PainelMsg;

#define SHARED_MEMORY_NAME "/mem_sensores"
#define SHARED_MEMORY_SIZE sizeof(DadosSensores)
#define MESSAGE_QUEUE_NAME "/fila_comandos"
#define MESSAGE_QUEUE_SIZE sizeof(PainelMsg)

volatile sig_atomic_t executar = 1;
int ocorrencias_velocidade = 0;
int ocorrencias_acelerar = 0;
int ocorrencias_frear = 0;
int ocorrencias_seta_direita = 0;
int ocorrencias_seta_esquerda = 0;
int ocorrencias_farol_baixo = 0;
int ocorrencias_farol_alto = 0;

void signal_handler(int signo) {
    if (signo == SIGUSR1) {
        executar = 0; // Pausar
    } else if (signo == SIGCONT) {
        executar = 1; // Retomar
    } else if (signo == SIGUSR2) {
        executar = -1; // Encerrar
    }
}

// Adicione essa função para finalizar o programa corretamente
void finalizar_programa(DadosSensores *sensores, pthread_t thread_comandos, int fd) {
    // Finalizar thread
    pthread_cancel(thread_comandos);
    pthread_join(thread_comandos, NULL);

    // Desmapear memória compartilhada
    munmap(sensores, SHARED_MEMORY_SIZE);
    close(fd);
    shm_unlink(SHARED_MEMORY_NAME);

    // Gerar relatório consolidado
    printf("\n[Controlador] Relatório Consolidado:\n");
    printf("Número de ocorrências em que a velocidade ultrapassou o limite (50 km/h): %d\n", ocorrencias_velocidade);
    printf("Número de vezes que o acelerador foi acionado: %d\n", ocorrencias_acelerar);
    printf("Número de vezes que o freio foi acionado: %d\n", ocorrencias_frear);
    printf("Número de vezes que a seta direita foi acionada: %d\n", ocorrencias_seta_direita);
    printf("Número de vezes que a seta esquerda foi acionada: %d\n", ocorrencias_seta_esquerda);
    printf("Número de vezes que o farol baixo foi acionado: %d\n", ocorrencias_farol_baixo);
    printf("Número de vezes que o farol alto foi acionado: %d\n", ocorrencias_farol_alto);

    printf("[Controlador] Encerrado.\n");
    exit(0);
}

// Função para processar comandos recebidos do painel
void *processar_comandos(void *arg) {
    mqd_t mq = mq_open(MESSAGE_QUEUE_NAME, O_RDONLY | O_CREAT, 0666, NULL);
    if (mq == (mqd_t)-1) {
        perror("[Controlador] Erro ao abrir fila de mensagens");
        pthread_exit(NULL);
    }

    PainelMsg comando;
    DadosSensores *sensores = (DadosSensores *)arg;
    while (executar != -1) {
        if (mq_receive(mq, (char *)&comando, sizeof(comando), NULL) >= 0) {
            printf("[Controlador] Comando recebido: %s\n", comando.descricao);

            pthread_mutex_lock(&sensores->mutex);
            sensores->comando = comando.comando; // Enviar comando para o simulador
            pthread_mutex_unlock(&sensores->mutex);

            // Incrementar contadores específicos
            switch (comando.comando) {
                case 1: ocorrencias_acelerar++; break;
                case 2: ocorrencias_frear++; break;
                case 3: ocorrencias_farol_baixo++; break;
                case 4: ocorrencias_farol_alto++; break;
                case 5: ocorrencias_seta_direita++; break;
                case 6: ocorrencias_seta_esquerda++; break;
                default: break;
            }
        }
    }

    mq_close(mq);
    mq_unlink(MESSAGE_QUEUE_NAME);
    pthread_exit(NULL);
}

int main() {
    signal(SIGUSR1, signal_handler);
    signal(SIGCONT, signal_handler);
    signal(SIGUSR2, signal_handler);

    int fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("[Controlador] Erro ao abrir memória compartilhada");
        exit(1);
    }

    DadosSensores *sensores = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sensores == MAP_FAILED) {
        perror("[Controlador] Erro ao mapear memória compartilhada");
        exit(1);
    }

    printf("[Controlador] Memória compartilhada conectada.\n");

    // Criar thread para processar comandos do painel
    pthread_t thread_comandos;
    if (pthread_create(&thread_comandos, NULL, processar_comandos, (void *)sensores) != 0) {
        perror("[Controlador] Erro ao criar thread de comandos");
        exit(1);
    }

    // Loop principal para leitura dos sensores
    while (executar != -1) {
        if (executar == 0) { // Pausar
            pause(); // Aguarda até receber SIGCONT para continuar
        }

        // Ler dados da memória compartilhada
        pthread_mutex_lock(&sensores->mutex);
        float velocidade = sensores->velocidade;
        float rpm = sensores->rpm;
        float temperatura = sensores->temperatura;
        float proximidade = sensores->proximidade;
        pthread_mutex_unlock(&sensores->mutex);

        // Imprimir os dados lidos
        printf("[Controlador] Sensores:\n");
        printf("  Velocidade: %.2f km/h\n", velocidade);
        printf("  RPM: %.2f\n", rpm);
        printf("  Temperatura: %.2f ºC\n", temperatura);
        printf("  Proximidade: %.2f m\n", proximidade);

        // Verificar se a velocidade ultrapassou o limite
        if (velocidade > 50.0) {
            ocorrencias_velocidade++;
        }

        sleep(1); // Simular intervalo de processamento
    }

    // Encerrar programa ao receber SIGUSR2
    finalizar_programa(sensores, thread_comandos, fd);

    return 0;
}
