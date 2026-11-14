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
    Item     *inventory;            /* dynamic array (malloc / realloc) */
    int       invCount, invCap;
    Item      weapon, armor;        /* equipped items */
    int       hasWeapon, hasArmor;
} Player;





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

/* ---------- items.c ---------- */
Item makeItem(const char *name, ItemType type, int value, int price);
Item randomItem(int floor);
void printItem(const Item *it);





#endif
