/*
 * $Id: priax.h 560 2006-04-27 18:54:48Z arnaldo $
 */

#ifndef _PRIAX_H
#define _PRIAX_H

/* widget */
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>
#include <SDL_thread.h>

/* system */
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <libintl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef WIN32
#include <pwd.h>
#endif

/* only defined when SDL is using X11 */
#if 0
#ifdef X_H
#define X11_TRAY_ICON
#endif
#endif

/* project */
#include "conf.h"
#include "iax2.h"
#include "path.h"
#include "tray.h"
#include "anim.h"
#include "fonts.h"
#include "event.h"
#include "clock.h"
#include "colors.h"
#include "global.h"
#include "myexit.h"
#include "keypad.h"
#include "lcdmesg.h"
#include "callreg.h"
#include "addrbook.h"

/* application name and version - release.revision (rel. 0.8) */
#define APP_NAME "Flizy v0.8.567"

/* internationalization */
#define _(x) gettext(x)

/* default toggle macro */
#ifndef TOGGLE
#define TOGGLE(x) x=!x
#endif

#endif /* priax.h */
