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

/* starting mana of each class */
static const int classMana[CLASS_COUNT] = {20, 50, 30};

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
        printf("     Skill: %s (%d mana)\n", skillName((ClassType)i), skillCost((ClassType)i));
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
    p->steps = 0;
    p->hasWeapon = 0;
    p->hasArmor = 0;
    p->invCap = 4;
    p->invCount = 0;
    p->inventory = (Item *)malloc(sizeof(Item) * p->invCap);
    if (p->inventory == NULL) { printf("Out of memory!\n"); exit(1); }
    addItem(p, makeItem("Small Potion", ITEM_POTION, 25, 10));    /* starter item */
    p->maxMana = classMana[cls];
    p->mana = p->maxMana;
    p->poisonTurns = 0;
    return p;
}

void freePlayer(Player *p)
{
    if (p == NULL) return;
    free(p->inventory);
    free(p);
}

/* attack / defense include the equipped weapon and armor */
int totalAttack(const Player *p)
{
    int a = p->baseAttack;
    if (p->hasWeapon) a += p->weapon.effect.attackBonus;
    return a;
}

int totalDefense(const Player *p)
{
    int d = p->baseDefense;
    if (p->hasArmor) d += p->armor.effect.defenseBonus;
    return d;
}

void showStats(const Player *p)
{
    printf("\n--- %s the %s ---\n", p->name, className(p->cls));
    printf("Level %d   XP %d/%d\n", p->level, p->xp, xpNeeded(p->level));
    printf("HP %d/%d   Attack %d   Defense %d\n", p->hp, p->maxHp, totalAttack(p), totalDefense(p));
    printf("Mana %d/%d", p->mana, p->maxMana);
    if (p->poisonTurns > 0) printf("   (POISONED for %d more turns)", p->poisonTurns);
    printf("\n");
    printf("Gold %d\n", p->gold);
    printf("Monsters slain %d   Floor %d/%d\n", p->kills, p->floor, MAX_FLOORS);
    printf("Weapon: ");
    if (p->hasWeapon) printItem(&p->weapon); else printf("none");
    printf("\nArmor:  ");
    if (p->hasArmor) printItem(&p->armor); else printf("none");
    printf("\n");
}

/* ---------- inventory ---------- */

void addItem(Player *p, Item it)
{
    if (p->invCount == p->invCap) {               /* backpack full: grow the array */
        int newCap = p->invCap * 2;
        Item *tmp = (Item *)realloc(p->inventory, sizeof(Item) * newCap);
        if (tmp == NULL) { printf("Your backpack is too heavy!\n"); return; }
        p->inventory = tmp;
        p->invCap = newCap;
    }
    p->inventory[p->invCount++] = it;
}

void removeItem(Player *p, int index)
{
    if (index < 0 || index >= p->invCount) return;
    for (int i = index; i < p->invCount - 1; i++)
        p->inventory[i] = p->inventory[i + 1];
    p->invCount--;
}

void useItem(Player *p, int index)
{
    if (index < 0 || index >= p->invCount) {
        printf("Invalid item.\n");
        return;
    }
    Item chosen = p->inventory[index];     /* copy it, then take it out of the backpack */
    removeItem(p, index);

    switch (chosen.type) {
        case ITEM_POTION: {
            int before = p->hp;
            p->hp = MIN(p->maxHp, p->hp + chosen.effect.healAmount);
            printf("You drink the %s and recover %d HP.\n", chosen.name, p->hp - before);
            if (p->poisonTurns > 0) {
                p->poisonTurns = 0;
                printf("The potion also cures your poison.\n");
            }
            break;
        }
        case ITEM_ELIXIR: {
            int before = p->mana;
            p->mana = MIN(p->maxMana, p->mana + chosen.effect.manaAmount);
            printf("You drink the %s and recover %d mana.\n", chosen.name, p->mana - before);
            break;
        }
        case ITEM_WEAPON: {
            Item old = p->weapon;
            int hadOld = p->hasWeapon;
            p->weapon = chosen;
            p->hasWeapon = 1;
            printf("You equip the %s. Attack is now %d.\n", chosen.name, totalAttack(p));
            if (hadOld) {
                addItem(p, old);
                printf("Your old %s goes back into the backpack.\n", old.name);
            }
            break;
        }
        case ITEM_ARMOR: {
            Item old = p->armor;
            int hadOld = p->hasArmor;
            p->armor = chosen;
            p->hasArmor = 1;
            printf("You wear the %s. Defense is now %d.\n", chosen.name, totalDefense(p));
            if (hadOld) {
                addItem(p, old);
                printf("Your old %s goes back into the backpack.\n", old.name);
            }
            break;
        }
    }
}

void showInventory(const Player *p)
{
    printf("\n--- Backpack (%d items) ---\n", p->invCount);
    if (p->invCount == 0) {
        printf("  (empty)\n");
        return;
    }
    for (int i = 0; i < p->invCount; i++) {
        printf("  %d. ", i + 1);
        printItem(&p->inventory[i]);
        printf("\n");
    }
}

/* ---------- levelling ---------- */

/* recursive: XP needed to go from `level` to `level + 1` */
int xpNeeded(int level)
{
    if (level <= 1)
        return 30;
    return xpNeeded(level - 1) + level * 20;
}

void gainXp(Player *p, int amount)
{
    p->xp += amount;
    while (p->xp >= xpNeeded(p->level)) {
        const ClassInfo *c = &classTable[p->cls];
        p->xp -= xpNeeded(p->level);
        p->level++;
        p->maxHp += c->hpGain;
        p->hp = p->maxHp;
        p->baseAttack += c->attackGain;
        p->baseDefense += c->defenseGain;
        p->maxMana += (p->cls == CLASS_MAGE) ? 6 : 3;
        p->mana = p->maxMana;
        printf("*** LEVEL UP! You are now level %d. HP restored! ***\n", p->level);
    }
}

int calcScore(const Player *p)
{
    return p->gold * 2 + p->kills * 10 + p->floor * 100 + p->level * 50;
}
