    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <time.h>
    #include <termios.h>
    #include "keyboard.h"
    #include "screen.h"
    #include "timer.h"

    #define FIELD_WIDTH 80
    #define FIELD_HEIGHT 24
    #define PADDLE_WIDTH 1
    #define PADDLE_HEIGHT 6
    #define PADDLE_MOVE_SPEED 10
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

    void render(Ball *ball, Paddle *p1, Paddle *p2, int **obstacles) {
        screenClear();

        for (int x = 0; x <= FIELD_WIDTH + 1; x++) {
            screenGotoxy(x, 0);
            printf("═");
            screenGotoxy(x, FIELD_HEIGHT + 1);
            printf("═");
        }
        for (int y = 0; y <= FIELD_HEIGHT + 1; y++) {
            screenGotoxy(0, y);
            printf("║");
            screenGotoxy(FIELD_WIDTH + 1, y);
            printf("║");
        }
        screenGotoxy(0, 0);
        printf("╔");
        screenGotoxy(FIELD_WIDTH + 1, 0);
        printf("╗");
        screenGotoxy(0, FIELD_HEIGHT + 1);
        printf("╚");
        screenGotoxy(FIELD_WIDTH + 1, FIELD_HEIGHT + 1);
        printf("╝");

        screenGotoxy((int)ball->x + 1, (int)ball->y + 1);
        printf("\033[31mO\033[0m");

        for (int i = 0; i < p1->height; i++) {
            if (p1->y + i >= 0 && p1->y + i < FIELD_HEIGHT) {
                screenGotoxy(p1->x + 1, p1->y + i + 1);
                printf("\033[34m|\033[0m");
            }
        }
        for (int i = 0; i < p2->height; i++) {
            if (p2->y + i >= 0 && p2->y + i < FIELD_HEIGHT) {
                screenGotoxy(p2->x + 1, p2->y + i + 1);
                printf("\033[34m|\033[0m");
            }
        }

        for (int y = 0; y < FIELD_HEIGHT; y++) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                if (obstacles[y][x]) {
                    screenGotoxy(x + 1, y + 1);
                    printf("|");
                }
            }
        }

        screenGotoxy(0, FIELD_HEIGHT + 3);
        printf("%s: Pontos - %d | Vidas - ", p1->name, p1->score);
        printHearts(p1->lives);
        printf("\n");
        printf("%s: Pontos - %d | Vidas - ", p2->name, p2->score);
        printHearts(p2->lives);
        printf("\n");

        fflush(stdout);
    }

    void writeScoreToFile(Paddle *winner, Paddle *loser) {
        FILE *file = fopen(SCORE_FILE, "a");
        if (file != NULL) {
            fprintf(file, "%s %d x %d %s\n", winner->name, winner->score, loser->score, loser->name);
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

        Paddle *winner = (p1->score > p2->score) ? p1 : p2;
        Paddle *loser = (p1->score < p2->score) ? p1 : p2;
        writeScoreToFile(winner, loser);

        printf("Pressione Enter para voltar ao menu...");
        while (getchar() != '\n') {}
    }

    void showAsciiArt() {
        screenClear();
        printf("\n");
        printf("          /\\     /\\              /\\    /\\\n");
        printf("         /  o   o  \\            /  o   o  \\ \n");
        printf("        ( ==  ^  == )          ( ==  ^  == )\n");
        printf("        )    U    (             )    U    (\n");
        printf("        /\\       /\\             /\\     /\\\n");
        printf("       /   |   |   \\          /   |   |   \\ \n");
        printf("      (    |   |    )         (    |   |    )\n");
        printf("      (||)       (||)         (||)       (||)   \n");
        printf("\n\n");
    }

    void hideCursor() {
        printf("\033[?25l");
        fflush(stdout);
    }

    void showCursor() {
        printf("\033[?25h");
        fflush(stdout);
    }

    void startGame(int singlePlayerMode) {
        srand(time(NULL));

        Ball ball = {BALL_INITIAL_X, BALL_INITIAL_Y, BALL_SPEED_X_INITIAL, BALL_SPEED_Y_INITIAL};
        Paddle p1 = {2, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0, ""};
        Paddle p2 = {FIELD_WIDTH - 3, FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT, INITIAL_LIVES, 0, ""};

        float computerPaddleSpeed = 1.5;

        int **obstacles = (int **)malloc(FIELD_HEIGHT * sizeof(int *));
        for (int i = 0; i < FIELD_HEIGHT; i++) {
            obstacles[i] = (int *)calloc(FIELD_WIDTH, sizeof(int));
        }

        int obstaclePresent = 0;

        time_t startTime = time(NULL);

        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        newt.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}

        printf("                Digite o nome do Jogador 1: ");
        if (fgets(p1.name, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
            perror("Erro ao ler o nome do Jogador 1");
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return;
        }
        p1.name[strcspn(p1.name, "\n")] = '\0';

        if (!singlePlayerMode) {
            printf("                Digite o nome do Jogador 2: ");
            if (fgets(p2.name, PLAYER_NAME_MAX_LENGTH, stdin) == NULL) {
                perror("Erro ao ler o nome do Jogador 2");
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                return;
            }
            p2.name[strcspn(p2.name, "\n")] = '\0'; 
        } else {
            strcpy(p2.name, "Computador");
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        hideCursor();

        while (p1.lives > 0 && p2.lives > 0) {
            time_t currentTime = time(NULL);
            double elapsedTime = difftime(currentTime, startTime);

            if (singlePlayerMode && !obstaclePresent && elapsedTime >= 2.0) {
                int ox, oy;
                do {
                    ox = rand() % (FIELD_WIDTH - 20) + 10;
                    oy = rand() % (FIELD_HEIGHT - 2) + 1;  
                } while ((ox == (int)BALL_INITIAL_X && oy == (int)BALL_INITIAL_Y) ||
                        (oy >= p1.y && oy <= p1.y + p1.height) ||
                        (oy >= p2.y && oy <= p2.y + p2.height));
                obstacles[oy][ox] = 1;
                obstaclePresent = 1;
            }

            
            float next_x = ball.x + ball.dx;
            float next_y = ball.y + ball.dy;

            if ((int)next_x >= 0 && (int)next_x < FIELD_WIDTH && (int)next_y >= 0 && (int)next_y < FIELD_HEIGHT) {
                if (obstacles[(int)next_y][(int)next_x]) {
                    ball.dx = -ball.dx;
                    ball.dy = -ball.dy;
                    obstacles[(int)next_y][(int)next_x] = 0;
                    obstaclePresent = 0;
                    startTime = time(NULL);
                }
            }

            ball.x += ball.dx;
            ball.y += ball.dy;

            if ((int)ball.y <= 1 || (int)ball.y >= FIELD_HEIGHT - 2) {
                ball.dy = -ball.dy;
            }

            if (((int)ball.x == p1.x + 1 && (int)ball.y >= p1.y && (int)ball.y < p1.y + p1.height) ||
                ((int)ball.x == p2.x - 1 && (int)ball.y >= p2.y && (int)ball.y < p2.y + p2.height)) {
                ball.dx = -ball.dx;
            }

            if ((int)ball.x <= 0) {
                p1.lives--;
                p2.score++;
                ball.x = BALL_INITIAL_X;
                ball.y = BALL_INITIAL_Y;
                ball.dx = BALL_SPEED_X_INITIAL;
                ball.dy = BALL_SPEED_Y_INITIAL;
                startTime = time(NULL);
                obstaclePresent = 0;

                for (int i = 0; i < FIELD_HEIGHT; i++) {
                    memset(obstacles[i], 0, FIELD_WIDTH * sizeof(int));
                }
            } else if ((int)ball.x >= FIELD_WIDTH - 1) {
                p2.lives--;
                p1.score++;
                ball.x = BALL_INITIAL_X;
                ball.y = BALL_INITIAL_Y;
                ball.dx = -BALL_SPEED_X_INITIAL;
                ball.dy = BALL_SPEED_Y_INITIAL;
                startTime = time(NULL);
                obstaclePresent = 0;
                for (int i = 0; i < FIELD_HEIGHT; i++) {
                    memset(obstacles[i], 0, FIELD_WIDTH * sizeof(int));
                }
            }

            render(&ball, &p1, &p2, obstacles);

            while (keyhit()) {
                char ch = readch();
                if (ch == 'w' && p1.y > 1) {
                    p1.y--;
                } else if (ch == 's' && p1.y + p1.height < FIELD_HEIGHT - 1) {
                    p1.y++;
                }

                if (!singlePlayerMode) {
                    if (ch == 'o' && p2.y > 1) {
                        p2.y--;
                    } else if (ch == 'l' && p2.y + p2.height < FIELD_HEIGHT - 1) {
                        p2.y++;
                    }
                }
            }

            if (singlePlayerMode) {
                if (ball.x >= FIELD_WIDTH / 2) {
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
            }

            while (!timerTimeOver()) {}
        }

        showCursor();

        printFinalScreen(&p1, &p2);

        for (int i = 0; i < FIELD_HEIGHT; i++) {
            free(obstacles[i]);
        }
        free(obstacles);
    }

    void showRules() {
        screenClear();
        printf("\n");
        printf("     /\\/\\                        /\\/\\    \n");
        printf("    ( o.o )                        ( o.o )     \n");
        printf("     > ^ <                          > ^ <       \n");
        printf("\n\n");
        printf("+---------------------------------------------------+\n");
        printf("|                    Regras do Jogo                 |\n");
        printf("|                                                   |\n");
        printf("| 1. Cada jogador controla uma raquete.             |\n");
        printf("| 2. O objetivo é rebater a bola e fazer o          |\n");
        printf("|    adversário perder vidas.                       |\n");
        printf("| 3. Jogador 1 usa 'w' e 's' para mover a raquete.  |\n");
        printf("| 4. Jogador 2 usa 'o' e 'l' para mover a raquete.  |\n");
        printf("| 5. No modo single-player, um obstáculo '#' aparece|\n");
        printf("|    após 2 segundos e a bola pode colidir com ele. |\n");
        printf("| 6. O jogo termina quando um dos jogadores perde   |\n");
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
            printf("+---------------------------------------------------+   /\\    _/\\ \n");
            printf("|                     Pong Game                     |  /  o   o  \\ \n");
            printf("|                                                   | ( ==  ^  == )\n");
            printf("|           1 - Jogar contra o computador           |  )    U    (\n");
            printf("|           2 - Jogar contra um amigo               |  | |     | | \n");
            printf("|           3 - Regras                              |  | |     | | \n");
            printf("|           0 - Sair                                |  | |     | | \n");
            printf("+---------------------------------------------------+\n");
            printf("                   Escolha uma opção: ");

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
                    startGame(1);
                    break;
                case 2:
                    startGame(0);
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
