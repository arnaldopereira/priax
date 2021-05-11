/*
 * $Id: lcdmesg.c 384 2005-12-16 19:22:35Z arnaldo $
 */

#include "priax.h"

#define MAX_LINES 10  /* maximum number of allowed lines */
#define MAX_CHARS 200 /* maximum number of allowed chars per line */

typedef struct {
    int ml;         /* max lines */
    int mc;         /* max chars per line (0 == don't care) */
    int x;          /* x position */
    int y;          /* y position */
    int w;          /* width of the printable rectangle for each line */
    int h;          /* height of the printable rectangle for each line */
    SDL_Color fg;   /* foreground */
    TTF_Font *fn;   /* font */
} LCDProfile;

/* profile definition */
static LCDProfile *LCD_GetProfile(int profile)
{
    static int initialized = 0;

    /* below is the initialization of coordinates only... */
    static LCDProfile pr[] = {
        { 1, 50,  43,  30, 233, }, /* LCD_Register */
        { 3, 20,  43,  55, 233, }, /* LCD_Dial */
        { 1, 20, 213, 153,  78, }, /* LCD_Clock */
        { 1, 30,  43, 145, 143, }, /* LCD_VoiceMail */
        { 1, 50,  43,  45, 253, }, /* LCD_TextInfo */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine1 */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine2 */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine3 */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine4 */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine5 */
        { 1, 50,  43,   0, 253, }, /* LCD_TextLine6 */
        { 1, 20,  63,  95, 253, }, /* LCD_AnimLine1 */
        { 1, 32,  53, 115, 253, }, /* LCD_AnimLine2 */
    };

    /* now: colors, font and height - things that cannot be
     * initialized because they aren't constant */
    if(!initialized) {

    /* things to initialize display for callreg and addressbook */
    int i, offset, display[] =
    { LCD_TextLine1, LCD_TextLine2, LCD_TextLine3,
      LCD_TextLine4, LCD_TextLine5, LCD_TextLine6 };
    int size = sizeof(display)/sizeof(display[0]);

    TOGGLE(initialized);

    /* LCD_Register */
    pr[LCD_Register].fg = PRIAX_Color(Gray);
    pr[LCD_Register].fn = PRIAX_Font(Arial_10);
    pr[LCD_Register].h  = TTF_FontHeight(pr[LCD_Register].fn);

    /* LCD_Dial */
    pr[LCD_Dial].fg = PRIAX_Color(White);
    pr[LCD_Dial].fn = PRIAX_Font(Digital_12);
    pr[LCD_Dial].h  = TTF_FontHeight(pr[LCD_Dial].fn);

    /* LCD_Clock */
    pr[LCD_Clock].fg = PRIAX_Color(Gray);
    pr[LCD_Clock].fn = PRIAX_Font(Digital_10);
    pr[LCD_Clock].h  = TTF_FontHeight(pr[LCD_Clock].fn);

    /* LCD_VoiceMail */
    pr[LCD_VoiceMail].fg = PRIAX_Color(Gray);
    pr[LCD_VoiceMail].fn = PRIAX_Font(Arial_10);
    pr[LCD_VoiceMail].h  = TTF_FontHeight(pr[LCD_VoiceMail].fn);

    /* LCD_AnimLine1 */
    pr[LCD_AnimLine1].fg = PRIAX_Color(AppText1);
    pr[LCD_AnimLine1].fn = PRIAX_Font(Digital_20);
    pr[LCD_AnimLine1].h  = TTF_FontHeight(pr[LCD_AnimLine1].fn);

    /* LCD_AnimLine2 */
    pr[LCD_AnimLine2].fg = PRIAX_Color(White);
    pr[LCD_AnimLine2].fn = PRIAX_Font(Monospace_11);
    pr[LCD_AnimLine2].h  = TTF_FontHeight(pr[LCD_AnimLine2].fn);

    /* LCD_TextInfo */
    pr[LCD_TextInfo].fg = PRIAX_Color(AppText1);
    pr[LCD_TextInfo].fn = PRIAX_Font(Arial_12);
    pr[LCD_TextInfo].h  = TTF_FontHeight(pr[LCD_TextInfo].fn);
    offset = 5 + pr[LCD_TextInfo].y + TTF_FontLineSkip(pr[LCD_TextInfo].fn);

    /* LCD_TextLine[1-6] */
    for(i = 0; i < size; i++) {
        /* only the first line may be white */
        pr[display[i]].fg = PRIAX_Color(!i ? White : Black);
        pr[display[i]].fn = PRIAX_Font(Arial_10);
        pr[display[i]].h  = TTF_FontHeight(pr[display[i]].fn);
        pr[display[i]].y  = offset;

        offset += TTF_FontLineSkip(pr[display[i]].fn);
    }

    } /* initialized */

    return &pr[profile];
}

