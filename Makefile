# Build with: make        (or: gcc -Wall -std=c99 *.c -o dungeon_quest)
CC     = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS   = main.o utils.o
OBJS  += player.o

dungeon_quest: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c game.h
	$(CC) $(CFLAGS) -c $<

# debug build: press x in the game to reveal the whole map
debug: CFLAGS += -DDEBUG -g
debug: clean dungeon_quest

clean:
	rm -f *.o dungeon_quest dungeon_quest.exe
