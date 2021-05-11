/*
 * $Id: tray.c 579 2006-05-08 14:45:03Z arnaldo $
 */

#include "priax.h"

#if defined(X11_TRAY_ICON) && !defined(WIN32)
#include "tray_xpm.h"

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define TRAY_EVENT_MASK (VisibilityChangeMask|ButtonPressMask)

/* freedesktop.org specification */
static int TrayGenericDock(TrayContext *tc)
{
    XEvent ev;
    char buf[32];
    Atom selectionAtom;
    Window selectionWin;

    snprintf(buf, sizeof(buf), "_NET_SYSTEM_TRAY_S%d", tc->scr);
    selectionAtom = XInternAtom(tc->dpy, buf, True);
    if(selectionAtom) {
        debug("Atom [%s] not found.\n", buf);
        return 1;
    }

    XGrabServer(tc->dpy);
    selectionWin = XGetSelectionOwner(tc->dpy, selectionAtom);
    XUngrabServer(tc->dpy);
    XSync(tc->dpy, False);

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = selectionWin;
    ev.xclient.message_type = 
        XInternAtom (tc->dpy, "_NET_SYSTEM_TRAY_OPCODE", True);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = CurrentTime;
    ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
    ev.xclient.data.l[2] = tc->win;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;

    XSendEvent(tc->dpy, selectionWin, False, NoEventMask, &ev);
    XSync(tc->dpy, False);

    return 0;
}

