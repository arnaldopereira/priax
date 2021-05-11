/*
 * $Id: myexit.h 106 2005-11-13 06:28:15Z alec $
 */

#ifndef _MYEXIT_H
#define _MYEXIT_H

/* I decided to do this because I'm not sure about mingw's implementation
 * of atexit() */

#define MAX_EXIT_CALLS 10

extern void my_atexit(void (*function)(void), char *debugstr);
extern void my_exit(int num, char *reason);

#endif /* myexit.h */
