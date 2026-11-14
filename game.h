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






/* ---------- utils.c ---------- */
void getLine(char *buf, int n);
char getChoice(void);
int  getNumber(void);







#endif
