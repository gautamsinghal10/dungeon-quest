/* items.c - item creation (uses the union ItemEffect) */
#include "game.h"

Item makeItem(const char *name, ItemType type, int value, int price)
{
    Item it;
    strncpy(it.name, name, ITEM_NAME_LEN - 1);
    it.name[ITEM_NAME_LEN - 1] = '\0';
    it.type = type;
    it.price = price;
    /* the same memory holds whichever effect matches the item type */
    switch (type) {
        case ITEM_POTION: it.effect.healAmount   = value; break;
        case ITEM_WEAPON: it.effect.attackBonus  = value; break;
        case ITEM_ARMOR:  it.effect.defenseBonus = value; break;
    }
    return it;
}

/* better loot is generated on deeper floors */
Item randomItem(int floor)
{
    static const char *weapons[] = {"Rusty Dagger", "Iron Sword", "Steel Axe", "Flame Blade", "Dragon Slayer"};
    static const int weaponBonus[] = {2, 4, 6, 9, 12};
    static const char *armors[] = {"Leather Vest", "Chain Mail", "Knight Shield", "Plate Armor", "Dragon Scale"};
    static const int armorBonus[] = {1, 2, 4, 6, 9};

    int tier = floor - 1;
    if (rand() % 3 == 0) tier--;                 /* sometimes a weaker drop */
    tier = MAX(0, MIN(4, tier));

    int roll = rand() % 100;
    if (roll < 40) {
        const char *name = (floor <= 2) ? "Small Potion" : (floor <= 4) ? "Health Potion" : "Large Potion";
        int heal = 20 + floor * 8;
        return makeItem(name, ITEM_POTION, heal, 10 + floor * 5);
    } else if (roll < 75) {
        int bonus = weaponBonus[tier] + rand() % 2;
        return makeItem(weapons[tier], ITEM_WEAPON, bonus, bonus * 15);
    } else {
        int bonus = armorBonus[tier] + rand() % 2;
        return makeItem(armors[tier], ITEM_ARMOR, bonus, bonus * 18);
    }
}

void printItem(const Item *it)
{
    switch (it->type) {
        case ITEM_POTION: printf("%s (heals %d HP)", it->name, it->effect.healAmount);   break;
        case ITEM_WEAPON: printf("%s (+%d attack)",  it->name, it->effect.attackBonus);  break;
        case ITEM_ARMOR:  printf("%s (+%d defense)", it->name, it->effect.defenseBonus); break;
    }
}
