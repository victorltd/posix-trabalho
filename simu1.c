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

// Estrutura para armazenar os dados dos sensores
typedef struct {
    float velocidade;
    float rpm;
    float temperatura;
    float proximidade;
    pthread_mutex_t mutex; // Proteção para acesso sincronizado
} DadosSensores;

// Estados internos
typedef struct {
    float estado_velocidade;
    float estado_rpm;
    float estado_temperatura;
} EstadosInternos;

EstadosInternos estados = {0};

#define SHARED_MEMORY_NAME "/mem_sensores"
#define SHARED_MEMORY_SIZE sizeof(DadosSensores)

// Função para simular inércia física
void atualizar_estados(char comando) {
    float aceleracao = 0.0;
    
    if (comando == 'A') { // Acelerar
        aceleracao = 5.0; // Incremento de velocidade
    } else if (comando == 'F') { // Frear
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

// Funções dos sensores
void *sensor_velocidade(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->velocidade = estados.estado_velocidade;
        printf("[Sensor Velocidade] Nova leitura: %.2f km/h\n", sensores->velocidade);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1); // Simula intervalo entre leituras
    }
}

void *sensor_rpm(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->rpm = estados.estado_rpm;
        printf("[Sensor RPM] Nova leitura: %.2f RPM\n", sensores->rpm);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1);
    }
}

void *sensor_temperatura(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->temperatura = estados.estado_temperatura;
        printf("[Sensor Temperatura] Nova leitura: %.2f ºC\n", sensores->temperatura);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1);
    }
}

void *sensor_proximidade(void *dados) {
    DadosSensores *sensores = (DadosSensores *)dados;
    while (1) {
        pthread_mutex_lock(&sensores->mutex);
        sensores->proximidade = (rand() % 100) + ((float)rand() / RAND_MAX);
        printf("[Sensor Proximidade] Nova leitura: %.2f m\n", sensores->proximidade);
        pthread_mutex_unlock(&sensores->mutex);
        sleep(1);
    }
}

void *simular_comandos(void *arg) {
    char comandos[] = {'A', 'F', 'N'}; // Acelerar, Frear, Neutro
    while (1) {
        char comando = comandos[rand() % 3];
        atualizar_estados(comando);
        sleep(2);
    }
}

int main() {
    // Criar ou abrir memória compartilhada
    int fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }
    ftruncate(fd, SHARED_MEMORY_SIZE);

    // Mapear memória compartilhada
    DadosSensores *sensores = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sensores == MAP_FAILED) {
        perror("Erro ao mapear memória compartilhada");
        exit(1);
    }

    // Inicializar mutex na memória compartilhada
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sensores->mutex, &attr);

    // Criar threads para cada sensor
    pthread_t t_velocidade, t_rpm, t_temperatura, t_proximidade, t_comandos;
    pthread_create(&t_velocidade, NULL, sensor_velocidade, (void *)sensores);
    pthread_create(&t_rpm, NULL, sensor_rpm, (void *)sensores);
    pthread_create(&t_temperatura, NULL, sensor_temperatura, (void *)sensores);
    pthread_create(&t_proximidade, NULL, sensor_proximidade, (void *)sensores);
    pthread_create(&t_comandos, NULL, simular_comandos, NULL);

    // Esperar as threads (simulação contínua)
    pthread_join(t_velocidade, NULL);
    pthread_join(t_rpm, NULL);
    pthread_join(t_temperatura, NULL);
    pthread_join(t_proximidade, NULL);
    pthread_join(t_comandos, NULL);

    // Limpar recursos (nunca será alcançado neste exemplo)
    pthread_mutex_destroy(&sensores->mutex);
    munmap(sensores, SHARED_MEMORY_SIZE);
    close(fd);
    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
