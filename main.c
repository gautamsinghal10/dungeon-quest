/* main.c - menu and main game loop */
#include "game.h"

static int currentSlot = 0;        /* save slot of the running game (0 = none) */

static void showIntro(void)
{
    printf("\n==============================================\n");
    printf("                DUNGEON QUEST\n");
    printf("==============================================\n");
    printf("The Dragon Lord has stolen the kingdom's crown and\n");
    printf("hidden it in the depths of a cursed dungeon.\n");
    printf("Fight through the floors, collect treasure, level up\n");
    printf("and defeat the Dragon Lord to win!\n");
}

static void showHelp(void)
{
    printf("\n----- HOW TO PLAY -----\n");
    printf("Move with w (up), a (left), s (down), d (right) and press Enter.\n");
    printf("The map starts hidden. You can see the tiles next to you.\n");
    printf("M = monster  $ = gold  C = chest  ^ = trap  # = wall\n");
    printf("S = shop  F = fountain  > = stairs down  B = final boss\n");
    printf("Reach the stairs on every floor, then defeat the boss on the last floor.\n");
    printf("Equip weapons and armor from your backpack and keep potions for hard fights.\n");
    printf("Your score goes into the high-score table when the game ends.\n");
}

static Player *createHero(void)
{
    char name[NAME_LEN];
    printf("\nEnter your hero's name: ");
    getLine(name, sizeof(name));
    if (name[0] == '\0')
        strcpy(name, "Hero");

    printf("\nChoose your class:\n");
    showClassList();

    int choice = 0;
    while (choice < 1 || choice > CLASS_COUNT) {
        printf("Class number: ");
        choice = getNumber();
    }
    return createPlayer(name, (ClassType)(choice - 1));
}

/* lets the player use or equip items until they type 0 */
static void inventoryMenu(Player *p)
{
    while (1) {
        showInventory(p);
        if (p->invCount == 0)
            return;
        printf("Item number to use or equip (0 to go back): ");
        int n = getNumber();
        if (n == 0)
            return;
        if (n < 1 || n > p->invCount) {
            printf("Invalid choice.\n");
            continue;
        }
        useItem(p, n - 1);
    }
}

static void gameOver(const Player *p, int won)
{
    printf("\n==============================================\n");
    if (won)
        printf("  VICTORY! The Dragon Lord is slain, %s!\n", p->name);
    else
        printf("  GAME OVER. %s has fallen on floor %d...\n", p->name, p->floor);
    printf("==============================================\n");

    int score = calcScore(p);
    if (won) score += 500;
    printf("Final score: %d\n", score);
    printf("Level %d | Steps %d | Monsters slain %d | Gold %d\n", p->level, p->steps, p->kills, p->gold);
    updateHighScores(p->name, score, (int)p->cls);
    if (currentSlot > 0) {                /* the run is over, so delete its save */
        deleteSave(currentSlot);
        currentSlot = 0;
    }
}

static void printStatus(const Dungeon *d, const Player *p)
{
    (void)d;
    printf("HP ");
    printBar(p->hp, p->maxHp, 12, (p->hp * 3 < p->maxHp) ? C_RED : C_GREEN);
    printf(" %d/%d", p->hp, p->maxHp);
    printf(" | MP %d/%d", p->mana, p->maxMana);
    if (p->poisonTurns > 0) printf(" | POISONED");
    printf(" | Lv %d | Gold %d", p->level, p->gold);
    printf(" | Monsters left: %d", countMonsters(d, 0));
    printf("\n");
}

/* shows the save slots and asks which one to use; returns 0 if cancelled */
static int chooseSlot(const char *action)
{
    printf("\n--- %s: choose a slot ---\n", action);
    for (int s = 1; s <= MAX_SLOTS; s++) {
        char info[96];
        if (slotInfo(s, info, sizeof(info)))
            printf("  %d. %s\n", s, info);
        else
            printf("  %d. (empty)\n", s);
    }
    printf("Slot number (0 to cancel): ");
    int n = getNumber();
    return (n >= 1 && n <= MAX_SLOTS) ? n : 0;
}

