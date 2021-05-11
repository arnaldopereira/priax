/*
 * $Id: keypad.h 420 2006-01-05 14:09:21Z arnaldo $
 */

#ifndef _KEYPAD_H
#define _KEYPAD_H

#define KEYPAD_W    332
#define KEYPAD_H    430
#define KEYPAD_MASK "phone-mask.bmp"

#define XLCD_X      35
#define XLCD_Y      22
#define KEYPAD_XLCD "display.bmp"

#define MUTE_LED_X  175
#define MUTE_LED_Y  225
#define MUTE_LED_W  7
#define MUTE_LED_H  7

#define BOOST_LED_X  175
#define BOOST_LED_Y  251
#define BOOST_LED_W  7
#define BOOST_LED_H  7

#define LINE1_LED_X  257
#define LINE1_LED_Y  225
#define LINE1_LED_W  7
#define LINE1_LED_H  7

#define LINE2_LED_X  257
#define LINE2_LED_Y  251
#define LINE2_LED_W  7
#define LINE2_LED_H  7

#define HOLD_LED_X  175
#define HOLD_LED_Y  277
#define HOLD_LED_W  7
#define HOLD_LED_H  7

#define TRANSFER_LED_X  175
#define TRANSFER_LED_Y  303
#define TRANSFER_LED_W  7
#define TRANSFER_LED_H  7

#define LED_ON_R 0
#define LED_ON_G 225
#define LED_ON_B 69

#define LED_OFF_R 0
#define LED_OFF_G 35
#define LED_OFF_B 11

typedef int PRIAX_Led;
enum {
    MuteLed= 0,
    BoostLed,
    Line1Led,
    Line2Led,
    HoldLed,
    TransferLed,
};

extern void  PRIAX_InitKeypad(SDL_Surface *screen, PRIAX_Conf *conf);
extern void *PRIAX_KeypadGetEv();
extern void  PRIAX_KeypadSetEv(void(*ptr)(char *keyname));
extern void  PRIAX_ActiveLed(PRIAX_Led led);
extern void  PRIAX_DeactiveLed(PRIAX_Led led);
extern SDL_Surface *PRIAX_MainScreen();
extern SDL_Surface *PRIAX_LCD(int *x, int *y);
extern void ShowMainWindow(void);
extern void HideMainWindow(void);

#endif /* keypad.h */
