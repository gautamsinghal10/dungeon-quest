# Dungeon Quest

A text-based dungeon crawler written entirely in C (CSE Semester 1 project).
Pick a class, explore a fog-of-war dungeon, fight monsters, loot chests and defeat the Dragon Lord.

## Build and run

    gcc -Wall -std=c99 *.c -o dungeon_quest
    ./dungeon_quest          (Windows: dungeon_quest.exe)

Or use `make`.

## Features

- Three classes (Warrior, Mage, Rogue) with different stats
- Backpack that grows as needed, with equippable weapons and armor
- Random dungeon floors with walls, fog of war, and a reachable-exit check (flood fill)
- Turn-based combat, XP and levelling, 10 monster types
- Gold, chests, traps, loot drops, 5 floors and a final boss
- Class skills with mana, critical hits, poison, life-drain and a boss that enrages
- Wandering merchant (buy and sell) and healing fountains
- 3 save slots (binary files) and a top-5 high-score table (text file)
- Coloured map and health bars, easy / normal / hard difficulty

## Controls

| Key | Action |
|-----|--------|
| w / a / s / d | move up / left / down / right (then Enter) |
| i | open backpack (use or equip items) |
| c | show stats |
| v | save game |
| q | quit to menu |

## Map symbols

`@` you, `?` unknown, `#` wall, `M` monster, `$` gold, `C` chest, `^` trap, `>` stairs, `B` boss, `S` shop, `F` fountain

## Project structure

| File | Purpose |
|------|---------|
| game.h | structs, union, enums, macros, prototypes |
| main.c | menu, game loop, settings |
| player.c | classes, stats, inventory, levelling |
| items.c | item creation (union) and loot tables |
| dungeon.c | map generation, movement, tile events |
| combat.c | monsters, skills, battles |
| shop.c | merchant |
| save.c | save slots and high scores (file handling) |
| utils.c | input and colour helpers |

## Syllabus concepts used

- **Unit 1:** algorithm of the game loop, data types, operators
- **Unit 2:** `#define`, macros (`MIN`, `MAX`), include guard, `#ifdef DEBUG`, `if / switch`, loops
- **Unit 3:** 2D arrays (map), strings, functions, recursion (`xpNeeded`, `countMonsters`, `floodFill`)
- **Unit 4:** pointers, `malloc / realloc / free`, `struct`, `union`, `enum`, file handling (`fread / fwrite`, `fprintf / fscanf`)

Debug build: `make debug` then press `x` in the game to reveal the whole map.
