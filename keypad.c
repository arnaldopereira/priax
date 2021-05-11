/*
 * $Id: keypad.c 570 2006-05-04 17:50:36Z arnaldo $
 */

#include "priax.h"

/* main surface and LCD */
static SDL_Surface *main_screen = NULL;
static SDL_Surface *main_xlcd   = NULL;

/* user's conf */
static PRIAX_Conf  *main_conf   = NULL;

/* keypad's cleanup */
static void  KeypadQuit();

/* main loop */
static void  KeypadLoop();

/* where do I send keyboard events? */
static void  (*KeypadEvPtr)(char *keyname);

/* handle keypad before sending events to the event handler */
static char *KeypadHandle(char *keyname, int modifier);

/* toggle mute button's label and return its state */
static int   KeypadMute();

/* return clicked button's name */
static char *KeypadGetClick(int x, int y);

/* tray icon */
#ifdef X11_TRAY_ICON
typedef struct {
    TrayContext   ctx;
    SDL_SysWMinfo wm;
    int           state; /* 0 hidden, 1 visible */
} TrayProp;
static TrayProp TRAY;
static void CreateTray();
static void UpdateTray();
static void FinishTray();

#elif defined(WIN32)
static TrayContext Tray;
static void CreateTray();
static void UpdateTray();
static void FinishTray();
#endif

/* generic button information */
typedef struct {
    char *name;
    int x;
    int w;
    int y;
    int h;
} PRIAX_KeypadButton;

/* application's keypad button map */
const static PRIAX_KeypadButton PRIAX_Buttons[] = {
    { "1",           48,   79, 192, 213  },
    { "2",           84,  115, 192, 213  },
    { "3",           120, 151, 192, 213  },
    { "4",           48,   79, 218, 239  },
    { "5",           84,  115, 218, 239  },
    { "6",           120, 151, 218, 239  },
    { "7",           48,   79, 244, 265  },
    { "8",           84,  115, 244, 265  },
    { "9",           120, 151, 244, 265  },
    { "*",           48,   79, 270, 291  },
    { "0",           84,  115, 270, 291  },
    { "#",           120, 151, 270, 291  },
    { "pickupgroup", 48,  151, 296, 317  },
    { "inc-output",  84,  115, 346, 367  },
    { "dec-output",  120, 151, 346, 367  },
    { "inc-input",   84,  115, 372, 393  },
    { "dec-input",   120, 151, 372, 393  },
    { "talk",        168, 224, 192, 213  },
    { "hangup",      229, 285, 192, 213  },
    { "mute",        168, 245, 218, 239  },
    { "line1",       250, 285, 218, 239  },
    { "boost",       168, 245, 244, 265  },
    { "line2",       250, 285, 244, 265  },
    { "transfer",    168, 245, 296, 317  },
    { "hold",        168, 245, 270, 291  },
    { "book",        168, 245, 346, 367  },
    { "history",     168, 245, 372, 393  },
    { "logoFlizy",   260, 300, 346, 386  },
};

/* return SDL's main screen */
SDL_Surface *PRIAX_MainScreen() { return main_screen; }

/* return LCD's surface and properties */
SDL_Surface *PRIAX_LCD(int *x, int *y)
{
    if(x != NULL) *x = XLCD_X;
    if(y != NULL) *y = XLCD_Y;

    return main_xlcd;
}

static void get_led_coordinates(PRIAX_Led led, SDL_Rect *rect)
{
    switch(led) {
        case MuteLed:
            rect->x = MUTE_LED_X;
            rect->y = MUTE_LED_Y;
            rect->w = MUTE_LED_W;
            rect->h = MUTE_LED_H;
            break;
        case BoostLed:
            rect->x = BOOST_LED_X;
            rect->y = BOOST_LED_Y;
            rect->w = BOOST_LED_W;
            rect->h = BOOST_LED_H;
            break;
        case Line1Led:
            rect->x = LINE1_LED_X;
            rect->y = LINE1_LED_Y;
            rect->w = LINE1_LED_W;
            rect->h = LINE1_LED_H;
            break;
        case Line2Led:
            rect->x = LINE2_LED_X;
            rect->y = LINE2_LED_Y;
            rect->w = LINE2_LED_W;
            rect->h = LINE2_LED_H;
            break;
        case HoldLed:
            rect->x = HOLD_LED_X;
            rect->y = HOLD_LED_Y;
            rect->w = HOLD_LED_W;
            rect->h = HOLD_LED_H;
            break;
        case TransferLed:
            rect->x = TRANSFER_LED_X;
            rect->y = TRANSFER_LED_Y;
            rect->w = TRANSFER_LED_W;
            rect->h = TRANSFER_LED_H;
            break;
    }
}