/* create tray's window */
int TrayInit(TrayContext *tc, Display *display, Window appwin)
{
    Atom atom;
    Pixmap mask;
    XWindowAttributes wa;
    XpmAttributes attr = {
        .closeness = 40000,
        .valuemask = XpmCloseness | XpmSize
    };

    int n;
    char temp[64];

    if(!tc || !display) return -1;

    tc->lock = True;
    tc->dpy = display;
    tc->scr = XScreenNumberOfScreen(DefaultScreenOfDisplay(tc->dpy));
    tc->win = XCreateSimpleWindow(display, RootWindow(tc->dpy, tc->scr),
            0, 0, 1, 1, 1, 0, 0);
    tc->gc  = XCreateGC(tc->dpy, tc->win, 0, NULL);

    /* save application's window to make map/unmap (hide/unhide) */
    tc->appwin = appwin;

    /* dock icon according freedesktop's spec */
    if(TrayGenericDock(tc)) {
        debug("TrayGenericDock() returned error.\n");
        //return -2;
    }

    /* change window property to make it dockable - under KDE */
    snprintf(temp, sizeof(temp), "%s", "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
    atom = XInternAtom(tc->dpy, (char *) temp, True);
    if(atom) {
        XChangeProperty(tc->dpy, tc->win, atom, XA_WINDOW, 32,
                PropModeReplace, (unsigned char *) &appwin, 1);
    } else
        debug("Atom [%s] doesn't exist.\n", temp);

    /* tray window's events */
    XSelectInput(tc->dpy, tc->win, TRAY_EVENT_MASK);

    /* map */
    XMapWindow(tc->dpy, tc->win);
    XSync(tc->dpy, False);

    /* load xpm buffer as Pixmap */
    n = XpmCreatePixmapFromData(tc->dpy, RootWindow(tc->dpy, tc->scr),
            tray_xpm, &tc->pixmap, &mask, &attr);

    if(n != XpmSuccess) return -2;

    tc->width  = attr.width;
    tc->height = attr.height;

    /* make it transparent */
    XShapeCombineMask(tc->dpy, tc->win, ShapeBounding, 0, 0,
            mask, ShapeSet);

    /* we don't need mask anymore */
    XSync(tc->dpy, False);
    //XFreePixmap(tc->dpy, mask);
    XSync(tc->dpy, False);

    /* adjust window size to icon's size */
    XGetWindowAttributes(tc->dpy, tc->win, &wa);
    if(wa.width < tc->width || wa.height < tc->height)
        XResizeWindow(tc->dpy, tc->win, tc->width, tc->height);
    XSync(tc->dpy, False);
    tc->lock = False;

    return 0;
}

/* realize */
int TrayRealize(TrayContext *tc)
{
    if(!tc || !tc->dpy || tc->lock) return -1;

    /* blit */
    XCopyArea(tc->dpy, tc->pixmap, tc->win, tc->gc, 0, 0,
            tc->width, tc->height, 0, 0);
    XSync(tc->dpy, False);

    return 0;
}

void TrayShow(TrayContext *tc)
{
    if(!tc || !tc->dpy || tc->lock) return;

    XMapWindow(tc->dpy, tc->appwin);
    XSync(tc->dpy, False);

    /* FIXME: SDL_WarpMouse() is here to force focus on KDE wm */
    SDL_WarpMouse(195, 203);
}

void TrayHide(TrayContext *tc)
{
    if(!tc || !tc->dpy || tc->lock) return;

    XUnmapWindow(tc->dpy, tc->appwin);
    XSync(tc->dpy, False);
    //SDL_WM_IconifyWindow();
}

void TrayClose(TrayContext *tc)
{
    if(!tc || !tc->dpy) return;
    else
        tc->lock = True;
        
    /* delete everything */
    XFreePixmap(tc->dpy, tc->pixmap);
    XFreeGC(tc->dpy, tc->gc);
    XDestroyWindow(tc->dpy, tc->win);
    XSync(tc->dpy, False);
}

#elif defined(WIN32)
///////////////////////////////////////////////////////////////////////////////
//win32 version
///////////////////////////////////////////////////////////////////////////////

#define _WIN32_IE 0x0600

#include <windows.h>
#include <winuser.h>
#include <shellapi.h>
#include "tray.h"

//just an identifier
#define TI_DEFAULT_ICON_INDEX 0xabad1cc0  
//the message id I'm using to identify tray messages
//#define TI_MSG (WM_USER + 0x1234)
//this is the id of exit menu item
#define TI_MENU_EXIT    0x1609
#define TI_MENU_SHOW    0x160A

//this string will be displayed to the user when the mouse pointer is over the tray icon
//must be truncated to 128 bytes including '\0'
#define TI_TIP_MESSAGE "PRIAX"

//this is the name of the file that contains the icon showed on the taskbar
#define TI_ICON_FILE    "images\\priax.ico"

//global variables
TrayContext * TrayContextCopy;
//functions
static LRESULT TrayMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TrayMsgHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static int TrayPopMenu(TrayContext *tc, unsigned x, unsigned y);
static void TrayAddMenuItem(TrayContext *tc, LPSTR szItem, DWORD MenuId);

//adds an item to the pop up menu shown when the icon is right clicked
//to add a separator, pass "-" as string
static void TrayAddMenuItem(TrayContext *tc, LPSTR szItem, DWORD MenuId)
{
    MENUITEMINFO mii;

    if(!tc->hMenu)
        tc->hMenu = CreatePopupMenu();
    mii.cbSize = sizeof(mii);

    mii.fMask = MIIM_ID | MIIM_TYPE;

    if(szItem[0] == '-')
        mii.fType = MFT_SEPARATOR;
    else
        mii.fType = MFT_STRING;

    mii.cch = strlen(szItem);
    mii.dwTypeData = szItem;
    mii.wID = MenuId; 
    InsertMenuItem(tc->hMenu, 0, TRUE, &mii);
    //AppendMenu(tc->hMenu, MF_STRING /*| MF_POPUP*/, (UINT) tc->hMenu, szItem); 
}

static int TrayPopMenu(TrayContext *tc, unsigned x, unsigned y)
{
    if(tc->hMenu)
    {
        if(TrackPopupMenu(tc->hMenu, 
            TPM_LEFTALIGN | TPM_LEFTBUTTON, //TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERNEGANIMATION, 
            x, y, 
            0, tc->hWnd, NULL))
        return 0;
    }
    return 1;
}

//callback to window handler
static LRESULT CALLBACK TrayMsgHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if(Msg == TrayContextCopy->MsgId)
        return TrayMessage(hWnd, Msg, wParam, lParam);
    else
        if(Msg == WM_COMMAND)
        {
            if((wParam & 0xffff) == TI_MENU_EXIT)
            {
                my_exit(EXIT_SUCCESS, NULL);
                return 0;
            }
            else
            {
                if((wParam & 0xffff) == TI_MENU_SHOW)
                {
                    if(IsWindowVisible(TrayContextCopy->hWnd))
                    {
                        ShowWindow(TrayContextCopy->hWnd, SW_HIDE);
                    }
                    else
                    {
                        ShowWindow(TrayContextCopy->hWnd, SW_SHOW);
                    }
                    return 0;
                }
            }
        }

    if(TrayContextCopy)
        return CallWindowProc(TrayContextCopy->OriginalWndHandler, hWnd, Msg, wParam, lParam);
    return 0;
}

