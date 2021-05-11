/*
 * $Id: anim.c 570 2006-05-04 17:50:36Z arnaldo $
 */

#include "priax.h"

/* range type */
typedef enum {
    Increase,
    Decrease,
    Stalled,
} RangeType;

/* generic animation */
typedef struct {
    int              id;
    char            *number;
    PRIAX_AnimState  state;
    int              rangestart;
    int              rangeend;
    int              rangepos;
    RangeType        rangetype;
    const char      *animation;
} PRIAX_AnimDef;

/* animations */
const char DialAnimStr[]  = { '>', '\0' };
const char RecvAnimStr[]  = { '<', '\0' };
const char TalkAnimStr[]  = { '\\', '|', '/', '-', '\0' };
const char PauseAnimStr[] = { '=', '\0' };
static PRIAX_AnimDef PRIAX_AnimList[] = {
    { DialAnim,  NULL, AnimStop,  0, 10,  0, Increase, DialAnimStr  },
    { RecvAnim,  NULL, AnimStop, 10,  0, 10, Decrease, RecvAnimStr  },
    { TalkAnim,  NULL, AnimStop,  0,  0,  0, Stalled,  TalkAnimStr  },
    { PauseAnim, NULL, AnimStop,  0,  0,  0, Stalled,  PauseAnimStr },
};

/* current */
static PRIAX_AnimDef *CurrentAnim = &PRIAX_AnimList[DialAnim];

/* set number for animation */
void PRIAX_AnimSet(PRIAX_AnimIndex index, char *number)
{
    /* stop current anim before anything */
    if(CurrentAnim)
    CurrentAnim->state = AnimStop;

    /* change current anim */
    CurrentAnim = &PRIAX_AnimList[index];
    CurrentAnim->state  = AnimStop;
    CurrentAnim->number = number;
    //CurrentAnim->number = PRIAX_AddrBookLookup(number);
}

/* change state of animation */
void PRIAX_AnimControl(PRIAX_AnimState state)
{
    if(!CurrentAnim || !CurrentAnim->number) return;
    else
    CurrentAnim->state = state;

    /* clear animation's area */
    if(state == AnimPause || state == AnimStop) {
        PRIAX_LCDClear(LCD_AnimLine1);
        PRIAX_LCDClear(LCD_AnimLine2);
    }
}

/* return state of the current animation */
PRIAX_AnimState PRIAX_AnimGetState() { return CurrentAnim->state; }

/* update animation window */
void PRIAX_AnimUpdate()
{
    char temp[50];
    PRIAX_AnimDef *p = NULL;

    if(!CurrentAnim || !CurrentAnim->number || CurrentAnim->state != AnimPlay)
        return;
    else
        p = CurrentAnim;

    /* print number or name */
    if(p->id == RecvAnim) {
        PRIAX_LCDText(LCD_AnimLine2,
                p->rangepos % 2 ? " " : p->number);
    } else
        PRIAX_LCDText(LCD_AnimLine2, p->number);

    /* print animation character on the right position */
    switch(p->rangetype) {
        case Increase:
            memset(temp, 0, sizeof(temp));

            if(p->rangepos == p->rangeend)
                p->rangepos = p->rangestart;
            else
            if(p->rangepos)
                memset(temp, ' ', p->rangepos);

            temp[p->rangepos++] = *p->animation;
            PRIAX_LCDText(LCD_AnimLine1, temp);
            break;
        case Decrease:
            memset(temp, ' ', sizeof(temp));
            temp[p->rangestart+1] = '\0';
            temp[p->rangepos--] = *p->animation;
            
            if(!p->rangepos) p->rangepos = p->rangestart;

            PRIAX_LCDText(LCD_AnimLine1, temp);
            break;
        case Stalled:
            PRIAX_LCDText(LCD_AnimLine1, "%c", 
                    p->animation[p->rangepos++ % strlen(p->animation)]);
            break;
    }
}
