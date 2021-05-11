/*
 * $Id: callreg.h 392 2005-12-16 21:32:45Z arnaldo $
 */

#ifndef _CALLREG_H
#define _CALLREG_H

/* these values MUST NOT be less than LCD_TextLineX */
#define MAXLOG_SEND 10
#define MAXLOG_RECV 10
#define MAXLOG_LOST 10

/* log type */
typedef enum {
    LogSend,  /* calls dialled */
    LogRecv,  /* calls received */
    LogLost,  /* missed calls */
} PRIAX_LogType;

extern void PRIAX_CallReg();
extern void PRIAX_LogAdd(PRIAX_LogType logtype, const char *number);
extern void PRIAX_DumpCalls();
extern void CR_LCDClear();

#endif /* callreg.h */
