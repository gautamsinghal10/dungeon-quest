/* game.h - shared definitions for Dungeon Quest
 * Concepts used here: preprocessor (Unit 2), struct / union / enum (Unit 4)
 */
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---------- constants and macros ---------- */
#define NAME_LEN       30

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ITEM_NAME_LEN  24
#define MAP_SIZE       10
#define MAX_FLOORS     5

/* tile symbols on the map */
#define T_EMPTY    '.'
#define T_WALL     '#'
#define T_MONSTER  'M'
#define T_GOLD     '$'
#define T_CHEST    'C'
#define T_TRAP     '^'
#define T_EXIT     '>'
#define T_BOSS     'B'
#define T_SHOP     'S'
#define T_FOUNTAIN 'F'

/* ---------- data types ---------- */
typedef enum { CLASS_WARRIOR, CLASS_MAGE, CLASS_ROGUE, CLASS_COUNT } ClassType;
typedef enum {
    ITEM_POTION,
    ITEM_WEAPON,
    ITEM_ARMOR
} ItemType;

/* only one effect is used at a time, so a union saves memory */
typedef union {
    int healAmount;      /* potion */
    int attackBonus;     /* weapon */
    int defenseBonus;    /* armor  */
} ItemEffect;

typedef struct {
    char       name[ITEM_NAME_LEN];
    ItemType   type;
    ItemEffect effect;
    int        price;
} Item;

typedef struct {
    char      name[NAME_LEN];
    ClassType cls;
    int       hp, maxHp;
    int       baseAttack, baseDefense;
    int       level, xp;
    int       gold, kills;
    int       floor;
    int       x, y;                 /* x = column, y = row */
    int       steps;
    Item     *inventory;            /* dynamic array (malloc / realloc) */
    int       invCount, invCap;
    Item      weapon, armor;        /* equipped items */
    int       hasWeapon, hasArmor;
} Player;

#define SPECIAL_NONE   0
#define SPECIAL_POISON 1
#define SPECIAL_DRAIN  2

typedef struct {
    char name[ITEM_NAME_LEN];
    int  hp, maxHp, attack, defense;
    int  xpReward, goldReward;
    int  special;
} Monster;

typedef struct {
    char cells[MAP_SIZE][MAP_SIZE];
    int  seen[MAP_SIZE][MAP_SIZE];      /* 1 = revealed to the player */
} Dungeon;



/* ---------- utils.c ---------- */
void getLine(char *buf, int n);
char getChoice(void);
int  getNumber(void);

/* ---------- player.c ---------- */
Player     *createPlayer(const char *name, ClassType cls);
void        freePlayer(Player *p);
const char *className(ClassType c);
void        showClassList(void);
void        showStats(const Player *p);
int         totalAttack(const Player *p);
int         totalDefense(const Player *p);
void        addItem(Player *p, Item it);
void        removeItem(Player *p, int index);
void        useItem(Player *p, int index);
void        showInventory(const Player *p);
int         xpNeeded(int level);              /* recursive */
void        gainXp(Player *p, int amount);
int         calcScore(const Player *p);

/* ---------- items.c ---------- */
Item makeItem(const char *name, ItemType type, int value, int price);
Item randomItem(int floor);
void printItem(const Item *it);

/* ---------- combat.c ---------- */
Monster makeMonster(int floor, int isBoss);
int     fight(Player *p, Monster *m, int isBoss);

/* ---------- dungeon.c ---------- */
void generateDungeon(Dungeon *d, const Player *p);
void printDungeon(const Dungeon *d, const Player *p);
int  movePlayer(Player *p, Dungeon *d, char dir);
void revealAll(Dungeon *d);
int  handleTile(Player *p, Dungeon *d, int prevX, int prevY);
int  countMonsters(const Dungeon *d, int idx);   /* recursive */
void nextFloor(Player *p, Dungeon *d);



#endif
