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

## Controls

| Key | Action |
|-----|--------|
| w / a / s / d | move up / left / down / right (then Enter) |
| i | open backpack (use or equip items) |
| c | show stats |
| q | quit to menu |

## Map symbols

`@` you, `?` unknown, `#` wall, `M` monster, `$` gold, `C` chest, `^` trap, `>` stairs, `B` boss, `S` shop, `F` fountain
