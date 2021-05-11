/*
 * $Id: myexit.c 106 2005-11-13 06:28:15Z alec $
 */

#include "priax.h"

static void *callbacks[MAX_EXIT_CALLS];
static char *xdebugstr[MAX_EXIT_CALLS];
static int   callbacks_pos = 0;

/* scheduler */
void my_atexit(void (*function)(void), char *debugstr)
{
    if(callbacks_pos >= MAX_EXIT_CALLS) return;
    else {
        callbacks[callbacks_pos] = function;
        xdebugstr[callbacks_pos] = debugstr;
        callbacks_pos++;
    }
}

/* my own exit */
void my_exit(int num, char *reason)
{
    int i;
    void (*function)(void) = NULL;
    
    for(i = MAX_EXIT_CALLS; i > -1; i--) {
        if(!(function = callbacks[i])) continue;
        else {
            char *str = xdebugstr[i];
            debug(_("exit (%s)\n"), str ? str : "unset");

            function();
        }
    }

    if(reason)
        fprintf(stdout, "PRIAX QUIT: %s\n", reason);

    exit(num);
}
