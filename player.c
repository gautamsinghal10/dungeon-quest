/* player.c - classes, player creation, stats, inventory (dynamic memory) and levelling */
#include "game.h"

typedef struct {
    const char *name;
    int hp, attack, defense;                /* starting stats */
    int hpGain, attackGain, defenseGain;    /* gained at every level up */
    const char *desc;
} ClassInfo;

static const ClassInfo classTable[CLASS_COUNT] = {
    {"Warrior", 70, 9, 4, 12, 2, 1, "Tough fighter with lots of HP and defense."},
    {"Mage",    45, 6, 1,  7, 3, 1, "Fragile, but casts very powerful spells."},
    {"Rogue",   55, 8, 2,  9, 2, 1, "Dodges attacks and lands critical hits."}
};

const char *className(ClassType c)
{
    return classTable[c].name;
}

void showClassList(void)
{
    for (int i = 0; i < CLASS_COUNT; i++) {
        const ClassInfo *c = &classTable[i];
        printf("  %d. %-8s HP %d  ATK %d  DEF %d\n", i + 1, c->name, c->hp, c->attack, c->defense);
        printf("     %s\n", c->desc);
    }
}

Player *createPlayer(const char *name, ClassType cls)
{
    Player *p = (Player *)malloc(sizeof(Player));
    if (p == NULL) { printf("Out of memory!\n"); exit(1); }

    strncpy(p->name, name, NAME_LEN - 1);
    p->name[NAME_LEN - 1] = '\0';
    p->cls = cls;
    p->maxHp = classTable[cls].hp;
    p->hp = p->maxHp;
    p->baseAttack = classTable[cls].attack;
    p->baseDefense = classTable[cls].defense;
    p->level = 1;
    p->xp = 0;
    p->gold = 30;
    p->kills = 0;
    p->floor = 1;
    p->x = 0;
    p->y = 0;
    return p;
}

void freePlayer(Player *p)
{
    if (p == NULL) return;
    free(p);
}

/* attack / defense include the equipped weapon and armor */
int totalAttack(const Player *p)
{
    int a = p->baseAttack;
    return a;
}

int totalDefense(const Player *p)
{
    int d = p->baseDefense;
    return d;
}

void showStats(const Player *p)
{
    printf("\n--- %s the %s ---\n", p->name, className(p->cls));
    printf("Level %d\n", p->level);
    printf("HP %d/%d   Attack %d   Defense %d\n", p->hp, p->maxHp, totalAttack(p), totalDefense(p));
    printf("Gold %d\n", p->gold);
}
