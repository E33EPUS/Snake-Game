#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define COLS 20
#define ROWS 20
#define MAX_LEN (COLS * ROWS)
#define MAX_BOMBS 100

typedef struct { int x, y; } Point;

Point body[MAX_LEN];
int head_idx, tail_idx, len;
bool occupied[ROWS][COLS];

Point bombs[MAX_BOMBS];
int bombCount;

int appleX, appleY;
int scissorsX, scissorsY;
bool scissorsExists;
int dirX, dirY;
bool paused, gameOver;
int applesSinceBomb, applesSinceScissors, nextScissorsAt;

enum { DIE_WALL, DIE_SELF, DIE_BOMB, WIN_FULL } deathCause;
enum { MODE_CLASSIC, MODE_BOMB_FRENZY } gameMode;
enum { DIFF_EASY, DIFF_HARD } difficulty;

void gotoxy(int x, int y) {
    COORD c = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void drawCell(int gx, int gy, const char *s) {
    gotoxy(2 + gx * 2, 1 + gy);
    printf("%s", s);
}

/* 检查该位置是否被炸弹占据 */
bool isBombAt(int x, int y) {
    for (int i = 0; i < bombCount; i++)
        if (bombs[i].x == x && bombs[i].y == y) return true;
    return false;
}

bool findEmpty(int *ex, int *ey, bool avoidApple) {
    /* 收集所有空格，按距中心距离加权随机 —— 中心权重高，边框权重低 */
    float cx = (COLS - 1) / 2.0f, cy = (ROWS - 1) / 2.0f;
    float maxDist = sqrtf(cx * cx + cy * cy); /* 中心到角落的距离 */
    float totalWeight = 0;
    int bestX = -1, bestY = -1;

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (occupied[y][x] || isBombAt(x, y) ||
                (scissorsExists && x == scissorsX && y == scissorsY) ||
                (avoidApple && x == appleX && y == appleY))
                continue;

            if (bestX < 0) { bestX = x; bestY = y; } /* 保底 */

            /* 距中心越近权重越高：中心=1.0，角落≈0.15 */
            float dx = x - cx, dy = y - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            float weight = 1.0f - (dist / maxDist) * 0.85f;
            totalWeight += weight;

            if ((float)rand() / RAND_MAX * totalWeight <= weight) {
                bestX = x; bestY = y;
            }
        }
    }

    if (bestX < 0) return false;
    *ex = bestX; *ey = bestY;
    return true;
}

void spawnApple() {
    if (findEmpty(&appleX, &appleY, false))
        drawCell(appleX, appleY, "\xf0\x9f\x8d\x8e");
}

bool spawnBomb() {
    if (bombCount >= MAX_BOMBS) return false;
    int bx, by;
    if (findEmpty(&bx, &by, true)) {
        bombs[bombCount].x = bx;
        bombs[bombCount].y = by;
        bombCount++;
        drawCell(bx, by, "\xf0\x9f\x92\xa3");
        return true;
    }
    return false;
}

void drawAllBombs() {
    for (int i = 0; i < bombCount; i++)
        drawCell(bombs[i].x, bombs[i].y, "\xf0\x9f\x92\xa3");
}

void spawnScissors() {
    int sx, sy;
    if (findEmpty(&sx, &sy, true)) {
        scissorsX = sx; scissorsY = sy;
        scissorsExists = true;
        drawCell(sx, sy, "\xe2\x9c\x82\xef\xb8\x8f"); /* ✂️ */
    }
}

void removeHalfBombs() {
    int removeCount = bombCount / 2;
    if (removeCount == 0 && bombCount > 0) removeCount = 1;
    for (int i = bombCount - removeCount; i < bombCount; i++)
        drawCell(bombs[i].x, bombs[i].y, "  ");
    bombCount -= removeCount;
}

void drawBorder() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    DWORD written;
    COORD home = {0, 0};
    FillConsoleOutputCharacterA(h, ' ', csbi.dwSize.X * csbi.dwSize.Y, home, &written);
    FillConsoleOutputAttribute(h, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, home, &written);
    gotoxy(0, 0);

    for (int i = 0; i < COLS + 2; i++) printf("\xf0\x9f\x9f\xab"); /* 🟫 */
    printf("\n");
    for (int y = 0; y < ROWS; y++) {
        printf("\xf0\x9f\x9f\xab");                      /* 🟫 */
        for (int x = 0; x < COLS * 2; x++) printf(" ");
        printf("\xf0\x9f\x9f\xab\n");                    /* 🟫 */
    }
    for (int i = 0; i < COLS + 2; i++) printf("\xf0\x9f\x9f\xab"); /* 🟫 */
}

void drawAllSnake() {
    int idx = tail_idx;
    for (int i = 0; i < len; i++) {
        drawCell(body[idx].x, body[idx].y, "\xf0\x9f\x9f\xa9");
        idx = (idx + 1) % MAX_LEN;
    }
}

