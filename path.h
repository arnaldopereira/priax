/*
 * $Id: path.h 47 2005-10-25 06:00:35Z alec $
 */

#ifndef _PATH_H
#define _PATH_H

#define MAXPATH_LEN 1024

extern char *PRIAX_Path_IMAGES(const char *name);
extern char *PRIAX_Path_SOUNDS(const char *name);
extern char *PRIAX_Path_FONTS (const char *name);
extern char *PRIAX_Path_CALLS (const char *name);
extern char *PRIAX_Path_CONF  (const char *name);
extern char *PRIAX_Path_BOOK  (const char *name);
extern char *PRIAX_Path_INTL  (const char *name);

#endif /* path.h */
