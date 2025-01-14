# Makefile

CC = gcc
CFLAGS = -Wall -pthread

# Nomes dos executáveis
CONTROLADOR = controlador
SIMULADOR = simulador
PAINEL = painel

# Arquivos fonte
CONTROLADOR_SRC = controlador.c
SIMULADOR_SRC = simulador.c
PAINEL_SRC = painel.c

# Alvo padrão
all: $(CONTROLADOR) $(SIMULADOR) $(PAINEL)

# Compilar o controlador
$(CONTROLADOR): $(CONTROLADOR_SRC)
    $(CC) $(CFLAGS) -o $(CONTROLADOR) $(CONTROLADOR_SRC) -lrt

# Compilar o simulador
$(SIMULADOR): $(SIMULADOR_SRC)
    $(CC) $(CFLAGS) -o $(SIMULADOR) $(SIMULADOR_SRC) -lrt

# Compilar o painel
$(PAINEL): $(PAINEL_SRC)
    $(CC) $(CFLAGS) -o $(PAINEL) $(PAINEL_SRC) -lrt

# Limpar os arquivos compilados
clean:
    rm -f $(CONTROLADOR) $(SIMULADOR) $(PAINEL)

# Executar o controlador e o simulador ao mesmo tempo
run: $(CONTROLADOR) $(SIMULADOR)
    ./$(SIMULADOR) &
    ./$(CONTROLADOR) &
    wait

# Executar o painel
run_painel: $(PAINEL)
    ./$(PAINEL)

# Parar a execução do controlador e do simulador
stop:
    @pkill -f $(CONTROLADOR)
    @pkill -f $(SIMULADOR)