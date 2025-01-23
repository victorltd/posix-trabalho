# Sistema de Simulação e Controle de Veículo com Painel Interativo

Este projeto consiste em três módulos principais: **Controlador**, **Simulador** e **Painel Interativo**, que juntos formam um sistema integrado para monitorar, controlar e visualizar sensores de um veículo. O projeto utiliza conceitos como memória compartilhada, comunicação entre processos, filas de mensagens, threads e manipulação de sinais, além de incluir um painel gráfico para melhorar a interação do usuário.

O link do vídeo, demonstrando o seu funcionamento é o : https://youtu.be/-7YdMUeo4HQ


---

## Sumário

- [Descrição Geral](#descrição-geral)
- [Funcionalidades](#funcionalidades)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Pré-requisitos](#pré-requisitos)
- [Como Executar](#como-executar)
- [Funcionamento do Sistema](#funcionamento-do-sistema)
  - [Controlador](#controlador)
  - [Simulador](#simulador)
  - [Painel Interativo](#painel-interativo)
- [Tecnologias Utilizadas](#tecnologias-utilizadas)
- [Possíveis Melhorias](#possíveis-melhorias)
- [Autor](#autor)

---

## Descrição Geral

Este projeto foi desenvolvido para simular e monitorar o comportamento de sensores em um veículo, permitindo também a visualização em um painel gráfico interativo. Ele utiliza **memória compartilhada** para a troca de informações entre o simulador e o controlador, e **filas de mensagens** para o envio de comandos ao simulador. 

O painel interativo exibe os dados em tempo real, incluindo gráficos e indicadores intuitivos.

---

## Funcionalidades

1. **Simulador**:
   - Atualiza os valores de sensores com base em estados físicos simulados.
   - Simula comandos como acelerar, frear ou manter o estado neutro.
   - Gera valores aleatórios para o sensor de proximidade.

2. **Controlador**:
   - Monitora e exibe os dados dos sensores em tempo real.
   - Processa comandos recebidos de uma fila de mensagens.
   - Registra a ocorrência de eventos, como ultrapassar o limite de velocidade.

3. **Painel Interativo**:
   - Interface gráfica para exibição dos dados dos sensores.
   - Gráficos e indicadores visuais para monitoramento fácil.
   - Atualização em tempo real dos valores.

4. **Comunicação Entre Processos**:
   - Uso de memória compartilhada para sincronizar dados entre o simulador e o controlador.
   - Uso de filas de mensagens para envio de comandos ao simulador.

5. **Relatório Consolidado**:
   - Geração de um relatório final com a contagem de eventos importantes.

---

## Estrutura do Projeto

- **`controlador.c`**: Implementa a lógica de monitoramento e controle.
- **`simulador.c`**: Implementa a lógica de simulação dos sensores e estados internos.
- **`painel.c`**: Implementa o painel gráfico interativo 
- **Memória Compartilhada**: Utilizada para troca de informações entre o simulador e o controlador.
- **Fila de Mensagens**: Utilizada para enviar comandos do controlador ao simulador.

---

## Pré-requisitos

Certifique-se de ter os seguintes requisitos instalados no sistema:

- GCC (compilador para C).
- Biblioteca `pthread` para manipulação de threads.
- Biblioteca `mqueue` para filas de mensagens.
- Sistema operacional baseado em UNIX (nesse trabalho foi totalmente usado o Ubuntu no WSL).
- **Padrões POSIX** como `shm_open`, `mmap`, e `mqueue` são amplamente utilizados.



---

## Como Executar

### Passo 1: Compilar os Programas

Os programas já estão compilados, bastante apenas executar, mas caso faça alguma atualização, deve-se realizar novamente, para isso, abra o terminal na pasta do projeto e execute os seguintes comandos:

```bash
make

```

### Passo 2: Executar o Controlador Principal

Inicie o controlador em um terminal:

```bash
make run
```

### Passo 3: Executar o painel

Em outro terminal, inicie o painel:

```bash
make run_painel
```



### Passo 4: Finalizar o programa

Para finalizar o programa e conferir o relatório final consolidado, é necessário  saber o PID do processo associado ao controlador principal,para isso basta digitar em outro terminal:

```bash
ps -ef | grep controlador
```
Assim será possível encontrar o PID desse processo, em seguida, basta digitar o comando a seguir,  junto do PID do processo, onde será possivel visualizar o relatório:
```bash
kill -SIGUSR2 <PID>
```
(substituir <PID> pelo número  do PID desse processo)

---

## Funcionamento do Sistema

### Controlador

- **Função Principal**:
  - Lê os dados atualizados dos sensores a partir da memória compartilhada.
  - Exibe os dados em tempo real.
  - Registra eventos relevantes, como exceder o limite de velocidade.
- **Relatório Consolidado**:
  - Ao término da execução, exibe um relatório detalhado com as estatísticas coletadas.

### Simulador

- **Estados Internos**:
  - Simula inércia física para velocidade e RPM.
  - Calcula temperatura com base no estado do motor.
- **Sensores**:
  - Atualiza periodicamente os valores na memória compartilhada.
  - Gera valores aleatórios para proximidade.

### Painel Interativo

- **Interface Gráfica**:
  - Exibe os dados dos sensores em tempo real.
  - Inclui gráficos de velocidade, RPM, temperatura e proximidade.
  - Atualizações dinâmicas e visuais para facilitar a compreensão.
- **Integração**:
  - Lê os valores diretamente da memória compartilhada para refletir no painel.

---

## Tecnologias Utilizadas

- **C Programming Language**:
  - Base para a implementação do controlador e simulador.
- **Memória Compartilhada** (`shm_open`, `mmap`):
  - Para sincronização entre simulador e controlador.
- **Filas de Mensagens** (`mqueue`):
  - Para envio de comandos entre processos.
- **Threads (`pthread`)**:
  - Para simular sensores simultâneos.


---

## Possíveis Melhorias

- Melhorar o design do painel gráfico para maior usabilidade.
- Implementar suporte para sensores adicionais ou novos comandos.
- Adicionar exportação de dados do painel para arquivo CSV ou JSON.

---

## Autor

Desenvolvido por Victor Augusto. Caso tenha dúvidas ou sugestões, entre em contato pelo Github.
