#include <stdio.h>

typedef struct {
    int x;
    int y;
    int active;
    int dirX;
    int dirY;
    int hitPoints[10][2];
    int hitCount;
} HuntState;

extern HuntState hunt;

void resetHunt(void) {
    hunt.active = 0;
    hunt.dirX = 0;
    hunt.dirY = 0;
    hunt.hitCount = 0;
}
