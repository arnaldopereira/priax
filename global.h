/*
 * $Id: global.h 564 2006-05-02 15:00:00Z arnaldo $
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H

#ifdef WIN32
/* you can change PRIAX_DRIVE environment variable on WINDOWS */
#define PRIAX_DRIVE "C:"
#define PRIAX_BASE  "/PRIAX"
#define PRIAX_CONF  "priax.conf"
#define PRIAX_CALLS "priax-calls.dat"
#define PRIAX_BOOK  "priax-book.txt"
#define PRIAX_ICON  "priax-32x32.bmp"
#else
//#define PRIAX_BASE "/usr/local/share/priax"
#define PRIAX_BASE  "./"
#define PRIAX_CONF  ".priax"
#define PRIAX_CALLS ".priax-calls"
#define PRIAX_BOOK  ".priax-book"
#define PRIAX_ICON  "priax-72x72.bmp"
//#define PRIAX_INTL  "/usr/local/share/"
#define PRIAX_INTL  "./"
#endif

/* directories */
#define PRIAX_IMAGES "images"
#define PRIAX_SOUNDS "sounds"
#define PRIAX_FONTS  "fonts"
#define PRIAX_LOCALE "locale"

/* max number of digits for phone numbers */
#define MAXNUM_DIGITS 20

/* here we assume any number < MAXNUM_REAL is an asterisk local channel,
 * someone on the PBX.
 * any number >= MAXNUM_REAL is a real phone number, from PSTN */
#define MAXNUM_REAL 7

/* common functions */
extern inline int         is_opened(char *file);     /* checks if a file is opened and writable */
extern inline char       *fixstr(char *str);         /* remove \r or \n */
extern inline int         ckline(char *str);         /* check blank lines or comment */
extern inline void        debug(char *str, ...);     /* debug */

/* convert BITMAP filename into SDL_Surface */
extern inline SDL_Surface *bitmap(SDL_Surface *screen, int x, int y, 
        const char *filename);

#endif
