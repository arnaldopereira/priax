/*
 * $Id: callreg.c 570 2006-05-04 17:50:36Z arnaldo $
 */

#include "priax.h"

/* generic log entry */
typedef struct {
    char number[MAXNUM_DIGITS];
    time_t date;
} LogData;

/* calls dialled */
static LogData sendlog[MAXLOG_SEND];
static int     sendlog_pos = 0; /* position */
static int     sendlog_off = 0; /* offset for printing and scrolling */
#define SL_INC (sendlog_pos++ % MAXLOG_SEND) /* incremental l00p */
#define SL_POS ((sendlog_pos - 1) % MAXLOG_SEND) /* current position */

/* calls received */
static LogData recvlog[MAXLOG_RECV];
static int     recvlog_pos = 0;
static int     recvlog_off = 0;
#define RL_INC (recvlog_pos++ % MAXLOG_RECV)
#define RL_POS ((recvlog_pos - 1) % MAXLOG_RECV)

/* calls lost */
static LogData lostlog[MAXLOG_LOST];
static int     lostlog_pos = 0;
static int     lostlog_off = 0;
#define LL_INC (lostlog_pos++ % MAXLOG_LOST)
#define LL_POS ((lostlog_pos - 1) % MAXLOG_LOST)

/* current records to show */
static int CR_current_record = LogSend;

/* show records */
static void CR_show(int logtype);

/* our own keyboard handler */
static void CR_EventHandler(char *keyname);

/* load/save all calls from/to file */
static void CR_LoadCalls();
static void CR_DumpCalls();

/* LCD properties */
int CR_display[] =
    { LCD_TextLine1, LCD_TextLine2, LCD_TextLine3,
      LCD_TextLine4, LCD_TextLine5, LCD_TextLine6 };

/* initialize callreg */
void PRIAX_CallReg()
{
    static int initialized = 0;

    if(!initialized) {
        TOGGLE(initialized);
        CR_LoadCalls();
    }

    /* check animation state */
    if(PRIAX_AnimGetState() == AnimPlay)
        PRIAX_AnimControl(AnimPause);

    /* change keypad events */
    PRIAX_KeypadSetEv(CR_EventHandler);

    /* show records */
    CR_show(CR_current_record);
}

/* returns a pointer to the current record */
static LogData *CR_CurrentRecord()
{
	LogData *p = NULL;

	switch (CR_current_record) {
		case LogSend:
			p = &sendlog[SL_POS - sendlog_off];
			break;
		case LogRecv:
			p = &recvlog[RL_POS - recvlog_off];
			break;
		case LogLost:
			p = &lostlog[LL_POS - lostlog_off];
			break;
	}

	return p;
}

/* show list of calls dialled or received */
static void CR_show(int logtype)
{
    int i, j, pos, off, maxlog, *display;
    const static int size = sizeof(CR_display)/sizeof(CR_display[0]);
    char *label = NULL;
    LogData *p, *reg = NULL;

    struct tm *tm = NULL;

    switch(logtype) {
        case LogSend:
            pos = SL_POS;
            maxlog = MAXLOG_SEND;
            label = _("   Calls dialled     >");
            reg = sendlog;
            off = sendlog_off;
            break;
        case LogRecv:
            pos = RL_POS;
            maxlog = MAXLOG_RECV;
            label = _("< Calls received  >");
            reg = recvlog;
            off = recvlog_off;
            break;
        case LogLost:
            pos = LL_POS;
            maxlog = MAXLOG_LOST;
            label = _("< Missed calls");
            reg = lostlog;
            off = lostlog_off;
            break;
        default:
            return;
    }

    PRIAX_LCDText(LCD_TextInfo, label);
    
    /* print */
    for(i = 0, j = pos - off, display = CR_display;
            i < size ; i++, j-- % maxlog) {

        /* if j become negative, we loop into the array
         * ex: maxlog == 5, j == -2, position is 3*/
        if(j < 0) j = maxlog + j;

        p = &reg[j];
        if(*p->number == '\0' || i == maxlog - off) break;

        tm = localtime(&p->date);
        PRIAX_LCDText(*display++, "%04d.%02d.%02d %02d:%02d:%02d  %s",
                tm->tm_year + 1900, tm->tm_mon+1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec,
                p->number);
    }
}

