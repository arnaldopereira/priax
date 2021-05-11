/*
 * $Id: anim.h 130 2005-11-18 20:46:05Z arnaldo $
 */

#ifndef _ANIM_H
#define _ANIM_H

typedef enum {
    DialAnim,
    RecvAnim,
    TalkAnim,
    PauseAnim,
} PRIAX_AnimIndex;

typedef enum {
    AnimPlay,
    AnimStop,
    AnimPause,
} PRIAX_AnimState;

extern void            PRIAX_AnimSet     (PRIAX_AnimIndex index, char *number);
extern void            PRIAX_AnimControl (PRIAX_AnimState state);
extern PRIAX_AnimState PRIAX_AnimGetState();

/* only used by the clock thread when needed */
extern void            PRIAX_AnimUpdate  ();

#endif /* anim.h */
