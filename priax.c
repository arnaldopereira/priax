/*
 * $Id: priax.c 570 2006-05-04 17:50:36Z arnaldo $
 */

#include "priax.h"

#ifdef WIN32
int SDL_main(int argc, char **argv)
#else
int main()
#endif
{
    int flags = SDL_SWSURFACE | SDL_DOUBLEBUF;

    SDL_Surface *screen, *icon;
    PRIAX_Conf *conf = PRIAX_GetConf(PRIAX_Path_CONF(PRIAX_CONF));

#ifdef WIN32
    /* set language (win32) */
    char temp[128];

    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "LANGUAGE=%s", conf->lang);
    gettext_putenv(temp);
#else
    /* set language (linux) */
    setlocale(LC_ALL, "");
    setlocale(LC_MESSAGES, conf->lang);
#endif

    bindtextdomain("priax", PRIAX_Path_INTL(PRIAX_LOCALE));
    textdomain("priax");

    /* initialize SDL */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        debug(_("Unable to init SDL: %s\n"), SDL_GetError());
        my_exit(1, _("Unable to initialize SDL."));
    } else 
        my_atexit(SDL_Quit, "SDL_Quit");

    /* initialize fonts */
    if(PRIAX_InitFonts() < 0) 
        my_exit(1, _("Unable to initialize TTF."));

    /* initialize addrbook */
    //PRIAX_InitAddrBook();

    /* set window title */
    SDL_WM_SetCaption(APP_NAME, NULL);

    /* set application icon */
    if((icon = SDL_LoadBMP(PRIAX_Path_IMAGES(PRIAX_ICON))) != NULL)
        SDL_WM_SetIcon(icon, NULL);

    /* create SDL screen */
    if((screen = 
            SDL_SetVideoMode(KEYPAD_W, KEYPAD_H, 16, flags)) == NULL) {
        debug(_("Unable to get SDL surface: %s\n"), SDL_GetError());
        my_exit(1, _("Unable to create SDL surface"));
    }

    /* dump all call records before quit - callreg.c */
    my_atexit(PRIAX_DumpCalls, "PRIAX_DumpCalls");

    /* initialize the keyboard - this is the main loop */
    PRIAX_InitKeypad(screen, conf);

    return 0;
}
