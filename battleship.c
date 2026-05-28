#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define EMPTY '.'
#define SHIP 'S'
#define MISS 'o'
#define HIT 'X'

typedef struct {
    int x;
    int y;
    int active;
    int dirX;
    int dirY;
    int hitPoints[10][2];
    int hitCount;
} HuntState;

HuntState hunt = {0};

void resetHunt(void);
void addHitPoint(int x, int y);
void waitForEnter(void);
void printField(char f[10][10], int hide);
int isValid(int x, int y);
void initEmpty(char f[10][10]);
int loadField(char* name, char f[10][10]);
int checkGameOver(char f[10][10]);
void markShipAround(char f[10][10], int x, int y);
int isKilled(char f[10][10], int x, int y);
int playerMove(char enemy[10][10]);
int getRandomShot(char player[10][10]);
void getNextHuntShot(char player[10][10], int* x, int* y);
void computerMove(char player[10][10], int lastX, int lastY, int* nextX, int* nextY, int* extra);
int canPlaceShip(char f[10][10], int x, int y, int dx, int dy, int len);
void computerPlaceShips(char f[10][10]);
int validateField(char f[10][10]);

void addHitPoint(int x, int y) {
    if (hunt.hitCount < 10) {
        hunt.hitPoints[hunt.hitCount][0] = x;
        hunt.hitPoints[hunt.hitCount][1] = y;
        hunt.hitCount++;
    }
}

void waitForEnter(void) {
    printf("[Press ENTER to continue...]");
    while (getchar() != '\n');
    getchar();
}

void printField(char f[10][10], int hide) {
    printf("  ");
    for (int i = 0; i < 10; i++) printf("%2d", i + 1);
    printf("\n");
    for (int i = 0; i < 10; i++) {
        printf("%2d", i + 1);
        for (int j = 0; j < 10; j++) {
            char c = f[i][j];
            if (hide && c == SHIP) c = EMPTY;
            printf(" %c", c);
        }
        printf("\n");
    }
}

int isValid(int x, int y) {
    return x >= 0 && x < 10 && y >= 0 && y < 10;
}

void initEmpty(char f[10][10]) {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            f[i][j] = EMPTY;
}

int loadField(char* name, char f[10][10]) {
    FILE* fp = fopen(name, "r");
    if (!fp) return 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            char c;
            do c = fgetc(fp); while (c == ' ' || c == '\n' || c == '\r');
            if (c == SHIP || c == EMPTY) f[i][j] = c;
            else { fclose(fp); return 0; }
        }
    }
    fclose(fp);
    return 1;
}

int checkGameOver(char f[10][10]) {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            if (f[i][j] == SHIP) return 0;
    return 1;
}

void markShipAround(char f[10][10], int x, int y) {
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (isValid(x + dx, y + dy) && f[x + dx][y + dy] == EMPTY)
                f[x + dx][y + dy] = MISS;
}

int isKilled(char f[10][10], int x, int y) {
    int visited[10][10] = {0};
    int queue[100][2];
    int front = 0, rear = 0;
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
   
    queue[rear][0] = x;
    queue[rear][1] = y;
    rear++;
    visited[x][y] = 1;
   
    while (front < rear) {
        int cx = queue[front][0];
        int cy = queue[front][1];
        front++;
       
        for (int d = 0; d < 4; d++) {
            int nx = cx + dirs[d][0];
            int ny = cy + dirs[d][1];
            if (isValid(nx, ny) && !visited[nx][ny] && (f[nx][ny] == HIT || f[nx][ny] == SHIP)) {
                visited[nx][ny] = 1;
                queue[rear][0] = nx;
                queue[rear][1] = ny;
                rear++;
            }
        }
    }
   
    for (int i = 0; i < rear; i++) {
        int cx = queue[i][0];
        int cy = queue[i][1];
        if (f[cx][cy] == SHIP) {
            return 0;
        }
    }
   
    for (int i = 0; i < rear; i++) {
        int cx = queue[i][0];
        int cy = queue[i][1];
        markShipAround(f, cx, cy);
    }
   
    return 1;
}

int playerMove(char enemy[10][10]) {
    int x, y;
    int result;
    while (1) {
        printf("Your shot (row col 1..10): ");
        result = scanf("%d %d", &x, &y);
       
        if (result != 2) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter two numbers.\n");
            continue;
        }
       
        x--; y--;
       
        if (!isValid(x, y)) {
            printf("Invalid coordinates. Use numbers 1-10.\n");
            continue;
        }
        if (enemy[x][y] == MISS || enemy[x][y] == HIT) {
            printf("Already shot here.\n");
            continue;
        }
        break;
    }

    if (enemy[x][y] == SHIP) {
        printf("HIT!\n");
        enemy[x][y] = HIT;
        if (isKilled(enemy, x, y)) {
            printf("You destroyed an enemy ship!\n");
        }
        return 1;
    } else {
        printf("MISS.\n");
        enemy[x][y] = MISS;
        return 0;
    }
}

