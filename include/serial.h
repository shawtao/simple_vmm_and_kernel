#ifndef _SERIAL_H
#define _SERIAL_H

#include <termios.h>
#include <stdio.h>

#define SERIAL_PORT         0x3f8

static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo);
/* Restore old terminal i/o settings */
void resetTermios(void);
/* Read 1 character - echo defines echo mode */
char getch_(int echo);
/* Read 1 character without echo */
char getch(void);
/* Read 1 character with echo */
char getche(void);

#endif

