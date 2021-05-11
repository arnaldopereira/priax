/*
 * $Id: tray.h 560 2006-04-27 18:54:48Z arnaldo $
 */

#ifndef _TRAY_H
#define _TRAY_H

#ifdef X11_TRAY_ICON

/* system */
#include <stdio.h>
#include <string.h>

/* X11 */
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#ifndef TOGGLE
#define TOGGLE(x) x=!x
#endif

/* Application's data */
typedef struct {
    Display *dpy;
    int      scr;
    GC       gc;
    Window   win;
    Window   appwin;
    int      appx;
    int      appy;
    int      lock;

    /* image information */
    Pixmap   pixmap;
    int      width;
    int      height;
} TrayContext;

extern int  TrayInit    (TrayContext *tc, Display *display, Window appwin);
extern int  TrayRealize (TrayContext *tc);
extern void TrayShow    (TrayContext *tc);
extern void TrayHide    (TrayContext *tc);
extern void TrayClose   (TrayContext *tc);

#elif defined(WIN32)

#include "priax.h"

typedef struct
{
   //stores the original message handler
   WNDPROC OriginalWndHandler;
   //handle to the window used
   HWND hWnd;
   //handle to pop up menu
   HMENU hMenu;
   //handle to the icon used
   HICON hTrayIcon;
   //unique message id used
   unsigned MsgId;
} TrayContext;

extern int  TrayInit    (TrayContext *tc);
extern int  TrayRealize (TrayContext *tc);
extern void TrayShow    (TrayContext *tc);
extern void TrayHide    (TrayContext *tc);
extern void TrayClose   (TrayContext *tc);
extern void TraySetToolTip(TrayContext *tc, const char *szMsg);

#endif /* X11_TRAY_ICON */


#endif /* tray.h */
