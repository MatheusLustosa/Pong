#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "keyboard.h"
#include "screen.h"
#include "timer.h"

#define FIELD_WIDTH 80
#define FIELD_HEIGHT 24
#define PADDLE_WIDTH 1
#define PADDLE_HEIGHT 6
#define PADDLE_MOVE_SPEED 2
#define INITIAL_LIVES 4
#define BALL_INITIAL_X (FIELD_WIDTH / 2)
#define BALL_INITIAL_Y (FIELD_HEIGHT / 2)
#define BALL_SPEED_X_INITIAL 1
#define BALL_SPEED_Y_INITIAL 1
#define PLAYER_NAME_MAX_LENGTH 50
#define SCORE_FILE "score.txt"

typedef struct {
    float x, y;
    float dx, dy;
} Ball;

typedef struct {
    int x, y;
    int width, height;
    int lives;
    int score;
    char name[PLAYER_NAME_MAX_LENGTH];
} Paddle;

void printHearts(int lives) {
    for (int i = 0; i < lives; i++) {
        printf("♥ ");
    }
}

void render(char **field, Ball *ball, Paddle *p1, Paddle *p2) {
    screenClear();

    // Desenhar bordas contínuas formando um retângulo perfeito
    for (int x = 0; x < FIELD_WIDTH; x++) {
        field[0][x] = '#';
        field[FIELD_HEIGHT - 1][x] = '#';
    }
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        field[y][0] = '#';
        field[y][FIELD_WIDTH - 1] = '#';
    }

    // Desenhar bola
    field[(int)ball->y][(int)ball->x] = 'O';

    // Desenhar paddles com "|"
    for (int i = 0; i < p1->height; i++) {
        if (p1->y + i > 0 && p1->y + i < FIELD_HEIGHT - 1) {
            field[p1->y + i][p1->x] = '|';
        }
    }
    for (int i = 0; i < p2->height; i++) {
        if (p2->y + i > 0 && p2->y + i < FIELD_HEIGHT - 1) {
            field[p2->y + i][p2->x] = '|';
        }
    }

    // Exibir pontuações e vidas com corações
    printf("\n%s: Pontos - %d | Vidas - ", p1->name, p1->score);
    printHearts(p1->lives);
    printf("\n");
    printf("\n%s: Pontos - %d | Vidas - ", p2->name, p2->score);
    printHearts(p2->lives);
    printf("\n");

    // Renderizar campo com cores
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            char ch = field[y][x];
            if (ch == '#') {
                // Bordas em verde
                printf("\033[32m%c\033[0m", ch);
            } else if (ch == '|') {
                // Paddles em azul
                printf("\033[34m%c\033[0m", ch);
            } else if (ch == 'O') {
                // Bola em vermelho
                printf("\033[31m%c\033[0m", ch);
            } else {
                putchar(ch);
            }
        }
        putchar('\n');
    }
}

void writeScoreToFile(Paddle *winner) {
    FILE *file = fopen(SCORE_FILE, "a");
    if (file != NULL) {
        fprintf(file, "%s %d x %d\n", winner->name, winner->score, winner->lives);
        fclose(file);
    }
}

