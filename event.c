/*
 * $Id: event.c 570 2006-05-04 17:50:36Z arnaldo $
 */

#include "priax.h"

static char         numeric_buffer[61];
static unsigned int numeric_buffer_pos = 0;
static int          transfer_pressed = 0;
static int          hold_pressed = 0;

/* main event handler */
static void EV_EventHandler(char *keyname);

/* set main event handler from outside this scope */
void PRIAX_Event()
{
    /* if there is something on the buffer, we blit it */
    if(numeric_buffer_pos)
        PRIAX_LCDText(LCD_Dial, numeric_buffer);

    /* set my handler! */
    PRIAX_KeypadSetEv(EV_EventHandler);
}

void zero_numeric_buffer(void)
{
    memset(numeric_buffer, 0, sizeof(numeric_buffer));
    numeric_buffer_pos = 0;
    PRIAX_LCDClear(LCD_Dial); /* now a complete clear! */
}

/*
 * Returns numeric_buffer_pos, which is a way
 * to see if it's greater than zero from the second
 * event handler (CR_EventHandler())
 * - we can't dial nowhere
 */
int numeric_buffer_size(void)
{
    return numeric_buffer_pos;
}

static void numeric_callback(char *key)
{
    IAX2ptr iax2 = IAX2Get();

    /* if the call is ACTIVE and FLASH is not pressed, 
     * digited numbers are sent as DTMF
     */
    if((iax2->selected() != -1))
        iax2->dtmf(*key);
    else {
        if(numeric_buffer_pos < sizeof(numeric_buffer)-1)
            numeric_buffer[numeric_buffer_pos++] = *key;
        else
            fprintf(stdout, "\a");

        PRIAX_LCDText(LCD_Dial, "%s", numeric_buffer);
    }
}

/* every keypad key is bound to numeric_callback() above */
static void ev_key_1    (){ numeric_callback("1"); }
static void ev_key_2    (){ numeric_callback("2"); }
static void ev_key_3    (){ numeric_callback("3"); }
static void ev_key_4    (){ numeric_callback("4"); }
static void ev_key_5    (){ numeric_callback("5"); }
static void ev_key_6    (){ numeric_callback("6"); }
static void ev_key_7    (){ numeric_callback("7"); }
static void ev_key_8    (){ numeric_callback("8"); }
static void ev_key_9    (){ numeric_callback("9"); }
static void ev_key_0    (){ numeric_callback("0"); }
static void ev_key_pound(){ numeric_callback("#"); }
static void ev_key_star (){ numeric_callback("*"); }

/* talk */
static void ev_key_talk()
{
    IAX2ptr iax2 = IAX2Get();

    /* if current call is ringing, log and answer */
    if (PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_INCOMING|PRIAX_STATE_WAITING)) {
        PRIAX_LCDClear(LCD_TextInfo);
        iax2->pickup();

    /* if current call is active and flash isn't pressed, user can't dial */
    } else
    if (PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE)) {
        debug(_("There's an active call, can't dial again.\n"));

#if 0
    /* transfer call */
    } else
    if (transfer_pressed) {

        iax2->transfer(numeric_buffer);
        PRIAX_LCDText(LCD_Register, _("Call transfered to %s."), numeric_buffer);
        zero_numeric_buffer();
        PRIAX_LCDClear(LCD_Dial);

        PRIAX_AnimControl(AnimStop);

        TOGGLE(transfer_pressed);
        PRIAX_DeactiveLed(TransferLed);

#endif
    /* just dial */
    } else
    if (numeric_buffer_pos) {
        iax2->dial(numeric_buffer);

    /* none of the previous conditions were met, go to callreg */
    } else
    if (!numeric_buffer_pos) {
        PRIAX_LCDClear(LCD_Dial);
        PRIAX_CallReg();
    }

    return;
}

/* hangup */
static void ev_key_hangup()
{
    IAX2ptr iax2 = IAX2Get();

    iax2->hangup(iax2->private.index);
    zero_numeric_buffer();
}

