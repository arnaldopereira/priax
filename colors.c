/*
 * $Id: colors.c 93 2005-11-12 06:11:35Z alec $
 */

#include "priax.h"

/* color definition */
typedef struct {
    SDL_Color color;
} PRIAX_ColorDef;

/* color list */
PRIAX_ColorDef PRIAX_ColorList[] = {
    { { 0x00, 0x00, 0x00, 0 } }, /* Black */
    { { 0xFF, 0xFF, 0xFF, 0 } }, /* White */
    { { 0xBF, 0xBF, 0xBF, 0 } }, /* Gray */
    { { 0xFF, 0x00, 0x00, 0 } }, /* SolidRed */
    { { 0x00, 0xFF, 0x00, 0 } }, /* SolidGreen */
    { { 0x00, 0x00, 0xFF, 0 } }, /* SolidBlue */
    { { 0xEE, 0x4D, 0x4D, 0 } }, /* AppRed */
    { { 0x63, 0xB3, 0x67, 0 } }, /* AppGreen */
    { { 0x2D, 0x2D, 0xDD, 0 } }, /* AppBlue */
    { { 0x07, 0x00, 0x27, 0 } }, /* AppPhone */
    { { 0x38, 0x52, 0x4C, 0 } }, /* AppDisplay */
    { { 0x90, 0xCF, 0xC1, 0 } }, /* AppText1 */
    { { 0x00, 0x00, 0x00, 0 } }, /* BLANK */
};

SDL_Color PRIAX_Color(PRIAX_ColorIndex index)
{
    const static int size = sizeof(PRIAX_ColorList)/sizeof(PRIAX_ColorList[0]);

    if(index > size) index = 0;

    return PRIAX_ColorList[index].color;
}
