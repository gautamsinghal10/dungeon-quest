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


/* ---------- data types ---------- */
typedef enum { CLASS_WARRIOR, CLASS_MAGE, CLASS_ROGUE, CLASS_COUNT } ClassType;

typedef struct {
    char      name[NAME_LEN];
    ClassType cls;
    int       hp, maxHp;
    int       baseAttack, baseDefense;
    int       level, xp;
    int       gold, kills;
    int       floor;
    int       x, y;                 /* x = column, y = row */
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






#endif
