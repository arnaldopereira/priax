/*
 * $Id: path.c 47 2005-10-25 06:00:35Z alec $
 */

#include "priax.h"

/* here in Brasil we call it "gambiarra" - but it works! */
static int getpath_conf = 0;

/* generic path constructor (1) */
static char *DirPath1(const char *prefix, const char *name)
{

#ifdef WIN32
    char *drv = getenv("PRIAX_DRIVE");
#endif
  
    static char path[MAXPATH_LEN];
    memset(path, 0, sizeof(path));

#ifdef WIN32
    if(!drv) drv = PRIAX_DRIVE;

    if(getpath_conf && !prefix) {
        TOGGLE(getpath_conf);
        snprintf(path, sizeof(path)-1, "%s%s/%s",
                drv, PRIAX_BASE, name);
    } else
        snprintf(path, sizeof(path)-1, "%s%s/%s/%s", 
                drv, PRIAX_BASE, prefix, name);
#else

    if(getpath_conf) {
        TOGGLE(getpath_conf);
        snprintf(path, sizeof(path)-1, "%s/%s", prefix, name);
    } else
        snprintf(path, sizeof(path)-1, "%s/%s/%s", PRIAX_BASE, prefix, name);
#endif

    return path;
}

/* generic path constructor (2) */
static char *DirPath2(const char *name)
{
#ifdef WIN32
    TOGGLE(getpath_conf);
    return DirPath1(NULL, name);
#else
    struct passwd *pw = getpwuid(getuid());

    TOGGLE(getpath_conf);
    return !pw ? "/" : DirPath1(pw->pw_dir, name);
#endif
}

/* return filename under the PRIAX_IMAGES path */
char *PRIAX_Path_IMAGES(const char *name) 
{ return DirPath1(PRIAX_IMAGES, name); }

/* return filename under the PRIAX_SOUNDS path */
char *PRIAX_Path_SOUNDS(const char *name) 
{ return DirPath1(PRIAX_SOUNDS, name); }

/* return filename under the PRIAX_FONTS path */
char *PRIAX_Path_FONTS(const char *name) 
{ return DirPath1(PRIAX_FONTS, name); }

/* return filename under the correct CALLS path */
char *PRIAX_Path_CALLS(const char *name)
{ return DirPath2(name); }

/* return filename under the correct CONF path */
char *PRIAX_Path_CONF(const char *name)
{ return DirPath2(name); }

/* return filename under the correct BOOK path */
char *PRIAX_Path_BOOK(const char *name)
{ return DirPath2(name); }

/* return path to locale */
char *PRIAX_Path_INTL(const char *name)
{
    TOGGLE(getpath_conf);
#ifdef WIN32
    return DirPath1(NULL, name);
#else
    return DirPath1(PRIAX_INTL, name);
#endif
}
