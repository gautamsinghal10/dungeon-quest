/* shop.c - the wandering merchant (buy and sell items) */
#include "game.h"

/* fills the merchant's shelves for a new floor */
void stockShop(Dungeon *d, int floor)
{
    d->shopItem[0] = makeItem("Health Potion", ITEM_POTION, 30 + floor * 10, 20 + floor * 6);
    for (int i = 1; i < SHOP_SLOTS; i++)
        d->shopItem[i] = randomItem(floor);
    for (int i = 0; i < SHOP_SLOTS; i++)
        d->shopSold[i] = 0;
}

void openShop(Player *p, Dungeon *d)
{
    int open = 1;
    printf("\n=== The Wandering Merchant ===\n");
    printf("\"Welcome, traveller! Take a look at my wares.\"\n");

    while (open) {
        printf("\nYour gold: %d\n", p->gold);
        for (int i = 0; i < SHOP_SLOTS; i++) {
            printf("  %d. ", i + 1);
            if (d->shopSold[i]) {
                printf("[SOLD]\n");
            } else {
                printItem(&d->shopItem[i]);
                printf(" - %d gold\n", d->shopItem[i].price);
            }
        }
        printf("[b] buy  [s] sell  [l] leave: ");
        char ch = getChoice();

        if (ch == 'b') {
            printf("Item number: ");
            int n = getNumber();
            if (n < 1 || n > SHOP_SLOTS) {
                printf("Invalid choice.\n");
            } else if (d->shopSold[n - 1]) {
                printf("That item is already sold.\n");
            } else if (p->gold < d->shopItem[n - 1].price) {
                printf("You do not have enough gold.\n");
            } else {
                p->gold -= d->shopItem[n - 1].price;
                d->shopSold[n - 1] = 1;
                addItem(p, d->shopItem[n - 1]);
                printf("You bought the %s.\n", d->shopItem[n - 1].name);
            }
        } else if (ch == 's') {
            if (p->invCount == 0) {
                printf("You have nothing to sell.\n");
                continue;
            }
            showInventory(p);
            printf("Item number to sell (0 to cancel): ");
            int n = getNumber();
            if (n >= 1 && n <= p->invCount) {
                int gain = MAX(1, p->inventory[n - 1].price / 2);
                printf("You sold the %s for %d gold.\n", p->inventory[n - 1].name, gain);
                p->gold += gain;
                removeItem(p, n - 1);
            }
        } else if (ch == 'l') {
            open = 0;
            printf("\"Come back soon!\"\n");
        } else {
            printf("Invalid choice.\n");
        }
    }
}
