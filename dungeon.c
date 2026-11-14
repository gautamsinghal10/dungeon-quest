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

/* put `count` copies of `tile` on random empty cells the player can reach */
static void placeRandom(Dungeon *d, int reach[MAP_SIZE][MAP_SIZE], char tile, int count)
{
    int tries = 0;
    while (count > 0 && tries < 2000) {
        int r = rand() % MAP_SIZE;
        int c = rand() % MAP_SIZE;
        tries++;
        if (d->cells[r][c] == T_EMPTY && reach[r][c] && !(r == 0 && c == 0)) {
            d->cells[r][c] = tile;
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

    placeRandom(d, reach, T_MONSTER, 4 + p->floor);
    placeRandom(d, reach, T_GOLD, 3);
    placeRandom(d, reach, T_CHEST, 2);
    placeRandom(d, reach, T_TRAP, 2 + p->floor / 2);

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
    p->mana = MIN(p->maxMana, p->mana + 1);       /* mana slowly comes back */
    return 1;
}

/* returns: 0 = continue, 1 = reached stairs, 2 = boss beaten (win), -1 = player died */
int handleTile(Player *p, Dungeon *d, int prevX, int prevY)
{
    char *tile = &d->cells[p->y][p->x];      /* pointer to the current cell */

    if (p->poisonTurns > 0) {
        p->poisonTurns--;
        p->hp -= 2;
        printf("\nPoison burns you for 2 damage.\n");
        if (p->hp <= 0) return -1;
    }

    switch (*tile) {
        case T_MONSTER: {
            Monster m = makeMonster(p->floor, 0);
            int result = fight(p, &m, 0);
            if (result == 0) return -1;
            if (result == 1) *tile = T_EMPTY;
            else { p->x = prevX; p->y = prevY; }      /* fled: step back */
            break;
        }
        case T_GOLD: {
            int gold = 10 + rand() % (12 * p->floor);
            p->gold += gold;
            printf("\nYou found %d gold coins!\n", gold);
            *tile = T_EMPTY;
            break;
        }
        case T_CHEST: {
            Item it = randomItem(p->floor);
            printf("\nYou open a chest and find: ");
            printItem(&it);
            printf("\n");
            addItem(p, it);
            *tile = T_EMPTY;
            break;
        }
        case T_TRAP: {
            int dmg = 4 + p->floor * 3;
            p->hp -= dmg;
            printf("\nA trap springs! You take %d damage.\n", dmg);
            *tile = T_EMPTY;
            if (p->hp <= 0) return -1;
            break;
        }
        case T_EXIT:
            printf("\nYou found the stairs leading down...\n");
            return 1;
        case T_BOSS: {
            Monster boss = makeMonster(p->floor, 1);
            printf("\nThe Dragon Lord rises from his throne!\n");
            return (fight(p, &boss, 1) == 1) ? 2 : -1;
        }
            p->mana = p->maxMana;
            p->poisonTurns = 0;
        default:
            break;
    }
    return 0;
}

void nextFloor(Player *p, Dungeon *d)
{
    p->floor++;
    p->x = 0;
    p->y = 0;
    p->hp = MIN(p->maxHp, p->hp + p->maxHp / 4);      /* a short rest */
    p->mana = MIN(p->maxMana, p->mana + p->maxMana / 4);
    printf("You descend to floor %d and rest for a moment.\n", p->floor);
    generateDungeon(d, p);
}

/* recursive: counts the monsters from cell number idx to the end of the map */
int countMonsters(const Dungeon *d, int idx)
{
    if (idx >= MAP_SIZE * MAP_SIZE)
        return 0;
    int here = (d->cells[idx / MAP_SIZE][idx % MAP_SIZE] == T_MONSTER) ? 1 : 0;
    return here + countMonsters(d, idx + 1);
}
