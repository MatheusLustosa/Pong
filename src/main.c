#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// Terminal control sequences
#define ESC            "\033"
#define NORMALTEXT     "[0m"
#define BOLDTEXT       "[1m"
#define ITALICTEXT     "[3m"
#define BLINKTEXT      "[5m"
#define REVERSETEXT    "[7m"
#define HOMECURSOR     "[f"
#define SHOWCURSOR     "[?25h"
#define HIDECURSOR     "[?25l"
#define CLEARSCREEN    "[2J"

// BOX Drawing - Unix like terminals
#define BOX_ENABLE     "(0"
#define BOX_DISABLE    "(B"
#define BOX_VLINE      0x78
#define BOX_HLINE      0x71
#define BOX_UPLEFT     0x6C
#define BOX_UPRIGHT    0x6B
#define BOX_DWNLEFT    0x6D
#define BOX_DWNRIGHT   0x6A
#define BOX_CROSS      0x6E
#define BOX_TLEFT      0X74
#define BOX_TRIGHT     0X75
#define BOX_TUP        0X77
#define BOX_TDOWN      0X76

#define BOX_DIAMOND    0x60
#define BOX_BLOCK      0x61
#define BOX_DOT        0x7E

// screen constants
#define SCRSTARTX      3      // Initial and final screen positions for the game
#define SCRENDX        75     // It means the area that can be drawn 
#define SCRSTARTY      1
#define SCRENDY        23

#define MINX           1      // min screen horizontal pos
#define MINY           1      // min screen vertical pos
#define MAXX           80     // max screen horizontal pos
#define MAXY           24     // max screen vertical pos

// Screen Colors type
typedef enum {BLACK, RED, GREEN, BROWN, BLUE, MAGENTA, CYAN, LIGHTGRAY,
        DARKGRAY, LIGHTRED, LIGHTGREEN, YELLOW, LIGHTBLUE, 
        LIGHTMAGENTA, LIGHTCYAN, WHITE} screenColor; 

static struct termios initialSettings, newSettings;
static int peekCharacter;
static struct timeval timer, now;
static int delay = -1;

int x = 34, y = 12;
int incX = 1, incY = 1;

void screenHomeCursor();
void screenShowCursor();
void screenHideCursor();
void screenClear();
void screenUpdate();
void screenSetNormal();
void screenSetBold();
void screenSetBlink();
void screenSetReverse();
void screenBoxEnable();
void screenBoxDisable();
void screenGotoxy(int x, int y);
void screenDrawBorders();
void screenInit(int drawBorders);
void screenDestroy();
void screenSetColor(screenColor fg, screenColor bg);
void timerInit(int valueMilliSec);
void timerDestroy();
void timerUpdateTimer(int valueMilliSec);
int getTimeDiff();
int timerTimeOver();
void timerPrint();
void keyboardInit();
void keyboardDestroy();
int keyhit();
int readch();
void printHello(int nextX, int nextY);
void printKey(int ch);

int main() {
    static int ch = 0;

    screenInit(1);
    keyboardInit();
    timerInit(50);

    printHello(x, y);
    screenUpdate();

    while (ch != 10) //enter
    {
        // Handle user input
        if (keyhit()) {
            ch = readch();
            printKey(ch);
            screenUpdate();
        }

        // Update game state (move elements, verify collision, etc)
        if (timerTimeOver() == 1) {
            int newX = x + incX;
            if (newX >= (MAXX - strlen("Hello World") - 1) || newX <= MINX + 1) incX = -incX;
            int newY = y + incY;
            if (newY >= MAXY - 1 || newY <= MINY + 1) incY = -incY;

            printKey(ch);
            printHello(newX, newY);

            screenUpdate();
        }
    }

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}

void screenHomeCursor() {
    printf("%s%s", ESC, HOMECURSOR);
}

void screenShowCursor() {
    printf("%s%s", ESC, SHOWCURSOR);
}

void screenHideCursor() {
    printf("%s%s", ESC, HIDECURSOR);
}

void screenClear() {
    screenHomeCursor();
    printf("%s%s", ESC, CLEARSCREEN);
}

void screenUpdate() {
    fflush(stdout);
}

void screenSetNormal() {
    printf("%s%s", ESC, NORMALTEXT);
}

void screenSetBold() {
    printf("%s%s", ESC, BOLDTEXT);
}

void screenSetBlink() {
    printf("%s%s", ESC, BLINKTEXT);
}

void screenSetReverse() {
    printf("%s%s", ESC, REVERSETEXT);
}

void screenBoxEnable() {
    printf("%s%s", ESC, BOX_ENABLE);
}

