/*
 * $Id: addrbook.c 105 2005-11-13 05:54:08Z alec $
 */

#include "priax.h"

/* addrbook ptr and data */
static AB_entry *AB_ptr, XAddrBook[MAXAB_ENTRIES];
static int       AB_pos = -1;
static int       AB_off = 0;

/* show entries and clear display */
static void AB_show();
static void AB_LCDClear();

/* event handler */
static void AB_EventHandler(char *keyname);

#ifdef WIN32
/* fix number convertion */
static char *fixnum(char *str)
{
    /* "%1.0f" doesn't work well with mingw... */
    char *px = str;
    while(*px != '\0')
        if(*px == '.') { *px = '\0'; break; }
        else px++;

    return str;
}
#endif

/* LCD properties */
int AB_display[] = 
    { LCD_TextLine1, LCD_TextLine2, LCD_TextLine4, LCD_TextLine5, };

/* init */
void PRIAX_InitAddrBook()
{
    FILE *fp;
    char *p, temp[1024];
    AB_entry *book = XAddrBook;

    memset(XAddrBook, 0, sizeof(XAddrBook));

    if((fp = fopen(PRIAX_Path_BOOK(PRIAX_BOOK), "r")) == NULL)
        return;

    while(!feof(fp) && AB_pos < MAXAB_ENTRIES) {
        memset(temp, 0, sizeof(temp));
        if(fgets(temp, sizeof(temp), fp) == NULL) break;

        /* blank lines or comments */
        if(ckline(temp)) continue;

        /* go to the first space and make it a \0 */
        p = temp;
        while(*p != '\0') {
            if(*p == ' ') { /* write \0 and go to the name */
                *p = '\0'; 
                p++;
                break;
            }
            else p++;
        }

        /* if there is only digits, go to the next entry */
        if(*p == '\0') continue;
        else while((*p == ' ' || *p == '\t') && *p != '\0') p++;

        /* check it again... */
        if(*p == '\0') continue;

        /* now temp has only digits and p has the name (we hope) */
        if(strlen(temp) > MAXNUM_DIGITS || strlen(p) > MAXAB_NAMELEN)
            continue;

        book->number = atof(temp);
        strncpy(book->name, fixstr(p), sizeof(book->name));

        AB_pos = AB_pos == -1 ? 0 : AB_pos + 1;
        book++;
    }

    fclose(fp);
    if(AB_pos != -1) AB_pos++;
    AB_ptr = XAddrBook;
}

/* set new event handler */
void PRIAX_AddrBook()
{
    /* check animation state */
    if(PRIAX_AnimGetState() == AnimPlay)
        PRIAX_AnimControl(AnimPause);

    /* clear the dial display */
    PRIAX_LCDClear(LCD_Dial);

    /* change event handler */
    PRIAX_KeypadSetEv(AB_EventHandler);
    AB_show();
}

/* lookup */
inline char *PRIAX_AddrBookLookup(char *number)
{
    int len;
    double num = atof(number);
    AB_entry *book = AB_ptr;

    /* if there is no addrbook we return the number, as we got it */
    if(AB_pos == -1 || !num) return number;
    else
    while(book && *book->name != '\0')
        if(book->number == num) return book->name;
        else book++;

    /* if number is not exactly the same, we try the hard lookup 
     * hard lookup will match numbers ignoring prefix but will be
     * used only for real phone numbers, from PSTN - never for
     * local asterisk channels/internal calls */
    if((len = strlen(number)) >= MAXNUM_REAL)
    {
        book = AB_ptr;
        int n1, n2 = len;
        char temp[MAXNUM_DIGITS], *p;

        while(book && *book->name != '\0') {
            memset(temp, 0, sizeof(temp));
            snprintf(temp, sizeof(temp), "%1.0f", book->number);
#ifdef WIN32
            fixnum(temp);
#endif

            n1 = strlen(temp);
            if(n2 > n1) {
                /* walk the difference */
                p = number; p+=n2-n1;

                /* compare */
                if(!strncmp(p, temp, n1))
                    return book->name;
            }
            
            book++;
        }
    }

    /* if we can't find number on that list, return the number as we got it */
    return number;
}

/* put addrbook on the LCD */
static void AB_show()
{
    AB_entry *p = AB_ptr;
    int i, *display = AB_display;
    const static int size = sizeof(AB_display)/sizeof(AB_display[0]);
    char temp[MAXNUM_DIGITS];

    PRIAX_LCDText(LCD_TextInfo, _("Address Book"));
    
    /* if there is nothing on the address book, return */
    if(AB_pos == -1) return;

    for(i = 0, p+=AB_off; i < size; i+=2, p++) {
        if(!p) break;
        else if(*p->name == '\0') break;

        /* print name */
        PRIAX_LCDText(*display++, "%s", p->name);

        memset(temp, 0, sizeof(temp));
        snprintf(temp, sizeof(temp), "%1.0f", p->number);

#ifdef WIN32
        fixnum(temp);
#endif
        /* print number */
        PRIAX_LCDText(*display++, "%s", temp);
    }
}

/* clear whole display */
static void AB_LCDClear()
{
    int i, *display = AB_display;
    const int size = sizeof(AB_display)/sizeof(AB_display[0]);

    PRIAX_LCDClear(LCD_TextInfo);

    for(i = 0; i < size; i++, display++)
        PRIAX_LCDClear(*display);
}

/* handle keypad events */
static void AB_EventHandler(char *keyname)
{
    if(keyname == NULL) return;

    /* go back to the default keyboard handler - event.c */
    if(!strncmp(keyname, "hangup", strlen(keyname))) {
        AB_LCDClear();

        /* check animation state */
        if(PRIAX_AnimGetState() == AnimPause)
            PRIAX_AnimControl(AnimPlay);

        /* go back to the default event handler */
        PRIAX_Event();
    } else
    if(!strncmp(keyname, "talk", strlen(keyname))) {
        char number[MAXNUM_DIGITS];

        AB_EventHandler("hangup");
        
        memset(number, 0, sizeof(number));
        snprintf(number, sizeof(number), "%1.0f", AB_ptr[AB_off].number);
#ifdef WIN32
        fixnum(number);
#endif

        /* dial */
        {
            IAX2ptr iax2 = IAX2Get();
            iax2->dial(number);
        }
    } else
    /* scroll up */
    if(!strncmp(keyname, "up", strlen(keyname))) {
        AB_off--;
        if(AB_off < 0) AB_off = 0;

        AB_LCDClear(); AB_show();
    } else
    /* scroll down */
    if(!strncmp(keyname, "down", strlen(keyname))) {
        AB_off++;
        if(AB_off == AB_pos) AB_off = AB_pos - 1;

        AB_LCDClear(); AB_show();
    }
}
