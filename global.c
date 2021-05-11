/*
 * $Id: global.c 560 2006-04-27 18:54:48Z arnaldo $
 */

#include "priax.h"

inline int is_opened(char *file)
{
    int fd;

    if((fd = open(file, O_RDWR)) < 0)
        return 1;

    close(fd);
    return 0;
}

inline char *fixstr(char *str)
{
    char *p;

    for(p = str; *p != '\0'; p++)
        if(*p == '\r' || *p == '\n') { *p = '\0'; break; }

    return str;
}

inline int ckline(char *str)
{
    return 
        (*str == '\r' || *str == '\n' || *str == '\t' || *str == '#') ? 1 : 0;
}

inline void debug(char *str, ...)
{
    va_list ap;

    if(!getenv("DEBUG")) return;

    va_start(ap, str);
    vfprintf(stderr, str, ap);
    va_end(ap);
}

inline SDL_Surface *bitmap(SDL_Surface *screen, int x, int y, 
        const char *filename)
{
    SDL_Rect dest;
    SDL_Surface *image;

    /* Load the BMP file into a surface */
    if((image = SDL_LoadBMP(filename)) == NULL) return NULL;

    /* Blit onto the screen surface.
       The surfaces should not be locked at this point.
     */
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);

    /* Update the changed portion of the screen */
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

    return image;
}
