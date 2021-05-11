/*
 * $Id: iax2.h 576 2006-05-04 23:24:37Z arnaldo $
 */

#ifndef _IAX2_H
#define _IAX2_H

#include <iaxclient.h>

/* just to be able to use MAXNUM_DIGITS */
#include "global.h"

/* max number of avaliable IAX lines */
#define MAXLINES 2

/* ring files */
#define IAX2_RING1      "ring1.raw"
#define IAX2_RINGBACK   "ringback.raw"

/* states */
#define PRIAX_STATE_RINGING     (1 << 0)
#define PRIAX_STATE_OUTGOING    (1 << 1)
#define PRIAX_STATE_INCOMING    (1 << 2)
#define PRIAX_STATE_ACTIVE      (1 << 3)
#define PRIAX_STATE_WAITING     (1 << 4)

/* change state of currently pointed call */
#define PRIAX_SET_CURRENT_STATE(state)   (IAX2Get()->private.current->PRIAX_state |=  (state))
#define PRIAX_UNSET_CURRENT_STATE(state) (IAX2Get()->private.current->PRIAX_state &= ~(state))
#define PRIAX_CHECK_CURRENT_STATE(state) (IAX2Get()->private.current->PRIAX_state &   (state))
#define PRIAX_FREE_CURRENT_STATE         (IAX2Get()->private.current->PRIAX_state =         0)

/* change state of #line call */
#define PRIAX_SET_STATE(line, state)     (IAX2Get()->private.callinfo[(line)].PRIAX_state |=  (state))
#define PRIAX_UNSET_STATE(line, state)   (IAX2Get()->private.callinfo[(line)].PRIAX_state &= ~(state))
#define PRIAX_CHECK_STATE(line, state)   (IAX2Get()->private.callinfo[(line)].PRIAX_state &   (state))
#define PRIAX_FREE_STATE(line)           (IAX2Get()->private.callinfo[(line)].PRIAX_state =         0)

/* registry information */
typedef struct {
    char user[64];
    char pass[64];
    char host[256];
    int  state; /* 0 unregistered, 1 registered */
} IAX2_RegistryDef;

/* call information */
typedef struct {
    int                       id;                    /* match iaxc callNo                       */
    int                       PRIAX_state;           /* we _can't_ rely on libiaxclient's state */
    char                      number[MAXNUM_DIGITS]; /* call number                             */
    int                       mute_state;            /* 0 talking, 1 mute                       */
    int                       answered;              /* wether this call was answered or not    */
    SDL_Thread               *speaker_thread;        /* thread that makes the speaker ring      */

    /* sound for this call 
     * WARNING: iaxc_sound must be at the end of this structure. */
    struct iaxc_sound sound;
} IAX2_CallDef;

/* private data */
typedef struct {
    /* registry */
    IAX2_RegistryDef  reginfo;

    /* calls */
    IAX2_CallDef      callinfo[MAXLINES]; 
    IAX2_CallDef     *current;                /* current call (see index below)             */
    IAX2_CallDef      callwaiting;            /* just to hold the call waiting ring tone ?  */
    int               index;                  /* callinfo[index]                            */
    int               last_active;            /* remember index of last-active call         */

    /* IAX2 state */
    int               state;                  /* 0 means problems, 1 means ready            */

    /* ringback */
    struct iaxc_sound rb_sound;

    /* remember sound devices data */
    struct iaxc_audio_device *snd_devices;
    int                       snd_ndevs;      /* number of sound devices                    */
    int                       snd_input;      /* index of input device                      */
    int                       snd_output;     /* index of output device                     */
    int                       snd_ring;       /* index of ring device                       */

} IAX2_PrivateDef;

struct _IAX2Data {
    /* private information */
    IAX2_PrivateDef private;

    /* config pointer */
    PRIAX_Conf *conf;

    /* registry information */
    void  (*setregistry)   (char *user, char *pass, char *host);
    void  (*setcallerid)   (char *name, char *number);

    /* call on and off */
    int   (*dial)          (char *number); /* dial                              */
    void  (*hangup)        (int index);    /* hangup current call               */
    void  (*pickup)        ();             /* pickup current call               */
    int   (*prev_call)     ();             /* select prev call or line          */
    int   (*next_call)     ();             /* select next call or line          */
    int   (*select_call)   (int index);    /* select call                       */

    /* call features */
    void  (*hold)          ();             /* hold current call                 */
    void  (*unhold)        ();             /* unhold current call               */
    void  (*mute)          ();             /* mute current call                 */
    void  (*unmute)        ();             /* unmute current call               */
    void  (*dtmf)          (char digit);   /* send DTMF digit                   */
    void  (*transfer)      ();             /* attended transfer                 */

    /* related to current call */
    int   (*is_active)     ();             /* 1 if current call is active       */
    int   (*is_ringing)    ();             /* 1 if there is someone calling     */
    int   (*selected)      ();             /* selected call, -1 if none         */
    char *(*number)        ();             /* return number of current call     */

    /* devices */
    int   (*setdevices)    (char *in, char *out, char *ring); /* selected call, -1 if none */
};

typedef struct _IAX2Data* IAX2ptr;

/* get current call's number and working state */
#define IAX2READY(iax2)  iax2->private.state

/* ptr to access IAX2 functions */
extern IAX2ptr IAX2Get();

#endif /* new-iax2.h */