void PRIAX_ActiveLed(PRIAX_Led led)
{
    SDL_Rect rect;
    SDL_Color bc;
    Uint32 color;

    bc.r = LED_ON_R;
    bc.g = LED_ON_G;
    bc.b = LED_ON_B;
    color = SDL_MapRGB(main_screen->format, bc.r, bc.g, bc.b);

    get_led_coordinates(led, &rect);
    SDL_FillRect(main_screen, &rect, color);
    SDL_UpdateRect(main_screen, rect.x, rect.y, rect.w, rect.h);
}

void PRIAX_DeactiveLed(PRIAX_Led led)
{
    SDL_Rect rect;
    SDL_Color bc;
    Uint32 color;

    bc.r = LED_OFF_R;
    bc.g = LED_OFF_G;
    bc.b = LED_OFF_B;
    color = SDL_MapRGB(main_screen->format, bc.r, bc.g, bc.b);

    get_led_coordinates(led, &rect);
    SDL_FillRect(main_screen, &rect, color);
    SDL_UpdateRect(main_screen, rect.x, rect.y, rect.w, rect.h);
}

/* this is ShowBMP + main loop */
void PRIAX_InitKeypad(SDL_Surface *screen, PRIAX_Conf *conf)
{
    char *file, buffer[256];
    SDL_Surface *image;

    /* save conf and screen pointers */
    main_conf   = conf;
    main_screen = screen;

    /* draw priax's interface */
    file = PRIAX_Path_IMAGES(KEYPAD_MASK);
    if((image = bitmap(screen, 0, 0, file)) == NULL) {
        snprintf(buffer, sizeof(buffer), _("Unable to load main bitmap image %s: %s\n"), file, SDL_GetError());
        debug(buffer);
        my_exit(1, buffer);
    } else {
        SDL_Rect clip = { 0, 0, image->w, image->h };
        SDL_FreeSurface(image);
        SDL_SetClipRect(screen, &clip);
    }

    /* draw priax's LCD */
    file = PRIAX_Path_IMAGES(KEYPAD_XLCD);
    if((image = bitmap(screen, XLCD_X, XLCD_Y, file)) == NULL) {
        snprintf(buffer, sizeof(buffer), _("Unable to load LCD bitmap image %s: %s\n"), file, SDL_GetError());
        debug(buffer);
        my_exit(1, buffer);
    } else {
        my_atexit(KeypadQuit, "KeypadQuit");
        main_xlcd = image;
    }

    /* register IAX2 */
    {
        IAX2ptr iax2 = IAX2Get();

        /* save config pointer */
        iax2->conf = conf;

        if(!IAX2READY(iax2)) 
            my_exit(1, _("Unable to initialize IAX2."));
        else {
            iax2->setregistry(conf->user, conf->pass, conf->host);
            iax2->setcallerid(conf->name, conf->number);
            iax2->setdevices(conf->snd_dev_input, conf->snd_dev_output, conf->snd_dev_ring);
            //iax2->setdevices("hw:2,0", "hw:2,0", "hw:2,0");
        }
    }

    /* start clock thread */
    PRIAX_ClockThread();

    /* set default event handler */
    PRIAX_Event();

    /* call event loop */
    KeypadLoop(screen);
}

/* get and set keypad's event handler */
void *PRIAX_KeypadGetEv() { return KeypadEvPtr; }
void  PRIAX_KeypadSetEv(void(*ptr)(char *keyname)) { KeypadEvPtr = ptr; }

static void KeypadQuit()
{
    if(main_xlcd) {
        SDL_FreeSurface(main_xlcd);
        main_xlcd = NULL;
    }

    if(main_screen) {
        SDL_FreeSurface(main_screen);
        main_screen = NULL;
    }
}

void ShowMainWindow(void)
{
    SDL_SysWMinfo wm;

    SDL_VERSION(&wm.version);
    if(!SDL_GetWMInfo(&wm)) {
        return;
    }

#ifndef WIN32
    wm.info.x11.lock_func();
    if(wm.subsystem == SDL_SYSWM_X11) {
        XMapWindow(wm.info.x11.display, wm.info.x11.wmwindow);
        XSync(wm.info.x11.display, False);
    }
#ifdef X11_TRAY_ICON
    TrayShow(&TRAY.ctx);
    TRAY.state = 1;
#endif
    wm.info.x11.unlock_func();
#endif
}

void HideMainWindow(void)
{
#ifdef X11_TRAY_ICON
    TRAY.wm.info.x11.lock_func();
    TrayHide(&TRAY.ctx);
    TRAY.state = 0;
    TRAY.wm.info.x11.unlock_func();
#else
    SDL_WM_IconifyWindow();
#endif
}

