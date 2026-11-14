/* utils.c - safe input helpers (fgets instead of scanf) and colour helpers */
#include "game.h"


void getLine(char *buf, int n)
{
    if (fgets(buf, n, stdin) == NULL) {      /* input closed (Ctrl+D / Ctrl+Z) */
        printf("\nInput closed. Goodbye!\n");
        exit(0);
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

char getChoice(void)
{
    char buf[16];
    getLine(buf, sizeof(buf));
    return (char)tolower((unsigned char)buf[0]);
}

int getNumber(void)
{
    char buf[16];
    getLine(buf, sizeof(buf));
    return atoi(buf);
}
