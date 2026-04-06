# Nome do executável exigido pelo projeto
BINARY = programaTrab

# Pastas de acordo com a sua árvore
MODULES = ./modules
SRC = ./src

# Compilador e Flags
CC = gcc
FLAGS = -g -Wall -Werror -I$(MODULES)

# Todos os arquivos .c necessários
SOURCES = $(SRC)/programaTrab.c $(MODULES)/functions.c $(MODULES)/utils.c

# 1. REGRA DE COMPILAÇÃO (Obrigatória: 'make all')
all:
	$(CC) $(FLAGS) $(SOURCES) -o $(BINARY) -lm

# 2. REGRA DE EXECUÇÃO (Obrigatória: 'make run')
run:
	./$(BINARY)

# 3. REGRA PARA CRIAR O ZIP (Para você usar antes de enviar)
# Esse comando limpa o que é desnecessário e compacta a estrutura
zip: clean
	zip -r submissao.zip Makefile modules/ src/

# 4. LIMPEZA
clean:
	rm -f $(BINARY) *.zip