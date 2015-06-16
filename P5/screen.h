#ifndef SCREEN_H
#define SCREEN_H

void screen_setup (int prio, int col, int lin);
void screen_refresh (void);

void screen_clear (void);
void screen_printxy (int x, int y, const char* txt);
int screen_getchar (void);

#endif