static void KeypadLoop()
{
    SDL_Event ev;

#if defined(X11_TRAY_ICON) || defined(WIN32)
    /* create tray icon */
    CreateTray();
    UpdateTray();

    /* enable wm events... */
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

    for(;;) {
        SDL_WaitEvent(&ev);

        switch(ev.type) {
            case SDL_KEYDOWN:
                KeypadEvPtr(KeypadHandle
                    (SDL_GetKeyName(ev.key.keysym.sym), ev.key.keysym.mod));

                break;
            case SDL_MOUSEBUTTONDOWN:
#ifdef X11_TRAY_ICON
                {
                    Uint8 st = SDL_GetAppState();

                    /* SDL will receive a click event on the tray's window
                     * only when the focus isn't on the application */
                    if(!(st & SDL_APPMOUSEFOCUS)) {
                        TRAY.wm.info.x11.lock_func();

                        if(TRAY.state)
                            TrayHide(&TRAY.ctx);
                        else
                            TrayShow(&TRAY.ctx);

                        TOGGLE(TRAY.state);
                        TRAY.wm.info.x11.unlock_func();
                        break;
                    }
                }
#endif
                KeypadEvPtr(KeypadHandle
                    (KeypadGetClick(ev.button.x, ev.button.y), 0));
                break;
            case SDL_QUIT:
                HideMainWindow();
                break;
#ifdef X11_TRAY_ICON
                /* this will show tray icon even when changing
                 * virtual desktops */
            case SDL_SYSWMEVENT:
                switch(ev.syswm.msg->event.xevent.type) {
                    case VisibilityNotify:
                    case PropertyNotify:
                        UpdateTray();
                }
                break;
#endif
        }
    }
}

/* handler */
static char *KeypadHandle(char *keyname, int modifier)
{
    if(!keyname)
        return NULL;

    if(modifier & KMOD_CTRL && tolower(*keyname) == 'q') {
        my_exit(EXIT_SUCCESS, NULL);
    }
	/* we understand keypad digits too */
    else if(*keyname == '[' && strlen(keyname) == 3) {
		static char foo[2];
		
		foo[0] = keyname[1];
		foo[1] = 0;
		keyname = foo;

	} else if(modifier & KMOD_SHIFT && *keyname == '8')
        keyname = "*";
    else if(modifier & KMOD_SHIFT && *keyname == '3')
        keyname = "#";
    else if(!strcmp(keyname, "return") || !strcmp(keyname, "enter"))
        keyname = "talk";
    else if(!strcmp(keyname, "escape"))
        keyname = "hangup";
    else if(!strcmp(keyname, "mute")) {
        if(!KeypadMute()) keyname = "unmute";
    }
    else if(!strcmp(keyname, "book")) {
        //PRIAX_AddrBook();
    }
    /* turn *keyname into upper case */
    else if(keyname[1] == '\0' && islower(*keyname)) {
        if(modifier & KMOD_SHIFT) {
            static char tmp[2];

            tmp[0] = toupper(*keyname);
            tmp[1] = '\0';
            keyname = tmp;
        }
    } 

    return keyname;
}

/* toggle mute button or just return its state */
static int KeypadMute()
{
    static int mute = 0;
    TOGGLE(mute);
    return mute;
}

/* return button name according to the mouse position */
static char *KeypadGetClick(int x, int y)
{
    int i;
    SDL_Rect r;
    PRIAX_KeypadButton *p = NULL;
    const static int size = sizeof(PRIAX_Buttons)/sizeof(PRIAX_Buttons[0]);

    /*
     fprintf(stderr, "click pos: %dx%d\n", x, y);
     */

    for(i = 0; i < size; i++) {
        p = (PRIAX_KeypadButton *) &PRIAX_Buttons[i];

        if(x >= p->x && x <= p->w && y >= p->y && y <= p->h)
            return p->name;
    }

    /* check if voicemail is active */
    if(PRIAX_VoiceMail(&r)) {
        /* if it is, and click match voicemail's text area, call mailbox */
        if(x >= r.x && x <= r.w && y >= r.y && y <= r.h)
            if(main_conf) {
				if(!PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE)) {
					IAX2ptr iax2 = IAX2Get();
					iax2->dial(main_conf->mailbox);
				}
            }
    }

    return NULL;
}

#ifdef X11_TRAY_ICON
static void CreateTray()
{
    SDL_VERSION(&TRAY.wm.version);
    if(!SDL_GetWMInfo(&TRAY.wm)) return;

    /* create tray icon */
    TRAY.wm.info.x11.lock_func();
    TrayInit(&TRAY.ctx, TRAY.wm.info.x11.display, TRAY.wm.info.x11.wmwindow);
    TRAY.wm.info.x11.unlock_func();

    /* set default icon state to 'active' */
    TRAY.state = 1;

    my_atexit(FinishTray, "FinishTray");
}

static void UpdateTray()
{
    TRAY.wm.info.x11.lock_func();
    TrayRealize(&TRAY.ctx);
    TRAY.wm.info.x11.unlock_func();
}

static void FinishTray()
{
    TRAY.wm.info.x11.lock_func();
    TrayClose(&TRAY.ctx);
    TRAY.wm.info.x11.unlock_func();
}

#elif defined(WIN32)

static void CreateTray()
{
    /* create tray icon */
    TrayInit(&Tray);
    my_atexit(FinishTray, "FinishTray");
}

static void UpdateTray()
{
}

static void FinishTray()
{
    TrayClose(&Tray);
}
#endif
