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

/* ---------- class skills ---------- */
typedef struct {
    const char *name;
    int cost;
} Skill;

static const Skill skillTable[CLASS_COUNT] = {
    {"Power Strike", 8},      /* Warrior */
    {"Fireball",    10},      /* Mage    */
    {"Backstab",     8}       /* Rogue   */
};

const char *skillName(ClassType c) { return skillTable[c].name; }
int skillCost(ClassType c)         { return skillTable[c].cost; }

/* returns 1 if the skill was used, 0 if there was not enough mana */
static int useSkill(Player *p, Monster *m)
{
    const Skill *s = &skillTable[p->cls];
    if (p->mana < s->cost) {
        printf("Not enough mana! %s needs %d.\n", s->name, s->cost);
        return 0;
    }
    p->mana -= s->cost;

    int dmg;
    switch (p->cls) {
        case CLASS_WARRIOR:             /* double damage */
            dmg = MAX(1, totalAttack(p) * 2 - m->defense + rand() % 4);
            break;
        case CLASS_MAGE:                /* magic ignores defense */
            dmg = totalAttack(p) * 2 + p->level * 2;
            break;
        default:                        /* rogue: double damage, half the defense counts */
            dmg = MAX(1, (totalAttack(p) - m->defense / 2) * 2 + rand() % 6);
            break;
    }
    m->hp -= dmg;
    printf("You use %s on the %s for %d damage!\n", s->name, m->name, dmg);
    return 1;
}

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
    int critChance = (p->cls == CLASS_ROGUE) ? 25 : 10;
    if (rand() % 100 < critChance) {
        dmg *= 2;
        *isCrit = 1;
    }
    return dmg;
}

static void monsterTurn(Player *p, Monster *m)
{
    int dmg = MAX(1, m->attack - totalDefense(p) / 2 + rand() % 3);
    if (p->cls == CLASS_ROGUE && rand() % 100 < 15) {
        printf("You nimbly dodge the %s's attack!\n", m->name);
        return;
    }
    p->hp -= dmg;
    printf("The %s hits you for %d damage.\n", m->name, dmg);
    if (m->special == SPECIAL_POISON && rand() % 100 < 40) {
        p->poisonTurns = 3;
        printf("You have been poisoned!\n");
    } else if (m->special == SPECIAL_DRAIN) {
        int heal = dmg / 2;
        m->hp = MIN(m->maxHp, m->hp + heal);
        printf("The %s drains %d HP from you!\n", m->name, heal);
    }
}

/* returns 1 = player won, 0 = player died, 2 = player fled */
int fight(Player *p, Monster *m, int isBoss)
{
    int enraged = 0;
    printf("\n*** A %s appears! (HP %d, ATK %d) ***\n", m->name, m->hp, m->attack);
    if (isBoss)
        printf("There is no escape from this fight!\n");

    while (p->hp > 0 && m->hp > 0) {
        printf("\n%s HP: %d/%d   |   %s HP: %d/%d\n", p->name, p->hp, p->maxHp, m->name, m->hp, m->maxHp);
        printf("[a] attack  ");
        printf("[s] %s (%d/%d mana)  ", skillName(p->cls), p->mana, p->maxMana);
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
        else if (choice == 's') {
            if (!useSkill(p, m))
                continue;                      /* no mana: the turn is not used */
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
            if (isBoss && !enraged && m->hp < m->maxHp / 2) {
                enraged = 1;
                m->attack += m->attack / 3;
                printf("The Dragon Lord roars in rage! His attacks grow stronger!\n");
            }
            monsterTurn(p, m);
        }

        /* end of the round */
        if (p->hp > 0 && p->poisonTurns > 0) {
            p->poisonTurns--;
            p->hp -= 2;
            printf("Poison burns you for 2 damage.\n");
        }
        p->mana = MIN(p->maxMana, p->mana + 2);
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
