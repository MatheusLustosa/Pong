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
#define PADDLE_MOVE_SPEED 2 // Defina a velocidade de movimento das paddles
#define INITIAL_LIVES 4
#define BALL_INITIAL_X (FIELD_WIDTH / 2)
#define BALL_INITIAL_Y (FIELD_HEIGHT / 2)
#define BALL_SPEED_X 1
#define BALL_SPEED_Y 1
#define PLAYER_NAME_MAX_LENGTH 50
#define SCORE_FILE "score.txt"

typedef struct {
    int x, y;
    int dx, dy;
} Ball;

typedef struct {
    int x, y;
    int width, height;
    int lives;
    int score;
} Paddle;

typedef struct PlayerNode {
    char name[PLAYER_NAME_MAX_LENGTH];
    int score;
    int lives;
    struct PlayerNode *next;
} PlayerNode;

void render(char **field, Ball *ball, Paddle *p1, Paddle *p2, char *playerName1, char *playerName2) {
    screenClear();

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        field[y][0] = '|';
        field[y][FIELD_WIDTH - 1] = '|';
    }
    for (int x = 0; x < FIELD_WIDTH; x++) {
        field[0][x] = '-';
        field[FIELD_HEIGHT - 1][x] = '-';
    }

    field[ball->y][ball->x] = 'O';

    for (int i = 0; i < p1->height; i++) {
        field[p1->y + i][p1->x] = '|';
    }
    for (int i = 0; i < p2->height; i++) {
        field[p2->y + i][p2->x] = '|';
    }

    printf("\n%s: Pontos - %d | Vidas - %d\n", playerName1, p1->score, p1->lives);
    printf("\n%s: Pontos - %d | Vidas - %d\n", playerName2, p2->score, p2->lives);

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            putchar(field[y][x]);
        }
        putchar('\n');
    }
}

void writeScoreToFile(PlayerNode *head) {
    FILE *file = fopen(SCORE_FILE, "a");
    if (file != NULL) {
        PlayerNode *current = head;
        while (current != NULL) {
            fprintf(file, "%s %d x %d\n", current->name, current->score, current->lives);
            current = current->next;
        }
        fclose(file);
    }
}

void printFinalScreen(PlayerNode *head) {
    screenClear();

    printf("+-----------------------------------------+\n");
    printf("|               Game Over!                |\n");
    printf("|                                         |\n");
    printf("|   Jogador       Pontos   Vidas Restantes|\n");
    printf("|-----------------------------------------|\n");

    PlayerNode *current = head;
    while (current != NULL) {
        printf("|   %-12s %-8d %-15d|\n", current->name, current->score, current->lives);
        current = current->next;
    }

    printf("+-----------------------------------------+\n");

    writeScoreToFile(head);

    printf("Pressione Enter para voltar ao menu...");
    while (getchar() != '\n') {}
}

void startGame() {
    // Alocar memória corretamente para o campo (matriz 2D)
    char **field = (char **)malloc(FIELD_HEIGHT * sizeof(char *));
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        field[i] = (char *)malloc(FIELD_WIDTH * sizeof(char));
    }

    srand(time(NULL));

    Ball ball = {BALL_INITIAL_X, BALL_INITIAL_Y, BALL_SPEED_X, BALL_SPEED_Y};
    Paddle p1 = {2, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0};
    Paddle p2 = {FIELD_WIDTH - 3, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0};

    // Limpar buffer de entrada
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}

    char playerName1[PLAYER_NAME_MAX_LENGTH];
    printf("Digite o nome do Jogador 1: ");
    if (fgets(playerName1, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
        perror("Erro ao ler o nome do Jogador 1");
        return;
    }
    playerName1[strcspn(playerName1, "\n")] = '\0';

    char playerName2[PLAYER_NAME_MAX_LENGTH];
    printf("Digite o nome do Jogador 2: ");
    if (fgets(playerName2, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
        perror("Erro ao ler o nome do Jogador 2");
        return;
    }
    playerName2[strcspn(playerName2, "\n")] = '\0';

    while (p1.lives > 0 && p2.lives > 0) {
        for (int y = 0; y < FIELD_HEIGHT; y++) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                field[y][x] = ' ';
            }
        }

        ball.x += ball.dx;
        ball.y += ball.dy;

        if (ball.y <= 0 || ball.y >= FIELD_HEIGHT - 1) {
            ball.dy = -ball.dy;
        }

        if ((ball.x == p1.x + 1 && ball.y >= p1.y && ball.y < p1.y + p1.height) ||
            (ball.x == p2.x - 1 && ball.y >= p2.y && ball.y < p2.y + p2.height)) {
            ball.dx = -ball.dx;
        }

        if (ball.x <= 0) {
            p2.lives--;
            p1.score++;
            ball.x = BALL_INITIAL_X;
            ball.y = BALL_INITIAL_Y;
            ball.dx = BALL_SPEED_X;
            ball.dy = BALL_SPEED_Y;
        } else if (ball.x >= FIELD_WIDTH - 1) {
            p1.lives--;
            p2.score++;
            ball.x = BALL_INITIAL_X;
            ball.y = BALL_INITIAL_Y;
            ball.dx = BALL_SPEED_X;
            ball.dy = BALL_SPEED_Y;
        }

        render(field, &ball, &p1, &p2, playerName1, playerName2);

        if (keyhit()) {
            char ch = readch();
            if (ch == 'w' && p1.y > 1) {
                p1.y--;
            } else if (ch == 's' && p1.y + p1.height < FIELD_HEIGHT - 1) {
                p1.y++;
            } else if (ch == 'o' && p2.y > 1) {
                p2.y--;
            } else if (ch == 'l' && p2.y + p2.height < FIELD_HEIGHT - 1) {
                p2.y++;
            }
        }

        while (!timerTimeOver()) {}
    }

    for (int i = 0; i < FIELD_HEIGHT; i++) {
        free(field[i]);
    }
    free(field);

    PlayerNode *winner;
    if (p1.lives == 0) {
        winner = (PlayerNode *)malloc(sizeof(PlayerNode));
        strncpy(winner->name, playerName2, PLAYER_NAME_MAX_LENGTH);
        winner->score = p2.score;
        winner->lives = p2.lives;
    } else {
        winner = (PlayerNode *)malloc(sizeof(PlayerNode));
        strncpy(winner->name, playerName1, PLAYER_NAME_MAX_LENGTH);
        winner->score = p1.score;
        winner->lives = p1.lives;
    }
    winner->next = NULL;

    printFinalScreen(winner);
    free(winner);
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
    printf("| 5. O jogo termina quando um dos jogadores perde   |\n");
    printf("|    todas as vidas.                                |\n");
    printf("+---------------------------------------------------+\n");
    printf("Pressione Enter para voltar ao menu...");
    while (getchar() != '\n') {}
}

void showMenu() {
    int choice = -1;
    char input[10];  // buffer para armazenar a entrada do usuário
    do {
        screenClear();
        printf("+---------------------------------+\n");
        printf("|           Pong Game             |\n");
        printf("|                                 |\n");
        printf("| 1 - Iniciar Jogo                |\n");
        printf("| 2 - Regras                      |\n");
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
                startGame();
                break;
            case 2:
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
