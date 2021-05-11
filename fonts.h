/*
 * $Id: fonts.h 93 2005-11-12 06:11:35Z alec $
 */

#ifndef _FONTS_H
#define _FONTS_H

#if(!TTF_MAJOR_VERSION || TTF_MAJOR_VERSION < 2)
#error "You need libsdl-ttf-2.0 or newer!"
#endif

typedef enum {
    Arial_10,
    Arial_12,
    Arial_15,
    Digital_10,
    Digital_12,
    Digital_20,
    Monospace_11,
    Logo,
} PRIAX_FontIndex;

extern int       PRIAX_InitFonts();
extern TTF_Font *PRIAX_Font(PRIAX_FontIndex index);
extern void      PRIAX_PrintText(SDL_Surface *screen, 
                            SDL_Color fgcolor, SDL_Color bgcolor, int fillbg,
                            TTF_Font *font, int x, int y, const char *string);

#endif /* fonts.h */
