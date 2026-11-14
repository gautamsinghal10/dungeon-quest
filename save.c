/* save.c - file handling: binary save slots and a text high-score table */
#include "game.h"

#define SAVE_MAGIC 0x44514D31          /* marks a file as a Dungeon Quest save */

static void slotPath(int slot, char *buf, size_t n)
{
    snprintf(buf, n, "save%d.dat", slot);
}

/* basic sanity check so a damaged save file cannot crash the game */
static int validPlayer(const Player *p)
{
    return p->cls >= 0 && p->cls < CLASS_COUNT &&
           p->invCount >= 0 && p->invCount <= 1000 &&
           p->floor >= 1 && p->floor <= MAX_FLOORS &&
           p->x >= 0 && p->x < MAP_SIZE && p->y >= 0 && p->y < MAP_SIZE;
}

int saveGame(const Player *p, const Dungeon *d, int slot)
{
    char path[32];
    int magic = SAVE_MAGIC;
    slotPath(slot, path, sizeof(path));

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) return 0;

    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(p, sizeof(Player), 1, fp);                       /* the player struct    */
    fwrite(p->inventory, sizeof(Item), p->invCount, fp);    /* the backpack items   */
    fwrite(d, sizeof(Dungeon), 1, fp);                      /* the current floor    */
    fclose(fp);
    return 1;
}

/* returns a new Player (free it with freePlayer) or NULL if the slot is empty or damaged */
Player *loadGame(Dungeon *d, int slot)
{
    char path[32];
    int magic = 0;
    slotPath(slot, path, sizeof(path));

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return NULL;

    Player *p = (Player *)malloc(sizeof(Player));
    if (p == NULL) { fclose(fp); return NULL; }

    if (fread(&magic, sizeof(magic), 1, fp) != 1 || magic != SAVE_MAGIC ||
        fread(p, sizeof(Player), 1, fp) != 1 || !validPlayer(p)) {
        free(p);
        fclose(fp);
        return NULL;
    }

    /* the saved pointer is meaningless now, so allocate a fresh backpack */
    p->invCap = MAX(4, p->invCount);
    p->inventory = (Item *)malloc(sizeof(Item) * p->invCap);
    if (p->inventory == NULL) { free(p); fclose(fp); return NULL; }

    if (fread(p->inventory, sizeof(Item), p->invCount, fp) != (size_t)p->invCount ||
        fread(d, sizeof(Dungeon), 1, fp) != 1) {
        freePlayer(p);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return p;
}

/* writes a short description of a slot into buf; returns 0 if the slot is empty */
int slotInfo(int slot, char *buf, size_t n)
{
    char path[32];
    int magic = 0;
    Player tmp;
    slotPath(slot, path, sizeof(path));

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return 0;

    if (fread(&magic, sizeof(magic), 1, fp) != 1 || magic != SAVE_MAGIC ||
        fread(&tmp, sizeof(Player), 1, fp) != 1 || !validPlayer(&tmp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    snprintf(buf, n, "%s the %s, level %d, floor %d", tmp.name, className(tmp.cls), tmp.level, tmp.floor);
    return 1;
}

void deleteSave(int slot)
{
    char path[32];
    slotPath(slot, path, sizeof(path));
    remove(path);
}

/* ---------- high scores (text file) ---------- */

void updateHighScores(const char *name, int score, int cls)
{
    ScoreEntry list[MAX_SCORES + 1];
    int n = 0;

    FILE *fp = fopen(SCORE_FILE, "r");
    if (fp != NULL) {
        while (n < MAX_SCORES &&
               fscanf(fp, "%29s %d %d", list[n].name, &list[n].score, &list[n].cls) == 3)
            n++;
        fclose(fp);
    }

    /* add the new entry; spaces become underscores so fscanf can read the name back */
    strncpy(list[n].name, name, NAME_LEN - 1);
    list[n].name[NAME_LEN - 1] = '\0';
    for (char *c = list[n].name; *c; c++)
        if (*c == ' ') *c = '_';
    list[n].score = score;
    list[n].cls = cls;
    n++;

    /* bubble sort: highest score first */
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (list[j].score < list[j + 1].score) {
                ScoreEntry tmp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = tmp;
            }
    if (n > MAX_SCORES) n = MAX_SCORES;

    fp = fopen(SCORE_FILE, "w");
    if (fp == NULL) return;
    for (int i = 0; i < n; i++)
        fprintf(fp, "%s %d %d\n", list[i].name, list[i].score, list[i].cls);
    fclose(fp);
}

void showHighScores(void)
{
    printf("\n===== HIGH SCORES =====\n");
    FILE *fp = fopen(SCORE_FILE, "r");
    if (fp == NULL) {
        printf("No scores yet. Be the first hero!\n");
        return;
    }
    char name[NAME_LEN];
    int score, cls, rank = 1;
    while (fscanf(fp, "%29s %d %d", name, &score, &cls) == 3) {
        const char *cname = (cls >= 0 && cls < CLASS_COUNT) ? className((ClassType)cls) : "?";
        printf("%d. %-20s %-8s %d\n", rank++, name, cname, score);
    }
    fclose(fp);
    if (rank == 1)
        printf("No scores yet. Be the first hero!\n");
}
