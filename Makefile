# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Arquivos fonte
SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/keyboard.c $(SRC_DIR)/screen.c $(SRC_DIR)/timer.c
# Arquivos de objeto correspondentes
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Nome do executável final
EXEC = $(BUILD_DIR)/pong

# Flags do compilador
CFLAGS = -Wall -I$(INCLUDE_DIR)  # -Wall habilita todos os warnings, e -I adiciona o diretório include

# Regra padrão para compilar o programa
all: $(EXEC)

# Como gerar o executável final a partir dos arquivos de objeto
$(EXEC): $(OBJ_FILES)
	@gcc $(OBJ_FILES) -o $(EXEC)

# Como compilar os arquivos .c em arquivos .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)  # Cria o diretório build caso não exista
	@gcc $(CFLAGS) -c $< -o $@

# Limpeza (remove arquivos gerados)
clean:
	@rm -rf $(BUILD_DIR)

# Regra para rodar o jogo
run: $(EXEC)
	@./$(EXEC)

# Regras para limpar os arquivos gerados
.PHONY: all clean run
