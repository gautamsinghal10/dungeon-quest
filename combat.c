/* combat.c - monsters and turn-based battles */
#include "game.h"

typedef struct {
    const char *name;
    int minFloor;                   /* first floor where it can appear */
    int hp, attack, defense;
    int xp, gold;
    int special;
} MonsterTemplate;

static const MonsterTemplate monsterTable[] = {
    {"Slime",        1, 14,  3, 0,  8,  4, SPECIAL_NONE},
    {"Goblin",       1, 18,  4, 1, 10,  6, SPECIAL_NONE},
    {"Giant Bat",    1, 12,  5, 0,  9,  5, SPECIAL_DRAIN},
    {"Skeleton",     2, 26,  7, 2, 16, 10, SPECIAL_NONE},
    {"Giant Spider", 2, 22,  6, 1, 15,  9, SPECIAL_POISON},
    {"Orc",          3, 38,  9, 3, 22, 14, SPECIAL_NONE},
    {"Wraith",       3, 30, 10, 2, 24, 16, SPECIAL_DRAIN},
    {"Dark Knight",  4, 48, 12, 5, 32, 22, SPECIAL_NONE},
    {"Troll",        4, 60, 11, 4, 34, 24, SPECIAL_NONE},
    {"Venom Drake",  5, 55, 14, 5, 45, 35, SPECIAL_POISON}
};
#define MONSTER_TYPES ((int)(sizeof(monsterTable) / sizeof(monsterTable[0])))

Monster makeMonster(int floor, int isBoss)
{
    Monster m;

    if (isBoss) {
        strcpy(m.name, "Dragon Lord");
        m.hp = 120;
        m.attack = 16;
        m.defense = 8;
        m.xpReward = 150;
        m.goldReward = 150;
        m.special = SPECIAL_NONE;
    } else {
        int candidates[16];
        int n = 0;
        for (int i = 0; i < MONSTER_TYPES; i++)
            if (monsterTable[i].minFloor <= floor)
                candidates[n++] = i;

        const MonsterTemplate *t = &monsterTable[candidates[rand() % n]];
        int scale = 100 + (floor - t->minFloor) * 20;     /* older monsters get stronger */

        strncpy(m.name, t->name, ITEM_NAME_LEN - 1);
        m.name[ITEM_NAME_LEN - 1] = '\0';
        m.hp = t->hp * scale / 100 + rand() % 5;
        m.attack = t->attack * scale / 100;
        m.defense = t->defense + (floor - t->minFloor) / 2;
        m.xpReward = t->xp;
        m.goldReward = t->gold + rand() % 6;
        m.special = t->special;
    }
    m.maxHp = m.hp;
    return m;
}

/* damage of a normal attack; the Rogue crits more often */
static int calcPlayerDamage(const Player *p, const Monster *m, int *isCrit)
{
    int dmg = MAX(1, totalAttack(p) - m->defense + rand() % 4);
    *isCrit = 0;
    return dmg;
}

static void monsterTurn(Player *p, Monster *m)
{
    int dmg = MAX(1, m->attack - totalDefense(p) / 2 + rand() % 3);
    p->hp -= dmg;
    printf("The %s hits you for %d damage.\n", m->name, dmg);
}

/* returns 1 = player won, 0 = player died, 2 = player fled */
int fight(Player *p, Monster *m, int isBoss)
{
    printf("\n*** A %s appears! (HP %d, ATK %d) ***\n", m->name, m->hp, m->attack);
    if (isBoss)
        printf("There is no escape from this fight!\n");

    while (p->hp > 0 && m->hp > 0) {
        printf("\n%s HP: %d/%d   |   %s HP: %d/%d\n", p->name, p->hp, p->maxHp, m->name, m->hp, m->maxHp);
        printf("[a] attack  ");
        printf("[u] use item  ");
        if (!isBoss) printf("[f] flee");
        printf(": ");

        char choice = getChoice();

        if (choice == 'a') {
            int crit;
            int dmg = calcPlayerDamage(p, m, &crit);
            m->hp -= dmg;
            if (crit) printf("CRITICAL HIT! ");
            printf("You hit the %s for %d damage.\n", m->name, dmg);
        }
        else if (choice == 'u') {
            if (p->invCount == 0) {
                printf("Your backpack is empty!\n");
                continue;
            }
            showInventory(p);
            printf("Item number: ");
            int n = getNumber();
            if (n < 1 || n > p->invCount) {
                printf("Invalid choice.\n");
                continue;
            }
            useItem(p, n - 1);
        } else if (choice == 'f' && !isBoss) {
            if (rand() % 2) {
                printf("You escaped!\n");
                return 2;
            }
            printf("You failed to escape!\n");
        } else {
            printf("Invalid choice.\n");
            continue;
        }

        if (m->hp > 0) {
            monsterTurn(p, m);
        }
    }

    if (p->hp <= 0)
        return 0;

    printf("\nYou defeated the %s! +%d XP, +%d gold.\n", m->name, m->xpReward, m->goldReward);
    p->gold += m->goldReward;
    p->kills++;
    gainXp(p, m->xpReward);
    if (!isBoss && rand() % 100 < 25) {              /* 25% chance of loot */
        Item drop = randomItem(p->floor);
        printf("The %s dropped: ", m->name);
        printItem(&drop);
        printf("\n");
        addItem(p, drop);
    }
    return 1;
}
