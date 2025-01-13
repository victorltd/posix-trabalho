#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define FILA_COMANDOS "/fila_comandos"

// Estrutura para enviar mensagens do painel
struct painel_msg {
    int comando;
    char descricao[100];
};

int main() {
    mqd_t mq;
    struct painel_msg msg;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(msg);
    attr.mq_curmsgs = 0;

    mq = mq_open(FILA_COMANDOS, O_WRONLY | O_CREAT, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("Erro ao abrir a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    int opcao;
    while (1) {
        printf("\nPainel do Carro:\n");
        printf("1 - Acelerar\n");
        printf("2 - Frear\n");
        printf("3 - Ligar Farol Baixo\n");
        printf("4 - Ligar Farol Alto\n");
        printf("5 - Seta Direita\n");
        printf("6 - Seta Esquerda\n");
        
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        if (opcao == 0) {
            printf("Saindo do painel...\n");
            break;
        }

        msg.comando = opcao;
        switch (opcao) {
            case 1: strcpy(msg.descricao, "Acelerar"); break;
            case 2: strcpy(msg.descricao, "Frear"); break;
            case 3: strcpy(msg.descricao, "Farol Baixo Ligado"); break;
            case 4: strcpy(msg.descricao, "Farol Alto Ligado"); break;
            case 5: strcpy(msg.descricao, "Seta direita"); break;
            case 6: strcpy(msg.descricao, "Seta esquerda"); break;
            default:
                printf("Comando inválido!\n");
                continue;
        }

        if (mq_send(mq, (const char *)&msg, sizeof(msg), 0) == -1) {
            perror("Erro ao enviar mensagem");
        } else {
            printf("Comando enviado: %s\n", msg.descricao);
        }
    }

    mq_close(mq);
    return 0;
}
