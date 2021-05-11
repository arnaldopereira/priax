/*
 * $Id: conf.h 384 2005-12-16 19:22:35Z arnaldo $
 */

#ifndef _CONF_H
#define _CONF_H

typedef struct {
    char user[50];        /* iax user */
    char pass[50];        /* iax pass */
    char host[128];       /* iax host - address of asterisk server */
    char name[32];        /* caller name */
    char number[32];      /* caller number */
    char mailbox[32];     /* # of server's mailbox */
    char lang[10];        /* priax's language - same as use $LANG */
    char pickupexten[12]; /* pickup extension */
    char atxferexten[12]; /* attended transfer extension */

    char snd_dev_input[32];  /* sound devices */
    char snd_dev_output[32]; 
    char snd_dev_ring[32];
} PRIAX_Conf;

PRIAX_Conf *PRIAX_GetConf(const char *filename);

#endif
