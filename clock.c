/*
 * $Id: clock.c 123 2005-11-16 23:34:39Z arnaldo $
 */

#include "priax.h"

static int print_voicemail = 0, tick = 0;
char *voicemail_msg = NULL;

/* clock loop */
int CT_loop(void *unused)
{
    for(;;) {
        /* this function is been called each 1/2 second and to
         * reduce flicker it is also called on each keyboard or
         * mouse event - but tick is only changed inside this
         * thread to keep ':' blinking 2 times each second
         */
        PRIAX_PrintClock();
        PRIAX_AnimUpdate();
        TOGGLE(tick);
        SDL_Delay(500);
    }

    return 0;
}

/* this is the clock thread creator */
void PRIAX_ClockThread()
{
    SDL_Thread *clock;

    voicemail_msg = _("You have voicemail");

    if((clock = SDL_CreateThread(CT_loop, NULL)) == NULL)
        debug(_("Unable to create ClockThread: %s\n"), SDL_GetError());
}

/* print clock - force */
void PRIAX_PrintClock()
{
    time_t t;
    struct tm *tm;
    char c, str[9];

    time(&t);
    tm = localtime(&t);

    c = tick ? ':' : ' ';

    snprintf(str, sizeof(str), "%02d%c%02d%c%02d",
            tm->tm_hour, c, tm->tm_min, c, tm->tm_sec);
    
    if(print_voicemail && tick)
        PRIAX_LCDText(LCD_VoiceMail, voicemail_msg);
    else
    if(print_voicemail && !tick)
        PRIAX_LCDClear(LCD_VoiceMail);

    PRIAX_LCDText(LCD_Clock, str);
}

void PRIAX_VoiceMailON()  { print_voicemail = 1; }
void PRIAX_VoiceMailOFF() { print_voicemail = 0; }

/* return voicemail's state and fills message rectangle */
int  PRIAX_VoiceMail(SDL_Rect *rect)
{
    int w, h;
    SDL_Rect r;
    TTF_Font *font;

    if(!rect || !print_voicemail) return 0;
    
    PRIAX_LCDInfo(LCD_VoiceMail, &r, &font);
    TTF_SizeText(font, voicemail_msg, &w, &h);

    (*rect).x = r.x;       /* x is the real x */
    (*rect).y = r.y - h;   /* y is the offset to blit text, so we adjust */
    (*rect).w = r.x + w;   /* w is text->w, so we add x to it */
    (*rect).h = r.y + h*2; /* h is text->h, so we add y plus h*2 */

    return 1;
}