void printFinalScreen(Paddle *p1, Paddle *p2) {
    screenClear();

    printf("+-----------------------------------------+\n");
    printf("|               Game Over!                |\n");
    printf("|                                         |\n");
    printf("|   Jogador       Pontos   Vidas Restantes|\n");
    printf("|-----------------------------------------|\n");

    printf("|   %-12s %-8d ", p1->name, p1->score);
    printHearts(p1->lives);
    printf("|\n");

    printf("|   %-12s %-8d ", p2->name, p2->score);
    printHearts(p2->lives);
    printf("|\n");

    printf("+-----------------------------------------+\n");

    // Determinar o vencedor e escrever no arquivo
    Paddle *winner = (p1->score > p2->score) ? p1 : p2;
    writeScoreToFile(winner);

    printf("Pressione Enter para voltar ao menu...");
    while (getchar() != '\n') {}
}
void showAsciiArt() {
    screenClear();
    printf("\n");
    printf("                 .::::.                           \n");
    printf("            ..:::.        .::::::.        ..::.   \n");
    printf("        .############=.    ::::::.   .=############.\n");
    printf("      -#################+   ....   =#################-\n");
    printf("    .####################=      -####################.\n");
    printf("    =######################+    +######################=\n");
    printf("   .########################+  =########################.\n");
    printf("   .#########################:.#########################:\n");
    printf("   .#########################-:#########################.\n");
    printf("   .+########################:.########################+.\n");
    printf("    :######################*-  -+######################: \n");
    printf("     -###################++.  .++###################-  \n");
    printf("      .#################++++:  :++++###############.   \n");
    printf("        -#############+++++++..=++++++*############-     \n");
    printf("          .=########+===++++++++++++===+*#######=.       \n");
    printf("                          .=+++++.                        \n");
    printf("                         -++++++++-                       \n");
    printf("                       :+++++::+++++-                     \n");
    printf("                       :+++-    :+++-                     \n");
    printf("\n\n");
}
void startGame(int singlePlayerMode) {
    // Alocar memória para o campo
    char **field = (char **)malloc(FIELD_HEIGHT * sizeof(char *));
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        field[i] = (char *)malloc(FIELD_WIDTH * sizeof(char));
    }

    srand(time(NULL));

    Ball ball = {BALL_INITIAL_X, BALL_INITIAL_Y, BALL_SPEED_X_INITIAL, BALL_SPEED_Y_INITIAL};
    Paddle p1 = {2, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0, ""};
    Paddle p2 = {FIELD_WIDTH - 3, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0, ""};

    // Variáveis para aumentar a velocidade da bola
    float speedMultiplier = 1.0;

    // Variável para controlar a velocidade do paddle do computador
    int computerPaddleSpeed = 2; // Aumentado para 2

    // Limpar buffer de entrada
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}

    printf("Digite o nome do Jogador 1: ");
    if (fgets(p1.name, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
        perror("Erro ao ler o nome do Jogador 1");
        return;
    }
    p1.name[strcspn(p1.name, "\n")] = '\0';  // Remover newline

    if (!singlePlayerMode) {
        printf("Digite o nome do Jogador 2: ");
        if (fgets(p2.name, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
            perror("Erro ao ler o nome do Jogador 2");
            return;
        }
        p2.name[strcspn(p2.name, "\n")] = '\0';  // Remover newline
    } else {
        strcpy(p2.name, "Computador");
    }

    while (p1.lives > 0 && p2.lives > 0) {
        // Limpar o campo
        for (int y = 0; y < FIELD_HEIGHT; y++) {
            memset(field[y], ' ', FIELD_WIDTH);
        }

        // Atualizar a posição da bola com a velocidade atual
        ball.x += ball.dx * speedMultiplier;
        ball.y += ball.dy * speedMultiplier;

        // Aumentar a velocidade da bola
        speedMultiplier += 0.005; // Aumenta a velocidade mais rapidamente

        // Verificar colisão com as bordas superior e inferior
        if ((int)ball.y <= 1 || (int)ball.y >= FIELD_HEIGHT - 2) {
            ball.dy = -ball.dy;
        }

        // Verificar colisão com os paddles
        if (((int)ball.x == p1.x + 1 && (int)ball.y >= p1.y && (int)ball.y < p1.y + p1.height) ||
            ((int)ball.x == p2.x - 1 && (int)ball.y >= p2.y && (int)ball.y < p2.y + p2.height)) {
            ball.dx = -ball.dx;
        }

        // Verificar se a bola saiu dos limites esquerdo ou direito
        if ((int)ball.x <= 0) {
            p2.lives--;
            p1.score++;
            // Reiniciar a bola e velocidade
            ball.x = BALL_INITIAL_X;
            ball.y = BALL_INITIAL_Y;
            ball.dx = BALL_SPEED_X_INITIAL;
            ball.dy = BALL_SPEED_Y_INITIAL;
            speedMultiplier = 1.0;
        } else if ((int)ball.x >= FIELD_WIDTH - 1) {
            p1.lives--;
            p2.score++;
            // Reiniciar a bola e velocidade
            ball.x = BALL_INITIAL_X;
            ball.y = BALL_INITIAL_Y;
            ball.dx = -BALL_SPEED_X_INITIAL;
            ball.dy = BALL_SPEED_Y_INITIAL;
            speedMultiplier = 1.0;
        }

        // Renderizar o jogo
        render(field, &ball, &p1, &p2);

        // Capturar todas as teclas pressionadas
        while (keyhit()) {
            char ch = readch();
            // Controle do Jogador 1
            if (ch == 'w' && p1.y > 1) {
                p1.y--;
            } else if (ch == 's' && p1.y + p1.height < FIELD_HEIGHT - 1) {
                p1.y++;
            }

            // Controle do Jogador 2
            if (!singlePlayerMode) {
                if (ch == 'o' && p2.y > 1) {
                    p2.y--;
                } else if (ch == 'l' && p2.y + p2.height < FIELD_HEIGHT - 1) {
                    p2.y++;
                }
            }
        }

        // Movimento automático do computador
        if (singlePlayerMode) {
            // O computador só se move se a bola estiver próxima da metade do campo ou no lado dele
            if (ball.x >= FIELD_WIDTH / 2) {
                // Movimento ajustado com velocidade aumentada
                if (ball.y > p2.y + p2.height / 2 && p2.y + p2.height < FIELD_HEIGHT - 1) {
                    p2.y += computerPaddleSpeed;
                    if (p2.y + p2.height >= FIELD_HEIGHT - 1) {
                        p2.y = FIELD_HEIGHT - 1 - p2.height;
                    }
                } else if (ball.y < p2.y + p2.height / 2 && p2.y > 1) {
                    p2.y -= computerPaddleSpeed;
                    if (p2.y <= 1) {
                        p2.y = 1;
                    }
                }
            }
            // Caso contrário, o computador não se move
        }

        // Aguarda o tempo do frame
        while (!timerTimeOver()) {}
    }

    // Liberar memória
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        free(field[i]);
    }
    free(field);

    // Exibir a tela final
    printFinalScreen(&p1, &p2);
}