void screenBoxDisable() {
    printf("%s%s", ESC, BOX_DISABLE);
}
void screenGotoxy(int x, int y) {
    x = (x < 0 ? 0 : x >= MAXX ? MAXX - 1 : x);
    y = (y < 0 ? 0 : y > MAXY ? MAXY : y);

    printf("%s[f%s[%dB%s[%dC", ESC, ESC, y, ESC, x);
}
void screenDrawBorders() {
    char hbc = BOX_HLINE;
    char vbc = BOX_VLINE;

    screenClear();
    screenBoxEnable();

    screenGotoxy(MINX, MINY);
    printf("%c", BOX_UPLEFT);

    for (int i = MINX + 1; i < MAXX; i++) {
        screenGotoxy(i, MINY);
        printf("%c", hbc);
    }
    screenGotoxy(MAXX, MINY);
    printf("%c", BOX_UPRIGHT);

    for (int i = MINY + 1; i < MAXY; i++) {
        screenGotoxy(MINX, i);
        printf("%c", vbc);
        screenGotoxy(MAXX, i);
        printf("%c", vbc);
    }

    screenGotoxy(MINX, MAXY);
    printf("%c", BOX_DWNLEFT);
    for (int i = MINX + 1; i < MAXX; i++) {
        screenGotoxy(i, MAXY);
        printf("%c", hbc);
    }
    screenGotoxy(MAXX, MAXY);
    printf("%c", BOX_DWNRIGHT);

    screenBoxDisable();
}

void screenInit(int drawBorders) {
    screenClear();
    if (drawBorders) screenDrawBorders();
    screenHomeCursor();
    screenHideCursor();
}

void screenDestroy() {
    printf("%s[0;39;49m", ESC); // Reset colors
    screenSetNormal();
    screenClear();
    screenHomeCursor();
    screenShowCursor();
}


void screenSetColor(screenColor fg, screenColor bg) {
    char atr[] = "[0;";

    if (fg > LIGHTGRAY) {
        atr[1] = '1';
        fg -= 8;
    }

    printf("%s%s%d;%dm", ESC, atr, fg + 30, bg + 40);
}

void timerInit(int valueMilliSec) {
    delay = valueMilliSec;
    gettimeofday(&timer, NULL);
}

void timerDestroy() {
    delay = -1;
}

void timerUpdateTimer(int valueMilliSec) {
    delay = valueMilliSec;
    gettimeofday(&timer, NULL);
}

int getTimeDiff() {
    gettimeofday(&now, NULL);
    long diff = (((now.tv_sec - timer.tv_sec) * 1000000) + now.tv_usec - timer.tv_usec) / 1000;
    return (int)diff;
}

int timerTimeOver() {
    int ret = 0;

    if (getTimeDiff() > delay) {
        ret = 1;
        gettimeofday(&timer, NULL);
    }

    return ret;
}

void timerPrint() {
    printf("Timer:  %d", getTimeDiff());
}

void keyboardInit() {
    tcgetattr(0, &initialSettings);
    newSettings = initialSettings;
    newSettings.c_lflag &= ~ICANON;
    newSettings.c_lflag &= ~ECHO;
    newSettings.c_lflag &= ~ISIG;
    newSettings.c_cc[VMIN] = 1;
    newSettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newSettings);
}

void keyboardDestroy() {
    tcsetattr(0, TCSANOW, &initialSettings);
}

int keyhit() {
    unsigned char ch;
    int nread;

    if (peekCharacter != -1) return 1;

    newSettings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &newSettings);
    nread = read(0, &ch, 1);
    newSettings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &newSettings);

    if (nread == 1) {
        peekCharacter = ch;
        return 1;
    }

    return 0;
}

int readch() {
    char ch;

    if (peekCharacter != -1) {
        ch = peekCharacter;
        peekCharacter = -1;
        return ch;
    }
    read(0, &ch, 1);
    return ch;
}

void printHello(int nextX, int nextY) {
    screenSetColor(CYAN, DARKGRAY);
    screenGotoxy(x, y);
    printf("           ");
    x = nextX;
    y = nextY;
    screenGotoxy(x, y);
    printf("Hello World");
}

void printKey(int ch) {
    screenSetColor(YELLOW, DARKGRAY);
    screenGotoxy(35, 22);
    printf("Key code :");

    screenGotoxy(34, 23);
    printf("            ");

    if (ch == 27) screenGotoxy(36, 23);
    else screenGotoxy(39, 23);

    printf("%d ", ch);
    while (keyhit()) {
        printf("%d ", readch());
    }
}
