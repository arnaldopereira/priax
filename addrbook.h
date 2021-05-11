/*
 * $Id: addrbook.h 45 2005-10-25 00:53:56Z alec $
 */

#ifndef _ADDRBOOK_H
#define _ADDRBOOK_H

/* max number of entries on the address book */
#define MAXAB_ENTRIES 1000
#define MAXAB_NAMELEN 40

/* generic addrbook entry */
typedef struct {
    double number;
    char name[MAXAB_NAMELEN];
} AB_entry;

extern void  PRIAX_InitAddrBook();
extern void  PRIAX_AddrBook();
extern inline char *PRIAX_AddrBookLookup(char *number);

#endif /* addrbook.h */