void showRules() {
    screenClear();
    printf("+---------------------------------------------------+\n");
    printf("|                    Regras do Jogo                 |\n");
    printf("|                                                   |\n");
    printf("| 1. Cada jogador controla uma raquete.             |\n");
    printf("| 2. O objetivo é rebater a bola e fazer o          |\n");
    printf("|    adversário perder vidas.                       |\n");
    printf("| 3. Jogador 1 usa 'w' e 's' para mover a raquete.  |\n");
    printf("| 4. Jogador 2 usa 'o' e 'l' para mover a raquete.  |\n");
    printf("| 5. No modo single-player, o computador só se move |\n");
    printf("|    quando a bola está na metade do campo dele.    |\n");
    printf("| 6. A velocidade da bola aumenta rapidamente.      |\n");
    printf("| 7. O jogo termina quando um dos jogadores perde   |\n");
    printf("|    todas as vidas.                                |\n");
    printf("+---------------------------------------------------+\n");
    printf("Pressione Enter para voltar ao menu...");
    while (getchar() != '\n') {}
}

void showMenu() {
    int choice = -1;
    char input[10];
    do {
        showAsciiArt();
        printf("+---------------------------------+\n");
        printf("|           Pong Game             |\n");
        printf("|                                 |\n");
        printf("| 1 - Jogar contra o computador   |\n");
        printf("| 2 - Jogar contra um amigo       |\n");
        printf("| 3 - Regras                      |\n");
        printf("| 0 - Sair                        |\n");
        printf("+---------------------------------+\n");
        printf("Escolha uma opção: ");

        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &choice) != 1) {
                choice = -1;
            }
        }
        if (choice == -1) {
            printf("Opção inválida, tente novamente.\n");
            printf("Pressione Enter para continuar...");
            while (getchar() != '\n') {}
        }
        switch (choice) {
            case 1:
                startGame(1); // Modo single-player
                break;
            case 2:
                startGame(0); // Modo dois jogadores
                break;
            case 3:
                showRules();
                break;
            case 0:
                printf("Saindo do jogo...\n");
                break;
            default:
                printf("Opção inválida, tente novamente.\n");
                printf("Pressione Enter para continuar...");
                while (getchar() != '\n') {}
        }
    } while (choice != 0);
}

int main() {
    keyboardInit();
    screenInit(0);
    timerInit(80);

    showMenu();

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}