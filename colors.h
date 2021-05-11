/*
 * $Id: colors.h 93 2005-11-12 06:11:35Z alec $
 */

#ifndef _COLORS_H
#define _COLORS_H

typedef enum {
    Black,
    White,
    Gray,
    SolidRed,
    SolidGreen,
    SolidBlue,
    AppRed,
    AppGreen,
    AppBlue,
    AppPhone,
    AppDisplay,
    AppText1,
    BLANK,
} PRIAX_ColorIndex;

extern SDL_Color PRIAX_Color(PRIAX_ColorIndex index);

#endif /* colors.h */