/* clear whole display */
void CR_LCDClear()
{
    int i, *display = CR_display;
    const int size = sizeof(CR_display)/sizeof(CR_display[0]);

    PRIAX_LCDClear(LCD_TextInfo);

    for(i = 0; i < size; i++, display++)
        PRIAX_LCDClear(*display);
}

static void CR_FireRedial()
{
	LogData *p = CR_CurrentRecord();

	CR_LCDClear();

	if (!p || !p->number || !strlen(p->number)) {
		return;
	}

	PRIAX_Event();

        /* dial */
        {
            IAX2ptr iax2 = IAX2Get();
            iax2->dial(p->number);
        }
}

/* keyboard handler */
static void CR_EventHandler(char *keyname)
{
	/* assume a redial */
	if (keyname == NULL) {
		CR_FireRedial();
		return;
	}

    /* go back to the default keyboard handler - event.c */
    if(!strncmp(keyname, "hangup", strlen(keyname))) {
        CR_LCDClear();

        /* check animation state */
        if(PRIAX_AnimGetState() == AnimPause)
            PRIAX_AnimControl(AnimPlay);

        /* go back to the default event handler */
        PRIAX_Event();
    } else
    if(!strncmp(keyname, "talk", strlen(keyname))) {
        LogData *p = CR_CurrentRecord();

        /* go back to the default event handler... */
        CR_EventHandler("hangup");

        /* update information for this record */
        p->date = time(NULL);

        /* call */
        if (numeric_buffer_size()) {
            IAX2ptr iax2 = IAX2Get();
            iax2->dial(p->number);
        } else
            CR_FireRedial(sendlog[SL_POS - sendlog_off].number);

    } else
    if(!strncmp(keyname, "left", strlen(keyname))) {
        switch(CR_current_record) {
            case LogSend:
                return;
            case LogRecv:
                CR_current_record = LogSend;
                break;
            case LogLost:
                CR_current_record = LogRecv;
                break;
        }

        CR_LCDClear();
        CR_show(CR_current_record);
    } else
    if(!strncmp(keyname, "right", strlen(keyname))) {
        switch(CR_current_record) {
            case LogSend:
                CR_current_record = LogRecv;
                break;
            case LogRecv:
                CR_current_record = LogLost;
                break;
            case LogLost:
                return;
        }

        CR_LCDClear();
        CR_show(CR_current_record);
    } else
    if(!strncmp(keyname, "up", strlen(keyname))) {
        int *off;
        switch(CR_current_record) {
            case LogSend:
                off = &sendlog_off;
                break;
            case LogRecv:
                off = &recvlog_off;
                break;
            case LogLost:
                off = &lostlog_off;
                break;
            default:
                return;
        }

        *off -= 1; /* fuck! I just cant do *off-- */
        if(*off < 0) *off = 0;

        CR_LCDClear();
        CR_show(CR_current_record);
    } else
    if(!strncmp(keyname, "down", strlen(keyname))) {
        LogData *p;
        int *off, pos, maxlog;
        switch(CR_current_record) {
            case LogSend:
                p = sendlog;
                off = &sendlog_off;
                pos = SL_POS;
                maxlog = MAXLOG_SEND;
                break;
            case LogRecv:
                p = recvlog;
                off = &recvlog_off;
                pos = RL_POS;
                maxlog = MAXLOG_RECV;
                break;
            case LogLost:
                p = lostlog;
                off = &lostlog_off;
                pos = LL_POS;
                maxlog = MAXLOG_LOST;
                break;
            default:
                return;
        }

        /* these cannot be negative */
        if(pos < 0) pos = 0; if(*off < 0) *off = 0;

        *off += 1; /* damn! *off++ just don't work! */
        if(*off == maxlog) *off -= 1;
        else if(*p[*off].number == '\0') *off = pos;

        CR_LCDClear();
        CR_show(CR_current_record);
    }
}

