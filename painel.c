#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define FILA_COMANDOS "/fila_comandos" // Nome da fila de mensagens

// Estrutura para enviar mensagens do painel
struct painel_msg {
    int comando; // Código do comando
    char descricao[100]; // Descrição do comando
};

int main() {
    mqd_t mq; // Descritor da fila de mensagens
    struct painel_msg msg; // Mensagem a ser enviada
    struct mq_attr attr; // Atributos da fila de mensagens

    // Configurar os atributos da fila de mensagens
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Número máximo de mensagens na fila
    attr.mq_msgsize = sizeof(msg); // Tamanho máximo de cada mensagem
    attr.mq_curmsgs = 0; // Número atual de mensagens na fila

    // Abrir a fila de mensagens para escrita
    mq = mq_open(FILA_COMANDOS, O_WRONLY | O_CREAT, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("Erro ao abrir a fila de mensagens"); // Exibe mensagem de erro se a abertura falhar
        exit(EXIT_FAILURE); // Encerra o programa em caso de erro
    }

    int opcao; // Variável para armazenar a opção escolhida pelo usuário
    while (1) {
        // Exibir o menu de opções do painel
        printf("\nPainel do Carro:\n");
        printf("1 - Acelerar\n");
        printf("2 - Frear\n");
        printf("3 - Ligar Farol Baixo\n");
        printf("4 - Ligar Farol Alto\n");
        printf("5 - Seta Direita\n");
        printf("6 - Seta Esquerda\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao); // Lê a opção escolhida pelo usuário

        if (opcao == 0) {
            printf("Saindo do painel...\n");
            break; // Sai do loop e encerra o programa
        }

        msg.comando = opcao; // Define o código do comando na mensagem
        switch (opcao) {
            case 1: strcpy(msg.descricao, "Acelerar"); break; // Descrição do comando "Acelerar"
            case 2: strcpy(msg.descricao, "Frear"); break; // Descrição do comando "Frear"
            case 3: strcpy(msg.descricao, "Farol Baixo Ligado"); break; // Descrição do comando "Farol Baixo Ligado"
            case 4: strcpy(msg.descricao, "Farol Alto Ligado"); break; // Descrição do comando "Farol Alto Ligado"
            case 5: strcpy(msg.descricao, "Seta direita"); break; // Descrição do comando "Seta direita"
            case 6: strcpy(msg.descricao, "Seta esquerda"); break; // Descrição do comando "Seta esquerda"
            default:
                printf("Comando inválido!\n"); // Mensagem de erro para comando inválido
                continue; // Volta ao início do loop para solicitar nova entrada
        }

        // Enviar a mensagem para a fila de mensagens
        if (mq_send(mq, (const char *)&msg, sizeof(msg), 0) == -1) {
            perror("Erro ao enviar mensagem"); // Exibe mensagem de erro se o envio falhar
        } else {
            printf("Comando enviado: %s\n", msg.descricao); // Confirmação de envio bem-sucedido
        }
    }

    // Fechar a fila de mensagens
    mq_close(mq);
    return 0;
}