static void gameLoop(Player *p, Dungeon *d)
{
    int running = 1;

    while (running) {
        printDungeon(d, p);
        printStatus(d, p);
        printf("[w/a/s/d] move  [i] inventory  [c] stats  ");
        printf("[v] save  ");
        printf("[q] quit: ");

        char ch = getChoice();
        switch (ch) {
            case 'w': case 'a': case 's': case 'd': {
                int px = p->x, py = p->y;
                if (!movePlayer(p, d, ch)) {
                    printf("Something blocks your way.\n");
                    break;
                }
                int res = handleTile(p, d, px, py);
                if (res == -1) {
                    gameOver(p, 0);
                    running = 0;
                }
                else if (res == 2) {
                    gameOver(p, 1);
                    running = 0;
                } else if (res == 1) {
                    nextFloor(p, d);
                }
                break;
            }
            case 'i':
                inventoryMenu(p);
                break;
            case 'c':
                showStats(p);
                break;
            case 'v': {
                int slot = chooseSlot("Save");
                if (slot > 0) {
                    if (saveGame(p, d, slot)) {
                        currentSlot = slot;
                        printf("Game saved to slot %d.\n", slot);
                    } else {
                        printf("Could not save the game.\n");
                    }
                }
                break;
            }
            case 'q':
                printf("Return to the menu? Unsaved progress is lost. (y/n): ");
                if (getChoice() == 'y')
                    running = 0;
                break;
#ifdef DEBUG
            case 'x':
                revealAll(d);
                printf("[DEBUG] whole map revealed\n");
                break;
#endif
            default:
                printf("Unknown command.\n");
        }
    }
}

static void playNewGame(void)
{
    Player *p = createHero();
    printf("\nWelcome, %s the %s!\n", p->name, className(p->cls));
    currentSlot = 0;
    Dungeon d;
    generateDungeon(&d, p);
    gameLoop(p, &d);
    freePlayer(p);
}

static void loadGameMenu(void)
{
    int slot = chooseSlot("Load");
    if (slot == 0)
        return;

    Dungeon d;
    Player *p = loadGame(&d, slot);
    if (p == NULL) {
        printf("That slot is empty or the save file is damaged.\n");
        return;
    }
    currentSlot = slot;
    printf("\nWelcome back, %s the %s!\n", p->name, className(p->cls));
    gameLoop(p, &d);
    freePlayer(p);
}

static void settingsMenu(void)
{
    static const char *diffNames[] = {"Easy", "Normal", "Hard"};
    int open = 1;

    while (open) {
        printf("\n===== SETTINGS =====\n");
        printf("1. Colours: %s\n", g_useColor ? "ON" : "OFF");
        printf("2. Difficulty: %s\n", diffNames[g_difficulty]);
        printf("3. Back\nChoice: ");
        switch (getNumber()) {
            case 1: g_useColor = !g_useColor;              break;
            case 2: g_difficulty = (g_difficulty + 1) % 3; break;
            case 3: open = 0;                              break;
            default: printf("Invalid choice.\n");
        }
    }
}

enum { M_NEW, M_LOAD, M_SCORES, M_SETTINGS, M_HELP, M_EXIT };
static const char *menuNames[] = {"New Game", "Load Game", "High Scores", "Settings", "How to Play", "Exit"};

int main(void)
{
    srand((unsigned int)time(NULL));
    enableColorSupport();
    showIntro();

    int running = 1;
    while (running) {
        /* the menu is built at run time, so only the options that exist are shown */
        int options[8];
        int n = 0;
        options[n++] = M_NEW;
        options[n++] = M_LOAD;
        options[n++] = M_SCORES;
        options[n++] = M_SETTINGS;
        options[n++] = M_HELP;
        options[n++] = M_EXIT;

        printf("\n===== MAIN MENU =====\n");
        for (int i = 0; i < n; i++)
            printf("%d. %s\n", i + 1, menuNames[options[i]]);
        printf("Choice: ");

        int choice = getNumber();
        if (choice < 1 || choice > n) {
            printf("Please enter a number from 1 to %d.\n", n);
            continue;
        }

        switch (options[choice - 1]) {
            case M_NEW:      playNewGame();   break;
            case M_LOAD:     loadGameMenu();  break;
            case M_SCORES:   showHighScores(); break;
            case M_SETTINGS: settingsMenu();  break;
            case M_HELP:     showHelp();      break;
            case M_EXIT:     running = 0;     break;
            default:         break;
        }
    }
    printf("Thanks for playing Dungeon Quest!\n");
    return 0;
}