/* hold */
static void ev_key_hold()
{
    IAX2ptr iax2 = IAX2Get();

    if(!PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        return;

    zero_numeric_buffer();
    TOGGLE(hold_pressed);

    if (hold_pressed) {
        iax2->hold();
        PRIAX_LCDText(LCD_Register, _("Hold"));
        PRIAX_AnimSet(PauseAnim, iax2->private.current->number);
        PRIAX_AnimControl(AnimPlay);

        PRIAX_ActiveLed(HoldLed);
    } else {
        iax2->unhold();
        PRIAX_AnimSet(TalkAnim, iax2->private.current->number);
        PRIAX_AnimControl(AnimPlay);
        PRIAX_LCDText(LCD_Register, _("UnHold"));

        PRIAX_DeactiveLed(HoldLed);
    }
}

/* XXX what the hell is this ? XXX */
static void ev_key_pickup()
{
    IAX2ptr iax2 = IAX2Get();

    iax2->dial(iax2->conf->pickupexten);
}

/* transfer */
static void ev_key_transfer()
{
    IAX2ptr iax2 = IAX2Get();

    if(!PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        return;

    TOGGLE(transfer_pressed);
    if(transfer_pressed) {
        iax2->transfer();
        PRIAX_ActiveLed(TransferLed);
    } else
        PRIAX_DeactiveLed(TransferLed);
}

/* mute */
static void ev_key_mute()
{
    IAX2ptr iax2 = IAX2Get();

    if(!PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        return;

    iax2->mute();

    if(PRIAX_AnimGetState() == AnimPlay) {
        PRIAX_AnimSet(PauseAnim, iax2->private.current->number);
        PRIAX_AnimControl(AnimPlay);
    }
    PRIAX_ActiveLed(MuteLed);
}

/* unmute */
static void ev_key_unmute()
{
    IAX2ptr iax2 = IAX2Get();
    iax2->unmute();

    if(PRIAX_AnimGetState() == AnimPlay) {
        PRIAX_AnimSet(TalkAnim, iax2->private.current->number);
        PRIAX_AnimControl(AnimPlay);
    }
    PRIAX_DeactiveLed(MuteLed);
}

/* backspace */
static void ev_key_backspace()
{
    int talking = 0;
    char *nb = numeric_buffer;

    if(!talking) {
        if(numeric_buffer_pos) {
            nb[--numeric_buffer_pos] = '\0';
            if(*nb)
                PRIAX_LCDText(LCD_Dial, "%s", nb);
            else
                PRIAX_LCDClear(LCD_Dial);
        }
    }
}

/* select line 1 */
static void ev_key_up()
{ 
    IAX2Get()->prev_call();
    PRIAX_DeactiveLed(Line2Led);
    PRIAX_ActiveLed(Line1Led);
}

/* select line 2 */
static void ev_key_down()
{ 
    IAX2Get()->next_call();
    PRIAX_DeactiveLed(Line1Led);
    PRIAX_ActiveLed(Line2Led);
}

/* go to the address book */
static void ev_key_right()
{
    //PRIAX_AddrBook();
}

static void ev_key_inc_output()
{
    double level;
    level = iaxc_output_level_get();
    iaxc_output_level_set(level+0.1);
}

static void ev_key_dec_output()
{
    double level;
    level = iaxc_output_level_get();
    if(level-0.1 >= 0)
        iaxc_output_level_set(level-0.1);
}

static void ev_key_inc_input()
{
    double level;
    level = iaxc_output_level_get();
    iaxc_input_level_set(level+0.1);
}

static void ev_key_dec_input()
{
    double level;
    level = iaxc_output_level_get();
    if(level-0.1 >= 0)
        iaxc_input_level_set(level-0.1);
}

static void ev_key_boost()
{
    int boost = iaxc_mic_boost_get();

    if(!PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        return;

    if(boost <= 0) {
        if(iaxc_mic_boost_set(boost) != -1)
            PRIAX_ActiveLed(BoostLed);
    } else {
        if(iaxc_mic_boost_set(0) != -1)
            PRIAX_DeactiveLed(BoostLed);
    }
}

static void ev_key_sel_line1() 
{ 
    IAX2Get()->select_call(1-1);
    PRIAX_DeactiveLed(Line2Led);
    PRIAX_ActiveLed(Line1Led);
}
static void ev_key_sel_line2() 
{ 
    IAX2Get()->select_call(2-1);
    PRIAX_DeactiveLed(Line1Led);
    PRIAX_ActiveLed(Line2Led);
}
static void ev_key_history() { PRIAX_CallReg(); }

static void ev_key_logo()
{
    puts("ev_key_logo(): TODO");
}


/* generic event */
typedef struct {
    char *key;
    void (*callback)(void);
} PRIAX_EventDef;

/* event list and callback ptr */
PRIAX_EventDef PRIAX_EventList[] = {
    { "1",           ev_key_1          },
    { "2",           ev_key_2          },
    { "3",           ev_key_3          },
    { "4",           ev_key_4          },
    { "5",           ev_key_5          },
    { "6",           ev_key_6          },
    { "7",           ev_key_7          },
    { "8",           ev_key_8          },
    { "9",           ev_key_9          },
    { "0",           ev_key_0          },
    { "#",           ev_key_pound      },
    { "*",           ev_key_star       },
    { "talk",        ev_key_talk       },
    { "hangup",      ev_key_hangup     },
    { "mute",        ev_key_mute       },
    { "unmute",      ev_key_unmute     },
    { "backspace",   ev_key_backspace  },
    { "up",          ev_key_up         },
    { "down",        ev_key_down       },
    { "right",       ev_key_right      },
    { "inc-output",  ev_key_inc_output },
    { "dec-output",  ev_key_dec_output },
    { "inc-input",   ev_key_inc_input  },
    { "dec-input",   ev_key_dec_input  },
    { "boost",       ev_key_boost      },
    { "line1",       ev_key_sel_line1  },
    { "line2",       ev_key_sel_line2  },
    { "history",     ev_key_history    },
    { "logoFlizy",   ev_key_logo       },
    { "pickupgroup", ev_key_pickup     },
    { "transfer",    ev_key_transfer   },
    { "hold",        ev_key_hold       },
};

/* handle events */
static void EV_EventHandler(char *keyname)
{
    int i;
    void (*callback)(void);
    const static int size = sizeof(PRIAX_EventList)/sizeof(PRIAX_EventList[0]);
    int TEST_found = 0;
    static char text_buffer[128];
    static int  text_buffer_pos = -1;
    static int  next_key_upper = 0;

    if(text_buffer_pos == -1)
        memset(&text_buffer, 0, sizeof(text_buffer));

    if(!keyname) return;

    for(i = 0; i < size; i++) {
        int cs = strlen(keyname) > strlen(PRIAX_EventList[i].key) ? 
            strlen(keyname) : strlen(PRIAX_EventList[i].key);
        if(strncmp(keyname, PRIAX_EventList[i].key, cs) == 0) {
            if(strcmp(keyname, "talk") == 0 && text_buffer_pos) {
                iaxc_send_text(text_buffer);
                memset(&text_buffer, 0, sizeof(text_buffer));
                text_buffer_pos = 0;
                PRIAX_LCDClear(LCD_Register);
            }
            if((callback = PRIAX_EventList[i].callback) != NULL) {
                callback();
                TEST_found = 1;
            }
        }
    }

    if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE) && !isdigit(*keyname)) {
        do {
            if(strcmp(keyname, "left alt") == 0 || strcmp(keyname, "alt gr") == 0)
                break;
            if(strcmp(keyname, "backspace") == 0 && text_buffer_pos)
                text_buffer[--text_buffer_pos] = '\0';

            if(text_buffer_pos <= (sizeof(text_buffer)-1) && !TEST_found) {
                if(strcmp(keyname, "space") == 0) {
                    text_buffer[text_buffer_pos++] = ' ';
                } else
                if(strcmp(keyname, "shift") == 0 || strcmp(keyname, "right shift") == 0) {
                    next_key_upper = 1;
                } else {
                    if(next_key_upper)
                        text_buffer[text_buffer_pos++] = toupper(*keyname);
                    else
                        text_buffer[text_buffer_pos++] = *keyname;
                    next_key_upper = 0;
                }
            }
            if(!text_buffer_pos)
                PRIAX_LCDClear(LCD_Register);
            else
                PRIAX_LCDText(LCD_Register, text_buffer);
        } while (0);
    }

    return;
}