/* clear a rectangle on the LCD display based on a profile */
static void LCDClearRect(SDL_Surface *screen, LCDProfile *pr)
{
    int x, y;
    SDL_Surface *lcd = PRIAX_LCD(&x, &y);
    SDL_Rect fgrect = { pr->x - x, pr->y - y, pr->w, pr->h * pr->ml };
    SDL_Rect bgrect = { pr->x, pr->y, pr->w, pr->h };

    //SDL_Color bc = pr->bg;
    //Uint32 color = SDL_MapRGB(screen->format, bc.r, bc.g, bc.b);

    SDL_BlitSurface(lcd, &fgrect, screen, &bgrect);
    SDL_UpdateRect(screen, bgrect.x, bgrect.y, bgrect.w, bgrect.h);

    return;
}

/* clear the LCD display according to the profile */
void PRIAX_LCDClear(PRIAX_DisplayIndex profile)
{
    LCDClearRect(PRIAX_MainScreen(), LCD_GetProfile(profile));
}

/* return profile's rectangle and font */
void PRIAX_LCDInfo(PRIAX_DisplayIndex profile, SDL_Rect *dest, TTF_Font **font)
{
    LCDProfile *pr = LCD_GetProfile(profile);

    if(dest != NULL) {
        (*dest).x = pr->x;
        (*dest).y = pr->y;
        (*dest).w = pr->w;
        (*dest).h = pr->h;
    }

    if(font != NULL) *font = pr->fn;
}

/* local PrintText - used only when printing via LCD profiles */
static void LCDPrintText(SDL_Surface *screen, SDL_Color fg,
        TTF_Font *fn, int x, int y, char *str, int len)
{
    char temp[MAX_CHARS];

    memset(temp, 0, sizeof(temp));
    strncpy(temp, str, len);
    
    PRIAX_PrintText(screen, fg, PRIAX_Color(BLANK), 0, fn, x, y, temp);
}

/* print text on the LCD display */
void PRIAX_LCDText(PRIAX_DisplayIndex profile, char *str, ...)
{
    va_list ap;
    int i, lines;
    char temp[MAX_CHARS], *p;
    SDL_Surface *screen = PRIAX_MainScreen();
    LCDProfile  *pr = LCD_GetProfile(profile);
    LCDProfile   pa = *pr;

    memset(temp, 0, sizeof(temp));
    va_start(ap, str);
    vsnprintf(temp, sizeof(temp), str, ap);
    va_end(ap);

    if(pa.mc) {
        /* if there is only one line avaliable and text is bigger
         * than the maximum number of characters we fix it with \0. */
        if(strlen(temp) > pa.mc && pa.ml == 1)
            temp[pa.mc] = '\0';
        
        /* count the number of lines */
        for(i = 0, lines = 0, p = temp; *p != '\0' && lines != pa.ml; i++, p++)
            if(i == pa.mc * lines) lines++;

        /* print each line on the correct Y position */
        for(i = 0; i < lines; i++) {
            if(i) {
                pa.y += TTF_FontLineSkip(pa.fn);
                pa.h /= 2;
            }
            p = temp + pa.mc * i;

            LCDClearRect(screen, &pa);
            LCDPrintText(screen, pa.fg, pa.fn, pa.x, pa.y, p, pa.mc);
        }
    } else {
        LCDClearRect(screen, &pa);
        PRIAX_PrintText(screen, pa.fg, PRIAX_Color(BLANK), 0,
                pa.fn, pa.x, pa.y, temp);
    }

    return;
}