void drawScore() {
    gotoxy(0, ROWS + 2);
    printf("长度: %-4d  %s  ", len, difficulty == DIFF_HARD ? "[困难]" : "[简单]");
    if (gameMode == MODE_BOMB_FRENZY || bombCount > 0) {
        gotoxy(28, ROWS + 2);
        printf("炸弹: %-4d", bombCount);
    }
    gotoxy(0, ROWS + 3);
    printf("WASD移动 | Q暂停 | R重置    ");
}

/* ---- 菜单 ---- */

void menuClear() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    DWORD written;
    COORD home = {0, 0};
    FillConsoleOutputCharacterA(h, ' ', csbi.dwSize.X * csbi.dwSize.Y, home, &written);
    FillConsoleOutputAttribute(h, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, home, &written);
    gotoxy(0, 0);
}

void menuBox(const char *title) {
    int w = 40;
    putchar('+');
    for (int i = 0; i < w - 2; i++) putchar('-');
    printf("+\n");
    printf("|");
    int pad = (w - 2 - (int)strlen(title)) / 2;
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s", title);
    int rpad = w - 2 - pad - (int)strlen(title);
    for (int i = 0; i < rpad; i++) putchar(' ');
    printf("|\n");
    printf("+");
    for (int i = 0; i < w - 2; i++) putchar('-');
    printf("+\n\n");
}

int showMenu(const char *title, const char *items[], int count) {
    int sel = 0;
    while (1) {
        menuClear();
        menuBox(title);
        for (int i = 0; i < count; i++) {
            printf("    %s %s\n", (i == sel) ? "\x1a" : " ", items[i]); /* → */
        }
        printf("\n  [W/S] 选择  [Enter] 确认  [Esc] 返回\n");

        int ch = _getch();
        if (ch == 224) {
            ch = _getch();
            if (ch == 72) sel = (sel + count - 1) % count;
            if (ch == 80) sel = (sel + 1) % count;
        }
        if (ch == 'w' || ch == 'W') sel = (sel + count - 1) % count;
        if (ch == 's' || ch == 'S') sel = (sel + 1) % count;
        if (ch == 13) return sel; /* Enter */
        if (ch == 'q' || ch == 'Q') return -1;
        if (ch == 27) return -1; /* Esc */
    }
}

/* ---- 游戏逻辑 ---- */

void reset() {
    gameOver = false;
    paused = false;
    dirX = 1; dirY = 0;
    bombCount = 0;
    scissorsExists = false;
    applesSinceBomb = 0;
    applesSinceScissors = 0;
    nextScissorsAt = (difficulty == DIFF_EASY) ? (8 + rand() % 5)
                                               : (5 + rand() % 4); /* 困难5~8 */

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            occupied[y][x] = false;

    int cx = COLS / 2, cy = ROWS / 2;
    len = 1;
    head_idx = 0;
    tail_idx = 0;
    body[0].x = cx;
    body[0].y = cy;
    occupied[cy][cx] = true;

    drawBorder();
    drawScore();
    spawnApple();
    drawAllSnake();
}

void moveSnake() {
    if (paused || gameOver) return;

    int newX = body[head_idx].x + dirX;
    int newY = body[head_idx].y + dirY;

    if (newX < 0 || newX >= COLS || newY < 0 || newY >= ROWS) {
        gameOver = true; deathCause = DIE_WALL; return;
    }

    bool ateApple    = (newX == appleX && newY == appleY);
    bool ateBomb     = isBombAt(newX, newY);
    bool ateScissors = (scissorsExists && newX == scissorsX && newY == scissorsY);

    if (ateBomb) { gameOver = true; deathCause = DIE_BOMB; return; }

    if (ateScissors) {
        removeHalfBombs();
        scissorsExists = false;
        applesSinceScissors = 0;
        nextScissorsAt = (difficulty == DIFF_EASY) ? (8 + rand() % 5)
                                                   : (5 + rand() % 4);
        gotoxy(34, ROWS + 2);
        printf("%-4d", bombCount);
    }

    if (!ateApple) {
        int tx = body[tail_idx].x;
        int ty = body[tail_idx].y;
        occupied[ty][tx] = false;
        drawCell(tx, ty, "  ");
        tail_idx = (tail_idx + 1) % MAX_LEN;
    } else {
        len++;
    }

    if (occupied[newY][newX]) { gameOver = true; deathCause = DIE_SELF; return; }

    head_idx = (head_idx + 1) % MAX_LEN;
    body[head_idx].x = newX;
    body[head_idx].y = newY;
    occupied[newY][newX] = true;

    drawCell(newX, newY, "\xf0\x9f\x9f\xa9");

    if (ateApple) {
        drawScore();
        if (!findEmpty(&appleX, &appleY, false)) {
            gameOver = true; deathCause = WIN_FULL; return;
        }
        drawCell(appleX, appleY, "\xf0\x9f\x8d\x8e");

        /* 炸弹 + 剪刀（炸弹狂欢 或 经典困难） */
        bool hasBombs = (gameMode == MODE_BOMB_FRENZY) ||
                        (gameMode == MODE_CLASSIC && difficulty == DIFF_HARD);
        if (hasBombs) {
            applesSinceBomb++;
            int threshold;
            if (gameMode == MODE_CLASSIC) {
                threshold = 5; /* 经典困难：固定每5个苹果一颗炸弹 */
            } else if (difficulty == DIFF_EASY) {
                threshold = (len < 10) ? 5 : (len < 20) ? 4 :
                            (len < 30) ? 3 : (len < 40) ? 2 : 1;
            } else {
                threshold = (len < 8) ? 3 : (len < 15) ? 2 : 1;
            }

            if (applesSinceBomb >= threshold) {
                if (spawnBomb()) {
                    applesSinceBomb = 0;
                    /* 炸弹狂欢困难：一次出两颗 */
                    if (difficulty == DIFF_HARD && gameMode == MODE_BOMB_FRENZY)
                        spawnBomb();
                    gotoxy(34, ROWS + 2);
                    printf("%-4d", bombCount);
                }
            }
        }

        /* 剪刀（炸弹狂欢专属） */
        if (gameMode == MODE_BOMB_FRENZY) {
            applesSinceScissors++;
            if (!scissorsExists && applesSinceScissors >= nextScissorsAt) {
                spawnScissors();
                applesSinceScissors = 0;
                nextScissorsAt = (difficulty == DIFF_EASY) ? (8 + rand() % 5)
                                                           : (5 + rand() % 4);
            }
        }
    }
}

