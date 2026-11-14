/* utils.c - safe input helpers (fgets instead of scanf) and colour helpers */
#include "game.h"
#ifdef _WIN32
#include <windows.h>
#endif

int g_useColor   = 1;
int g_difficulty = 1;

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

/* old Windows consoles need ANSI colours switched on first */
void enableColorSupport(void)
{
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004);     /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
#endif
}

void setColor(const char *code)
{
    if (g_useColor)
        printf("%s", code);
}

/* draws [#####-----] style bar */
void printBar(int cur, int max, int width, const char *color)
{
    int filled = (max > 0) ? cur * width / max : 0;
    if (cur > 0 && filled == 0) filled = 1;
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;

    setColor(color);
    printf("[");
    for (int i = 0; i < width; i++)
        putchar(i < filled ? '#' : '-');
    printf("]");
    setColor(C_RESET);
}