int getRandomShot(char player[10][10]) {
    int x, y;
    do {
        x = rand() % 10;
        y = rand() % 10;
    } while (player[x][y] == MISS || player[x][y] == HIT);
    return x * 10 + y;
}

void getNextHuntShot(char player[10][10], int* x, int* y) {
    int shot, lastX, lastY, nx, ny;
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    if (hunt.hitCount == 1 && hunt.dirX == 0 && hunt.dirY == 0) {
        int cx = hunt.hitPoints[0][0];
        int cy = hunt.hitPoints[0][1];
        for (int i = 0; i < 4; i++) {
            int nx = cx + dirs[i][0];
            int ny = cy + dirs[i][1];
            if (isValid(nx, ny) && player[nx][ny] != MISS && player[nx][ny] != HIT) {
                *x = nx;
                *y = ny;
                hunt.dirX = dirs[i][0];
                hunt.dirY = dirs[i][1];
                return;
            }
        }
        resetHunt();
        shot = getRandomShot(player);
        *x = shot / 10;
        *y = shot % 10;
        return;
    }
   
    lastX = hunt.hitPoints[hunt.hitCount - 1][0];
    lastY = hunt.hitPoints[hunt.hitCount - 1][1];
    nx = lastX + hunt.dirX;
    ny = lastY + hunt.dirY;
   
    if (isValid(nx, ny) && player[nx][ny] != MISS && player[nx][ny] != HIT) {
        *x = nx;
        *y = ny;
        return;
    }
   
    for (int i = 0; i < 4; i++) {
        nx = hunt.hitPoints[0][0] + dirs[i][0];
        ny = hunt.hitPoints[0][1] + dirs[i][1];
        if (isValid(nx, ny) && player[nx][ny] != MISS && player[nx][ny] != HIT) {
            *x = nx;
            *y = ny;
            hunt.dirX = dirs[i][0];
            hunt.dirY = dirs[i][1];
            return;
        }
    }
   
    resetHunt();
    shot = getRandomShot(player);
    *x = shot / 10;
    *y = shot % 10;
}

void computerMove(char player[10][10], int lastX, int lastY, int* nextX, int* nextY, int* extra) {
    int x, y;
    *extra = 0;
   
    if (!hunt.active && lastX != -1) {
        hunt.active = 1;
        hunt.dirX = 0;
        hunt.dirY = 0;
        hunt.hitCount = 0;
        addHitPoint(lastX, lastY);
    }
   
    if (hunt.active) {
        getNextHuntShot(player, &x, &y);
    } else {
        int shot = getRandomShot(player);
        x = shot / 10;
        y = shot % 10;
    }
   
    printf("Computer shoots at (%d,%d)... ", x + 1, y + 1);
   
    if (player[x][y] == SHIP) {
        printf("HIT!\n");
        player[x][y] = HIT;
        addHitPoint(x, y);
       
        if (isKilled(player, x, y)) {
            printf("Ship destroyed!\n");
            resetHunt();
            *extra = 1;
            *nextX = -1;
            *nextY = -1;
            return;
        }
        *extra = 1;
        *nextX = x;
        *nextY = y;
        return;
    } else {
        printf("MISS\n");
        player[x][y] = MISS;
       
        if (hunt.active) {
            *extra = 0;
            *nextX = -1;
            *nextY = -1;
            return;
        }
        *extra = 0;
        *nextX = -1;
        *nextY = -1;
    }
}

int canPlaceShip(char f[10][10], int x, int y, int dx, int dy, int len) {
    for (int i = 0; i < len; i++) {
        int nx = x + i * dx, ny = y + i * dy;
        if (!isValid(nx, ny) || f[nx][ny] != EMPTY) return 0;
        for (int a = -1; a <= 1; a++)
            for (int b = -1; b <= 1; b++)
                if (isValid(nx + a, ny + b) && f[nx + a][ny + b] == SHIP) return 0;
    }
    return 1;
}

void computerPlaceShips(char f[10][10]) {
    int ships[] = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    int total = 10;
    for (int s = 0; s < total; s++) {
        int len = ships[s];
        int placed = 0;
        while (!placed) {
            int x = rand() % 10;
            int y = rand() % 10;
            int horiz = rand() % 2;
            int dx = horiz ? 1 : 0;
            int dy = horiz ? 0 : 1;
            if (canPlaceShip(f, x, y, dx, dy, len)) {
                for (int i = 0; i < len; i++) {
                    int nx = x + i * dx, ny = y + i * dy;
                    f[nx][ny] = SHIP;
                }
                placed = 1;
            }
        }
    }
}