void input() {
    if (!_kbhit()) return;
    int ch = _getch();

    if (ch == 'q' || ch == 'Q') {
        paused = !paused;
        gotoxy(0, ROWS + 4);
        if (paused) printf("II 暂停中 | 按 Q 继续    ");
        else        printf("                           ");
        return;
    }

    if (ch == 'r' || ch == 'R') { reset(); return; }
    if (paused || gameOver) return;

    if (ch == 224) {
        ch = _getch();
        switch (ch) {
            case 72: if (dirY == 0) { dirX =  0; dirY = -1; } break;
            case 80: if (dirY == 0) { dirX =  0; dirY =  1; } break;
            case 75: if (dirX == 0) { dirX = -1; dirY =  0; } break;
            case 77: if (dirX == 0) { dirX =  1; dirY =  0; } break;
        }
        return;
    }

    switch (ch) {
        case 'w': case 'W': if (dirY == 0) { dirX =  0; dirY = -1; } break;
        case 's': case 'S': if (dirY == 0) { dirX =  0; dirY =  1; } break;
        case 'a': case 'A': if (dirX == 0) { dirX = -1; dirY =  0; } break;
        case 'd': case 'D': if (dirX == 0) { dirX =  1; dirY =  0; } break;
    }
}

void runGame() {
    DWORD lastTick = GetTickCount();
    const DWORD interval = (difficulty == DIFF_EASY) ? 150 : 80;

    while (!gameOver) {
        input();
        DWORD now = GetTickCount();
        if (now - lastTick >= interval) {
            lastTick = now;
            moveSnake();
        }
        Sleep(1);
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));
    hideCursor();

    /* 主菜单 */
    const char *mainItems[] = {"开始游戏", "退出游戏"};
    while (1) {
        int mainSel = showMenu("贪  吃  蛇", mainItems, 2);
        if (mainSel < 0 || mainSel == 1) break; /* Q 或退出 */

        /* 模式选择 */
        const char *modeItems[] = {"经典模式", "炸弹狂欢"};
        int modeSel = showMenu("选择模式", modeItems, 2);
        if (modeSel < 0) continue; /* Q 返回主菜单 */
        gameMode = (modeSel == 0) ? MODE_CLASSIC : MODE_BOMB_FRENZY;

        /* 难度选择 */
        const char *diffItems[] = {"简单模式", "困难模式"};
        int diffSel = showMenu("选择难度", diffItems, 2);
        if (diffSel < 0) continue;
        difficulty = (diffSel == 0) ? DIFF_EASY : DIFF_HARD;

        /* 游戏循环 */
        do {
            reset();
            runGame();

            gotoxy(0, ROWS + 5);
            printf("==============================      ");
            gotoxy(0, ROWS + 6);
            switch (deathCause) {
                case WIN_FULL: printf("  你赢了！蛇填满了整个场地！  "); break;
                case DIE_BOMB: printf("  炸弹！最终长度: %-4d         ", len); break;
                case DIE_WALL: printf("  撞墙了！最终长度: %-4d       ", len); break;
                default:       printf("  咬到自己！最终长度: %-4d     ", len); break;
            }
            gotoxy(0, ROWS + 7);
            printf("==============================      ");
            gotoxy(0, ROWS + 9);
            printf("  [R] 重新开始    [M] 返回菜单      ");

            int ch;
            while (1) {
                if (_kbhit()) { ch = _getch(); break; }
                Sleep(10);
            }
            if (ch == 'r' || ch == 'R') continue;
            break; /* 返回模式选择 */
        } while (1);
    }

    menuClear();
    return 0;
}