//handles specifics tray's messages
static LRESULT TrayMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    POINT P;

    //I'm interested in the left button click only
    if(lParam == WM_RBUTTONUP) {
        GetCursorPos(&P);
        TrayPopMenu(TrayContextCopy, P.x, P.y);
    }
    else
    {
        //hides the program
        /*
        if(lParam == WM_LBUTTONUP)
        {
            ShowWindow(TrayContextCopy->hWnd, SW_HIDE);
        }
        */
    }
    return 0;
}

//initializes the tray icon
int TrayInit(TrayContext *tc)
{
    SDL_SysWMinfo info;
    NOTIFYICONDATA nid;
    HICON hTrayIcon;

    TrayContextCopy = tc;

    SDL_VERSION(&info.version);
    if(SDL_GetWMInfo(&info)) {
        tc->hWnd = info.window;
    }
    else
        return 1;

    //I have to intercept the current message handler to handle tray messages
    //like in a subclassing
    //keep old handler
    tc->OriginalWndHandler = (WNDPROC)GetWindowLong(tc->hWnd, GWL_WNDPROC);
    if(!tc->OriginalWndHandler) {
        //some error has ocurred
        return 1;
    }

    //set new handler
    if(!SetWindowLong(tc->hWnd, GWL_WNDPROC, (unsigned)TrayMsgHandler)) {
        //some error has ocurred
        return 1;
    }

    //-w- debug just a test to see if the XP bug disappears
    if(!SetWindowLong(tc->hWnd, GWL_EXSTYLE, GetWindowLong(tc->hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED)) {
        //ignore errors
    }

    //notifies the shell we want a icon in the taskbar
    memset(&nid, 0, sizeof(nid));

    //nid.cbSize = NOTIFYICONDATA_V2_SIZE; //sizeof(nid);
    nid.cbSize = sizeof(nid);
    //creates an unique message id
    //to avoid conflicts with other unknown sdl messages
    tc->MsgId = RegisterWindowMessage("priaxwin");
    nid.uID = TI_DEFAULT_ICON_INDEX;
    nid.uCallbackMessage = tc->MsgId;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
    nid.hWnd = tc->hWnd;
    strcpy(nid.szTip, TI_TIP_MESSAGE);

    /* Balloon text & title */
    strcpy(nid.szInfo, TEXT("teste de balloon do priax"));
    strcpy(nid.szInfoTitle, TEXT("Título"));

    hTrayIcon = LoadImage(NULL, TI_ICON_FILE, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADFROMFILE);

    if(hTrayIcon)
    {
        nid.hIcon = hTrayIcon;
    }

    Shell_NotifyIcon(NIM_ADD, &nid);

    TrayAddMenuItem(tc, "Sair do PRIAX", TI_MENU_EXIT);
    TrayAddMenuItem(tc, "-", 0);
    TrayAddMenuItem(tc, "Mostrar/Ocultar", TI_MENU_SHOW);

    return 0;
}

//nothing to do yet
void TrayShow(TrayContext *tc)
{

}

//nothing to do yet
void TrayHide(TrayContext *tc)
{

}

//cleanup tray icon system, MUST be called
void TrayClose(TrayContext *tc)
{
    NOTIFYICONDATA nid;

    //protection against disasters
    if(tc->OriginalWndHandler)
    {
        //restores the original message handler
        if(!SetWindowLong(tc->hWnd, GWL_WNDPROC, (unsigned)tc->OriginalWndHandler)) 
        {
            //this never can happen
            //log?
        }

        //notifies the shell we want to remove the taskbar icon
        memset(&nid, 0, sizeof(nid));
        nid.hWnd = tc->hWnd;
        nid.uID = TI_DEFAULT_ICON_INDEX;
        nid.cbSize = sizeof(nid);
        nid.uFlags = NIF_ICON;
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }

    //frees unused resources
    if(tc->hTrayIcon)
        DestroyIcon(tc->hTrayIcon);

    if(tc->hMenu)
        DestroyMenu(tc->hMenu);

    //don't allow reuse?
    memset(tc, 0, sizeof(*tc));
}

//call to change the tooltip text
void TraySetToolTip(TrayContext *tc, const char *szMsg)
{
    NOTIFYICONDATA nid;

    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = tc->hWnd;
    nid.uID = TI_DEFAULT_ICON_INDEX;
    nid.uFlags = NIF_TIP;
    strcpy(nid.szTip, szMsg);

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

#endif /* X11_TRAY_ICON */
