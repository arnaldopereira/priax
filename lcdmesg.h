/*
 * $Id: lcdmesg.h 93 2005-11-12 06:11:35Z alec $
 */

#ifndef _LCDMESG_H
#define _LCDMESG_H

typedef enum {
    LCD_Register = 0,
    LCD_Dial,
    LCD_Clock,
    LCD_VoiceMail,
    LCD_TextInfo,
    LCD_TextLine1,
    LCD_TextLine2,
    LCD_TextLine3,
    LCD_TextLine4,
    LCD_TextLine5,
    LCD_TextLine6,
    LCD_AnimLine1,
    LCD_AnimLine2,
} PRIAX_DisplayIndex;

extern void PRIAX_LCDText  (PRIAX_DisplayIndex profile, char *str, ...);
extern void PRIAX_LCDClear (PRIAX_DisplayIndex profile);
extern void PRIAX_LCDInfo  (PRIAX_DisplayIndex profile, 
                            SDL_Rect *rect, TTF_Font **font);

#endif /* lcdmesg.h */