int validateField(char f[10][10]) {
    int shipCounts[5] = {0,0,0,0,0};
    int visited[10][10] = {0};
   
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (f[i][j] == SHIP) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if ((dx != 0 || dy != 0) && isValid(i+dx, j+dy) && f[i+dx][j+dy] == SHIP) {
                            if (dx != 0 && dy != 0) {
                                printf("Error: Ships touch diagonally at (%d,%d) and (%d,%d)\n", i+1, j+1, i+dx+1, j+dy+1);
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
   
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (f[i][j] == SHIP && !visited[i][j]) {
                int len = 1;
                int horiz = 0, vert = 0;
               
                if (j+1 < 10 && f[i][j+1] == SHIP) horiz = 1;
                if (i+1 < 10 && f[i+1][j] == SHIP) vert = 1;
               
                if (horiz && vert) {
                    printf("Error: Ship is L-shaped at (%d,%d)\n", i+1, j+1);
                    return 0;
                }
               
                if (horiz) {
                    int k = j;
                    while (k < 10 && f[i][k] == SHIP) {
                        visited[i][k] = 1;
                        k++;
                        len++;
                    }
                    len--;
                } else if (vert) {
                    int k = i;
                    while (k < 10 && f[k][j] == SHIP) {
                        visited[k][j] = 1;
                        k++;
                        len++;
                    }
                    len--;
                } else {
                    visited[i][j] = 1;
                    len = 1;
                }
               
                if (len < 1 || len > 4) {
                    printf("Error: Invalid ship length %d at (%d,%d)\n", len, i+1, j+1);
                    return 0;
                }
                shipCounts[len]++;
            }
        }
    }
   
    if (shipCounts[1] != 4 || shipCounts[2] != 3 || shipCounts[3] != 2 || shipCounts[4] != 1) {
        printf("Error: Wrong ship counts (1:%d,2:%d,3:%d,4:%d). Need (4,3,2,1)\n",
               shipCounts[1], shipCounts[2], shipCounts[3], shipCounts[4]);
        return 0;
    }
   
    return 1;
}

int main(int argc, char* argv[]) {
    char playerField[10][10];
    char enemyField[10][10];
    char playerView[10][10];
    int gameOver = 0, turn = 1;
    int lastHitX = -1, lastHitY = -1;
    int hit, extra, nextX, nextY;

    if (argc != 2) {
        printf("Usage: %s field.txt\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    resetHunt();

    if (!loadField(argv[1], playerField)) {
        printf("Error loading field\n");
        return 1;
    }
   
    if (!validateField(playerField)) {
        printf("Invalid ship placement!\n");
        return 1;
    }

    initEmpty(enemyField);
    initEmpty(playerView);

    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            if (playerField[i][j] == SHIP)
                playerView[i][j] = SHIP;

    computerPlaceShips(enemyField);

    while (!gameOver) {
        if (turn == 1) {
            printf("\n========== YOUR TURN ==========\n");
            printf("Your field:\n");
            printField(playerView, 0);
            printf("\nEnemy field (targets):\n");
            printField(enemyField, 1);
            printf("\n");
           
            hit = playerMove(enemyField);
           
            printf("\n[Press ENTER to continue...]");
            while (getchar() != '\n');
            getchar();
           
            if (hit) {
                printf("\n[EXTRA TURN!]\n");
                continue;
            }
            turn = 2;
            lastHitX = -1;
            lastHitY = -1;
        } else {
            printf("\n========== COMPUTER TURN ==========\n");
           
            extra = 0;
            nextX = -1, nextY = -1;
           
            computerMove(playerView, lastHitX, lastHitY, &nextX, &nextY, &extra);
           
            printf("\nYour field after computer's move:\n");
            printField(playerView, 0);
           
            printf("\n[Press ENTER to continue...]");
            while (getchar() != '\n');
            getchar();
           
            if (extra) {
                lastHitX = nextX;
                lastHitY = nextY;
                printf("\n[COMPUTER GETS ANOTHER TURN]\n");
                continue;
            }
            turn = 1;
            lastHitX = -1;
            lastHitY = -1;
        }

        if (checkGameOver(playerView)) {
            printf("\n==================================\n");
            printf("         GAME OVER — YOU LOSE!\n");
            printf("==================================\n");
            gameOver = 1;
        } else if (checkGameOver(enemyField)) {
            printf("\n==================================\n");
            printf("         GAME OVER — YOU WIN!\n");
            printf("==================================\n");
            gameOver = 1;
        }
    }
    return 0;
}
