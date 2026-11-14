/* main.c - menu and main game loop */
#include "game.h"

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

static void playNewGame(void)
{
    Player *p = createHero();
    printf("\nWelcome, %s the %s!\n", p->name, className(p->cls));
    showStats(p);
    inventoryMenu(p);
    freePlayer(p);
}

enum { M_NEW, M_LOAD, M_SCORES, M_SETTINGS, M_HELP, M_EXIT };
static const char *menuNames[] = {"New Game", "Load Game", "High Scores", "Settings", "How to Play", "Exit"};

int main(void)
{
    srand((unsigned int)time(NULL));
    showIntro();

    int running = 1;
    while (running) {
        /* the menu is built at run time, so only the options that exist are shown */
        int options[8];
        int n = 0;
        options[n++] = M_NEW;
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
            case M_HELP:     showHelp();      break;
            case M_EXIT:     running = 0;     break;
            default:         break;
        }
    }
    printf("Thanks for playing Dungeon Quest!\n");
    return 0;
}
