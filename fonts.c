/*
 * $Id: fonts.c 560 2006-04-27 18:54:48Z arnaldo $
 */

#include "priax.h"

/* font definition */
typedef struct {
    TTF_Font   *font;
    const char *path;
    const int ptsize;
    const int flags;
} PRIAX_FontDef;

/* font list */
PRIAX_FontDef PRIAX_FontList[] = {
    { NULL, "arial.ttf",     10, TTF_STYLE_NORMAL                  },
    { NULL, "arial.ttf",     12, TTF_STYLE_NORMAL                  },
    { NULL, "arial.ttf",     15, TTF_STYLE_NORMAL                  },
    { NULL, "digital.ttf",   10, TTF_STYLE_NORMAL                  },
    { NULL, "digital.ttf",   12, TTF_STYLE_NORMAL                  },
    { NULL, "digital.ttf",   20, TTF_STYLE_NORMAL | TTF_STYLE_BOLD },
    { NULL, "monospace.ttf", 11, TTF_STYLE_NORMAL                  },
    { NULL, "logo.ttf",      14, TTF_STYLE_NORMAL | TTF_STYLE_BOLD }
};

const static
int FontList_Size = sizeof(PRIAX_FontList)/sizeof(PRIAX_FontList[0]);

static void PRIAX_CloseFonts()
{
    int i;

    for(i = 0; i < FontList_Size; i++) {
        TTF_Font *font = PRIAX_FontList[i].font;
        if(font)
            TTF_CloseFont(font);
    }

    return;
}

int PRIAX_InitFonts()
{
    int i;
    TTF_Font *font;

    if(TTF_Init() < 0) {
        debug(_("Unable to init TTF: %s\n"), SDL_GetError());
        return -1;
    } else
        my_atexit(TTF_Quit, "TTF_Quit");

    for(i = 0; i < FontList_Size; i++) {
        const char *path  = PRIAX_FontList[i].path;
        const int  ptsize = PRIAX_FontList[i].ptsize; 
        
        font = TTF_OpenFont(PRIAX_Path_FONTS(path), ptsize);

        if(font == NULL) {
            fprintf(stderr, _("Couldn't load %d pt font from %s: %s\n"),
                ptsize, PRIAX_Path_FONTS(path), SDL_GetError());
            return -1;
        } else {
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
            PRIAX_FontList[i].font = font;
        }
    }

    my_atexit(PRIAX_CloseFonts, "PRIAX_CloseFonts");
    
    return 0;
}

TTF_Font *PRIAX_Font(PRIAX_FontIndex index)
{
    if(index > FontList_Size) index = 0;
    return PRIAX_FontList[index].font;
}

void PRIAX_PrintText(SDL_Surface *screen,
                     SDL_Color fgcolor, SDL_Color bgcolor, int fillbg,
                     TTF_Font *font, int x, int y, const char *string)
{
    SDL_Rect rect;
    SDL_Surface *text;
    
    text = fillbg ? TTF_RenderText_Shaded(font, string, fgcolor, bgcolor) :
                    TTF_RenderText_Blended(font, string, fgcolor);

    rect.x = x;
    rect.y = y;
    rect.w = text->w;
    rect.h = text->h;

    SDL_BlitSurface(text, NULL, screen, &rect);
    SDL_FreeSurface(text);

    SDL_UpdateRect(screen, rect.x, rect.y, rect.w, rect.h);

    return;
}
