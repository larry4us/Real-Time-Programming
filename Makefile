# Compilador
CC = gcc

# Flags de compilacao (adicionada a -Iinclude aqui)
CFLAGS = -g -Wall -std=c17 -Iinclude

# Diretorios
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/programa_teste

# Encontra todos os arquivos .c na pasta src
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Regra principal
all: $(TARGET)
	./$(TARGET)

# Regra para criar o executavel
$(TARGET): $(SOURCES)
	@mkdir -p $(BINDIR)
	$(CC) $(SOURCES) $(CFLAGS) -o $@

# Regra para limpar os arquivos compilados
clean:
	rm -rf $(BINDIR)

.PHONY: all clean