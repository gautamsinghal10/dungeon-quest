/* dungeon.c - map generation (2D arrays), movement and tile events */
#include "game.h"

/* recursive flood fill: marks every cell that can be reached from (x, y) */
static void floodFill(const Dungeon *d, int reach[MAP_SIZE][MAP_SIZE], int x, int y)
{
    if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) return;
    if (reach[y][x] || d->cells[y][x] == T_WALL) return;
    reach[y][x] = 1;
    floodFill(d, reach, x + 1, y);
    floodFill(d, reach, x - 1, y);
    floodFill(d, reach, x, y + 1);
    floodFill(d, reach, x, y - 1);
}

static void placeWalls(Dungeon *d, int count)
{
    while (count > 0) {
        int r = rand() % MAP_SIZE;
        int c = rand() % MAP_SIZE;
        if (d->cells[r][c] == T_EMPTY && !(r == 0 && c == 0)) {
            d->cells[r][c] = T_WALL;
            count--;
        }
    }
}

/* reveal the 3x3 area around (x, y) */
static void revealAround(Dungeon *d, int x, int y)
{
    for (int r = y - 1; r <= y + 1; r++)
        for (int c = x - 1; c <= x + 1; c++)
            if (r >= 0 && r < MAP_SIZE && c >= 0 && c < MAP_SIZE)
                d->seen[r][c] = 1;
}

void revealAll(Dungeon *d)
{
    for (int r = 0; r < MAP_SIZE; r++)
        for (int c = 0; c < MAP_SIZE; c++)
            d->seen[r][c] = 1;
}

void generateDungeon(Dungeon *d, const Player *p)
{
    int reach[MAP_SIZE][MAP_SIZE];
    int exitX = MAP_SIZE - 1;
    int exitY = MAP_SIZE - 1;

    /* keep generating until the exit can be reached from the start */
    do {
        for (int r = 0; r < MAP_SIZE; r++)
            for (int c = 0; c < MAP_SIZE; c++) {
                d->cells[r][c] = T_EMPTY;
                d->seen[r][c] = 0;
                reach[r][c] = 0;
            }
        d->cells[exitY][exitX] = (p->floor == MAX_FLOORS) ? T_BOSS : T_EXIT;
        placeWalls(d, 10 + p->floor * 2);
        floodFill(d, reach, 0, 0);
    } while (!reach[exitY][exitX]);

    revealAround(d, 0, 0);
}

void printDungeon(const Dungeon *d, const Player *p)
{
    printf("\n===== FLOOR %d / %d =====\n", p->floor, MAX_FLOORS);
    for (int r = 0; r < MAP_SIZE; r++) {
        printf(" ");
        for (int c = 0; c < MAP_SIZE; c++) {
            char ch;
            if (p->x == c && p->y == r)
                ch = '@';
            else if (d->seen[r][c])
                ch = d->cells[r][c];
            else
                ch = '?';
            printf("%c ", ch);
        }
        printf("\n");
    }
    printf(" @ you  ? unknown  # wall  M monster  $ gold  C chest  ^ trap\n");
    printf(" S shop  F fountain  > stairs  B boss\n");
}

/* returns 1 if the player moved, 0 if something blocked the way */
int movePlayer(Player *p, Dungeon *d, char dir)
{
    int nx = p->x;
    int ny = p->y;

    switch (dir) {
        case 'w': ny--; break;
        case 's': ny++; break;
        case 'a': nx--; break;
        case 'd': nx++; break;
        default:  return 0;
    }
    if (nx < 0 || nx >= MAP_SIZE || ny < 0 || ny >= MAP_SIZE)
        return 0;
    if (d->cells[ny][nx] == T_WALL)
        return 0;

    p->x = nx;
    p->y = ny;
    p->steps++;
    revealAround(d, nx, ny);
    return 1;
}