/* add a call to the dialled or received buffer */
void PRIAX_LogAdd(PRIAX_LogType logtype, const char *number)
{
    LogData *p = NULL;

    if(!number) return;

    /* define where to save */
    switch(logtype) {
        case LogSend:
            p = &sendlog[SL_INC];
            break;
        case LogRecv:
            p = &recvlog[RL_INC];
            break;
        case LogLost:
            p = &lostlog[LL_INC];
            break;
        default:
            return;
    }

    /* save it! */
    strncpy(p->number, number, sizeof(p->number));
    //printf("saved[%d]: %s\n", SL_INC, p->number);
    p->date = time(NULL);
}

/* load all records from file */
static void CR_LoadCalls()
{
    int i, j, k, *pos = NULL, maxlog = 0;
    FILE *fp;

    LogData sl[MAXLOG_SEND];
    LogData rl[MAXLOG_RECV];
    LogData ll[MAXLOG_LOST];
    LogData *ps = NULL, *pd= NULL;

    /* prepare file for reading */
    if((fp = fopen(PRIAX_Path_CALLS(PRIAX_CALLS), "r")) == NULL)
        return;

    for(k = 0; k < 3; k++) {
        switch(k) {
            case 0:
                pd = sendlog;
                ps = sl;
                pos = &sendlog_pos;
                maxlog = MAXLOG_SEND;
                break;
            case 1:
                pd = recvlog;
                ps = rl;
                pos = &recvlog_pos;
                maxlog = MAXLOG_RECV;
                break;
            case 2:
                pd = lostlog;
                ps = ll;
                pos = &lostlog_pos;
                maxlog = MAXLOG_LOST;
                break;
        }

        memset(ps, 0, maxlog);
        memset(pd, 0, maxlog);

        /* read data */
        fread(ps, sizeof(ps[0]), maxlog, fp);

        /* determine pos */
        for(i = 0; i < maxlog; i++)
            if(*ps[i].number == '\0') break;

        /* pos cannot be negative */
        if((*pos = i - 1) < 0) *pos = 0;
        
        /* copy */
        for(i = 0, j = *pos; i < maxlog && j >= 0; i++, j--) {
            memcpy(&pd[i], &ps[j], sizeof(pd[i]));
            //fprintf(stdout, "READING => %d = (%d) %s\n", i, j, pd[i].number);
        }

        if(i) i--;
        if(*pd[i].number != '\0') *pos += 1;
    }

    fclose(fp);
}

/* dump all records before quit */
void PRIAX_DumpCalls() { CR_DumpCalls(); }

static void CR_DumpCalls()
{
    FILE *fp;
    int i, j, k, pos = 0, maxlog = 0;

    LogData sl[MAXLOG_SEND];
    LogData rl[MAXLOG_RECV];
    LogData ll[MAXLOG_LOST];
    LogData *ps = NULL, *pd= NULL;

    /* prepare file to write data */
    if((fp = fopen(PRIAX_Path_CALLS(PRIAX_CALLS), "w")) == NULL)
        return;

    for(k = 0; k < 3; k++) {
        switch(k) {
            case 0:
                ps = sendlog;
                pd = sl;
                pos = SL_POS;
                maxlog = MAXLOG_SEND;
                break;
            case 1:
                ps = recvlog;
                pd = rl;
                pos = RL_POS;
                maxlog = MAXLOG_RECV;
                break;
            case 2:
                ps = lostlog;
                pd = ll;
                pos = LL_POS;
                maxlog = MAXLOG_LOST;
                break;
        }

        memset(pd, 0, maxlog);
    
        for(i = 0, j = pos; i < maxlog; i++, j-- % maxlog) {
            if(j == -1) j = maxlog - 1;
            memcpy(&pd[i], &ps[j], sizeof(pd[i]));
            //fprintf(stdout, "%d = (%d) %s\n", i, j, pd[i].number);
        }

        fwrite(pd, sizeof(pd[0]), maxlog, fp);
    }

    fclose(fp);
}
