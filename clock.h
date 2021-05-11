/*
 * $Id: clock.h 60 2005-10-27 23:07:02Z alec $
 */

#ifndef _CLOCK_H
#define _CLOCK_H

extern void PRIAX_ClockThread();
extern void PRIAX_PrintClock();

/* print voicemail message via clock thread */
extern void PRIAX_VoiceMailON();
extern void PRIAX_VoiceMailOFF();

/* return voicemail's state and fills text rectangle */
extern int  PRIAX_VoiceMail(SDL_Rect *rect);

#endif /* clock.h */
