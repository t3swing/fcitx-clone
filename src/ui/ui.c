/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui/ui.h"

#include <X11/Xlocale.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <iconv.h>

#include "core/xim.h"
#include "core/MyErrorsHandlers.h"
#include "ui/MainWindow.h"
#include "ui/InputWindow.h"
#include "im/special/vk.h"
#include "core/IC.h"
#include "tools/tools.h"
#include "im/pinyin/sp.h"
#include "ui/about.h"
#include "ui/TrayWindow.h"

Display        *dpy;
int             iScreen;

#ifdef _USE_XFT
XftFont        *xftFont = (XftFont *) NULL;
XftFont        *xftFontEn = (XftFont *) NULL;
XftDraw        *xftDraw = (XftDraw *) NULL;
XftFont        *xftMainWindowFont = (XftFont *) NULL;
XftFont        *xftMainWindowFontEn = (XftFont *) NULL;
XftFont        *xftVKWindowFont = (XftFont *) NULL;
Bool            bUseBold = True;
int             iMainWindowFontSize = 9;
int             iVKWindowFontSize = 11;
int             iFontSize = 12;
char            strUserLocale[50] = "zh_CN.UTF-8";
#else
XFontSet        fontSet = NULL;
XFontSet        fontSetMainWindow = NULL;
XFontSet        fontSetVKWindow = NULL;
int             iMainWindowFontSize = 12;
int             iVKWindowFontSize = 12;
int             iFontSize = 16;
Bool            bUseBold = True;
char            strUserLocale[50] = "zh_CN.gbk";
#endif

GC              dimGC = (GC) NULL;
GC              lightGC = (GC) NULL;

#ifdef _USE_XFT
char            strFontName[100] = "AR PL ShanHeiSun Uni";
char            strFontEnName[100] = "Courier New";
#else
char            strFontName[100] = "*";
char            strFontEnName[100] = "Courier";
#endif

extern Window   mainWindow;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern Window   inputWindow;
extern int      iInputWindowX;
extern int      iInputWindowY;
extern Window   VKWindow;
extern int      iVKWindowX;
extern int      iVKWindowY;
extern Window   ximWindow;
extern WINDOW_COLOR mainWindowColor;
extern WINDOW_COLOR inputWindowColor;
extern WINDOW_COLOR VKWindowColor;
extern HIDE_MAINWINDOW hideMainWindow;
extern Bool bMainWindow_Hiden;
extern int      MAINWND_WIDTH;
extern Bool     bCompactMainWindow;
extern INT8     iIMIndex;
extern CARD16   connect_id;
extern Bool     bShowVK;
extern Bool     bVK;
extern Bool     bCorner;
extern Bool     bIsDisplaying;

#ifdef _ENABLE_TRAY
extern tray_win_t tray;
#endif

//added by yunfan
extern Window   aboutWindow;
extern Atom     about_protocol_atom;
extern Atom     about_kill_atom;
//**********************************

Bool InitX (void)
{
    if ((dpy = XOpenDisplay ((char *) NULL)) == NULL) {
    fprintf (stderr, "Error: FCITX can only run under X\n");
    return False;
    }

    SetMyXErrorHandler ();
    iScreen = DefaultScreen (dpy);

    return True;
}

void InitGC (Window window)
{
    XGCValues       values;
    XColor          color;
    int             iPixel;

    if (lightGC)
    XFreeGC (dpy, lightGC);
    lightGC = XCreateGC (dpy, window, 0, &values);
    color.red = LIGHT_COLOR;
    color.green = LIGHT_COLOR;
    color.blue = LIGHT_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
    iPixel = color.pixel;
    else
    iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, lightGC, iPixel);

    if (dimGC)
    XFreeGC (dpy, dimGC);
    dimGC = XCreateGC (dpy, window, 0, &values);
    color.red = DIM_COLOR;
    color.green = DIM_COLOR;
    color.blue = DIM_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
    iPixel = color.pixel;
    else
    iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, dimGC, iPixel);

    if (mainWindowColor.backGC)
    XFreeGC (dpy, mainWindowColor.backGC);
    mainWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowColor.backColor)))
    iPixel = mainWindowColor.backColor.pixel;
    else
    iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, mainWindowColor.backGC, iPixel);

    if (inputWindowColor.foreGC)
    XFreeGC (dpy, inputWindowColor.foreGC);
    inputWindowColor.foreGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.foreColor)))
    iPixel = inputWindowColor.foreColor.pixel;
    else
    iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.foreGC, iPixel);

    if (inputWindowColor.backGC)
    XFreeGC (dpy, inputWindowColor.backGC);
    inputWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.backColor)))
    iPixel = inputWindowColor.backColor.pixel;
    else
    iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.backGC, iPixel);

    if (VKWindowColor.foreGC)
    XFreeGC (dpy, VKWindowColor.foreGC);
    VKWindowColor.foreGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowColor.foreColor)))
    iPixel = VKWindowColor.foreColor.pixel;
    else
    iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, VKWindowColor.foreGC, iPixel);

    if (VKWindowColor.backGC)
    XFreeGC (dpy, VKWindowColor.backGC);
    VKWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowColor.backColor)))
    iPixel = VKWindowColor.backColor.pixel;
    else
    iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, VKWindowColor.backGC, iPixel);
}

/*
 * 当locale为UTF-8时，setlocale(LC_CTYPE,"")会导致程序崩溃
 */
/*void SetLocale(void)
{
    char *p;

    p=getenv("LC_CTYPE");
    if ( !p ) {
        p=getenv("LC_ALL");
        if ( !p)
            p=getenv("LANG");
    }

    //试验发现，当locale为zh_CN.UTF-8时，setlocale必须设置这zh_CN.UTF-8而不能是zh_CN.utf-8
    if ( p ) {
        strcpy(strUserLocale,p);
        p=strUserLocale;
        while ( *p!='.' && *p )
            p++;
        while ( *p ) {
            *p=toupper(*p);
            p++;
        }
        setlocale(LC_CTYPE,strUserLocale);
    }
    else
        setlocale(LC_CTYPE,"");
}
*/

#ifdef _USE_XFT
void CreateFont (void)
{
    //为了让界面能够正常显示，必须设置为中文模式
    if (strUserLocale[0])
    setlocale (LC_CTYPE, strUserLocale);

    if (xftFont)
    XftFontClose (dpy, xftFont);
    xftFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iFontSize, XFT_ANTIALIAS, XftTypeBool, True, NULL);

    if (xftFontEn)
    XftFontClose (dpy, xftFontEn);
    xftFontEn = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontEnName, XFT_SIZE, XftTypeDouble, (double) iFontSize, XFT_ANTIALIAS, XftTypeBool, True, NULL);

    if (xftMainWindowFont)
    XftFontClose (dpy, xftMainWindowFont);
    xftMainWindowFont =
    XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iMainWindowFontSize, XFT_ANTIALIAS, XftTypeBool, True, XFT_WEIGHT, XftTypeInteger, (bUseBold) ? XFT_WEIGHT_BOLD : XFT_WEIGHT_MEDIUM, NULL);

    if (xftMainWindowFontEn)
    XftFontClose (dpy, xftMainWindowFontEn);
    xftMainWindowFontEn =
    XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontEnName, XFT_SIZE, XftTypeDouble, (double) iMainWindowFontSize, XFT_ANTIALIAS, XftTypeBool, True, XFT_WEIGHT, XftTypeInteger, (bUseBold) ? XFT_WEIGHT_BOLD : XFT_WEIGHT_MEDIUM, NULL);

    if (xftVKWindowFont)
    XftFontClose (dpy, xftVKWindowFont);
    xftVKWindowFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iVKWindowFontSize, XFT_ANTIALIAS, XftTypeBool, True, NULL);

    if (xftDraw)
    XftDrawDestroy (xftDraw);
    xftDraw = XftDrawCreate (dpy, inputWindow, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)));

    setlocale (LC_CTYPE, "");
}
#else
void CreateFont (void)
{
    char          **missing_charsets;
    int             num_missing_charsets = 0;
    char           *default_string;
    char            strFont[256];

    //为了让界面能够正常显示，必须设置为中文模式
    if (strUserLocale[0])
    setlocale (LC_CTYPE, strUserLocale);

    if (bUseBold)
    sprintf (strFont, "-*-%s-bold-r-normal--%d-*-*-*-*-*-*-*,-*-%s-bold-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iMainWindowFontSize, strFontName, iMainWindowFontSize);
    else
    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iMainWindowFontSize, strFontName, iMainWindowFontSize);
    if (fontSetMainWindow)
    XFreeFontSet (dpy, fontSetMainWindow);
    fontSetMainWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
    fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
    iMainWindowFontSize = 14;
    if (bUseBold)
        sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iMainWindowFontSize, iMainWindowFontSize);
    else
        sprintf (strFont, "-*-*-bold-r-normal--%d-*-*-*-*-*-*-*,-*-*-bold-r-normal--%d-*-*-*-*-*-*-*", iMainWindowFontSize, iMainWindowFontSize);
    fontSetMainWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0)
        fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iFontSize, strFontName, iFontSize);
    if (fontSet)
    XFreeFontSet (dpy, fontSet);
    fontSet = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
    fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
    iFontSize = 16;
    sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iFontSize, iFontSize);
    fontSet = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0)
        fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iVKWindowFontSize, strFontName, iVKWindowFontSize);
    if (fontSetVKWindow)
    XFreeFontSet (dpy, fontSetVKWindow);
    fontSetVKWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
    fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
    iFontSize = 12;
    sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iFontSize, iFontSize);
    fontSetVKWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0)
        fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    setlocale (LC_CTYPE, "");
}
#endif

/*
 * 让指定的区域显示三维效果
 * effect:
 *  _3D_UPPER:显示为凸出效果
 *  _3D_LOWER:显示为凹下效果
 */
void Draw3DEffect (Window window, int x, int y, int width, int height, _3D_EFFECT effect)
{
    if (effect == _3D_UPPER) {
    XDrawLine (dpy, window, lightGC, x, y, x + width - 1, y);
    XDrawLine (dpy, window, lightGC, x, y, x, y + height - 1);
    XDrawLine (dpy, window, dimGC, x + width - 1, y, x + width - 1, y + height - 1);
    XDrawLine (dpy, window, dimGC, x, y + height - 1, x + width - 1, y + height - 1);
    }
    else if (effect == _3D_LOWER) {
    XDrawLine (dpy, window, lightGC, x, y, x + width - 1, y);
    XDrawLine (dpy, window, lightGC, x, y, x, y + height - 1);
    XDrawLine (dpy, window, dimGC, x + width - 1, y, x + width - 1, y + height - 1);
    XDrawLine (dpy, window, dimGC, x, y + height - 1, x + width - 1, y + height - 1);

    XDrawLine (dpy, window, dimGC, x + 1, y + 1, x + width - 2, y + 1);
    XDrawLine (dpy, window, dimGC, x + 1, y + 1, x + 1, y + height - 2);
    XDrawLine (dpy, window, lightGC, x + 1, y + height - 2, x + width - 2, y + height - 2);
    XDrawLine (dpy, window, lightGC, x + width - 2, y + 1, x + width - 2, y + height - 2);
    }
    else {          //   _3D_FLAT
    XDrawRectangle (dpy, window, dimGC, x, y, width - 1, height - 1);
    return;
    }
}

/*
 * 有关界面的消息都在这里处理
 *     有关tray重画的问题，此处的解决方案似乎很dirt
 */
void MyXEventHandler (XEvent * event)
{
    unsigned char   iPos;
    
    switch (event->type) {
#ifdef _ENABLE_TRAY
    /*case ReparentNotify:
    if ( event->xreparent.window == tray.window ) {
        if (ConnectIDGetState (connect_id) == IS_CHN)
        DrawTrayWindow (ACTIVE_ICON);
        else
        DrawTrayWindow (INACTIVE_ICON);
    }
    
    break;*/
#endif
    case ClientMessage:
    if ((event->xclient.message_type == about_protocol_atom) && ((Atom) event->xclient.data.l[0] == about_kill_atom)) {
        XUnmapWindow (dpy, aboutWindow);
        DrawMainWindow ();
    }
#ifdef _ENABLE_TRAY 
    tray_event_handler(event);
#endif
    break;
    case Expose:
#ifdef _DEBUG
    fprintf (stderr, "XEvent--Expose\n");
#endif
    if (event->xexpose.count > 0)
        break;
    if (event->xexpose.window == mainWindow)
        DrawMainWindow ();
    else if (event->xexpose.window == VKWindow)
        DrawVKWindow ();
    else if (event->xexpose.window == inputWindow)
        DrawInputWindow ();
#ifdef _ENABLE_TRAY
    else if (event->xexpose.window == tray.window) {
        tray_event_handler(event);
    }
#endif
    //added by yunfan
    else if (event->xexpose.window == aboutWindow)
        DrawAboutWindow ();
    //******************************
    break;
    case DestroyNotify:
#ifdef _ENABLE_TRAY 
    tray_event_handler(event);
#endif
    break;
    case ButtonPress:
    switch (event->xbutton.button) {
    case Button1:
        if (event->xbutton.window == inputWindow) {
        int x, y;
        x = event->xbutton.x;
        y = event->xbutton.y;
        MouseClick (&x, &y, inputWindow);
        DrawInputWindow ();
        }
        else if (event->xbutton.window == mainWindow) {
        if (IsInBox (event->xbutton.x, event->xbutton.y, 1, 1, 16, 17)) {
            iMainWindowX = event->xbutton.x;
            iMainWindowY = event->xbutton.y;
            if (!MouseClick (&iMainWindowX, &iMainWindowY, mainWindow)) {
            if (ConnectIDGetState (connect_id) != IS_CHN) {
                SetIMState ((ConnectIDGetState (connect_id) == IS_ENG) ? False : True);
                DrawMainWindow ();
            }
            else
                ChangeIMState (connect_id);
#ifdef _ENABLE_TRAY
            if (ConnectIDGetState (connect_id) == IS_CHN)
                DrawTrayWindow (ACTIVE_ICON, 0, 0, TRAY_ICON_HEIGHT, TRAY_ICON_WIDTH );
            else
                DrawTrayWindow (INACTIVE_ICON, 0, 0, TRAY_ICON_HEIGHT, TRAY_ICON_WIDTH );
#endif
            }
        }
        }
        else if (event->xbutton.window == VKWindow) {
        if (!VKMouseKey (event->xbutton.x, event->xbutton.y)) {
            iVKWindowX = event->xbutton.x;
            iVKWindowY = event->xbutton.y;
            MouseClick (&iVKWindowX, &iVKWindowY, VKWindow);
            DrawVKWindow ();
        }
        }
        //added by yunfan
        else if (event->xbutton.window == aboutWindow) {
        XUnmapWindow (dpy, aboutWindow);
        DrawMainWindow ();
        }
        //****************************
        SaveProfile ();
        break;
    }
    break;
    case ButtonRelease:
    if (event->xbutton.window == mainWindow) {
        switch (event->xbutton.button) {
        case Button1:
        iPos = 20;
        if (!bCompactMainWindow) {
            if (IsInBox (event->xbutton.x, event->xbutton.y, 20, 1, 38, 19))
            ChangePunc ();
            else if (IsInBox (event->xbutton.x, event->xbutton.y, 38, 1, 55, 19))
            ChangeCorner ();
            else if (IsInBox (event->xbutton.x, event->xbutton.y, 56, 1, 73, 19))
            ChangeLegend ();
            else if (IsInBox (event->xbutton.x, event->xbutton.y, 74, 1, 91, 19))
            ChangeGBKT ();

            iPos = 92;
        }
        if (IsInBox (event->xbutton.x, event->xbutton.y, iPos, 1, iPos + 10, 19))
            ChangeLock ();
        else {
            if (bShowVK || !bCompactMainWindow) {
            if (IsInBox (event->xbutton.x, event->xbutton.y, iPos + 11, 1, iPos + 30, 19))
                SwitchVK ();
            iPos += 20;
            }

            if (!bCorner && IsInBox (event->xbutton.x, event->xbutton.y, iPos + 11, 1, MAINWND_WIDTH, 19))
            SwitchIM (-1);
        }
        break;
        //added by yunfan
        case Button2:
        DisplayAboutWindow ();
        break;
        //********************
        case Button3:
        if (IsInBox (event->xbutton.x, event->xbutton.y, 1, 1, 16, 17))
            XUnmapWindow (dpy, mainWindow);
        else if (!bVK) {
            bCompactMainWindow = !bCompactMainWindow;
            SwitchIM (iIMIndex);
        }
        break;
        }
    }
#ifdef _ENABLE_TRAY
    else if (event->xbutton.window == tray.window) {
        switch (event->xbutton.button) {
        case Button1:
        if (ConnectIDGetState (connect_id) != IS_CHN) {
            SetIMState (True);
            DrawMainWindow ();

            DrawTrayWindow (ACTIVE_ICON, 0, 0, TRAY_ICON_HEIGHT, TRAY_ICON_WIDTH );
        }
        else {
            SetIMState (False);
            DrawTrayWindow (INACTIVE_ICON, 0, 0, TRAY_ICON_HEIGHT, TRAY_ICON_WIDTH );
            }

        break;
        case Button2:
        if (IsWindowVisible(mainWindow)) {
            bMainWindow_Hiden = True;
            XUnmapWindow(dpy,mainWindow);
        }
        else {
            bMainWindow_Hiden = False;
            DisplayMainWindow();
            DrawMainWindow();
        }
        break;
        }
    }
#endif
    break;
    case FocusIn:
    if (ConnectIDGetState (connect_id) == IS_CHN)
        DisplayInputWindow ();
    if (hideMainWindow != HM_HIDE)
        XMapRaised (dpy, mainWindow);
    break;
    default:
    break;
    }
}

/*
 * 判断鼠标点击处是否处于指定的区域内
 */
Bool IsInBox (int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (x0 >= x1 && x0 <= x2 && y0 >= y1 && y0 <= y2)
    return True;

    return False;
}

#ifdef _USE_XFT
int StringWidth (char *str, XftFont * font)
{
    XGlyphInfo      extents;

    if (!font)
    return 0;

    XftTextExtentsUtf8 (dpy, font, (FcChar8 *) str, strlen (str), &extents);
    if (font == xftMainWindowFont)
    return extents.width;

    return extents.xOff;
}

int FontHeight (XftFont * font)
{
    XGlyphInfo      extents;
    char            str[] = "Ay中";

    if (!font)
    return 0;

    XftTextExtentsUtf8 (dpy, font, (FcChar8 *) str, strlen (str), &extents);

    return extents.height;
}
#else
int StringWidth (char *str, XFontSet font)
{
    XRectangle      InkBox, LogicalBox;

    if (!font || !str)
    return 0;

    Xutf8TextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.width;
}

int FontHeight (XFontSet font)
{
    XRectangle      InkBox, LogicalBox;
    char            str[] = "Ay中";

    if (!font)
    return 0;

    Xutf8TextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.height;
}
#endif

/*
 * 以指定的颜色在窗口的指定位置输出字串
 */
#ifdef _USE_XFT
void OutputString (Window window, XftFont * font, char *str, int x, int y, XColor color)
{
    XftColor        xftColor;
    XRenderColor    renderColor;

    if (!font || !str)
    return;

    renderColor.red = color.red;
    renderColor.green = color.green;
    renderColor.blue = color.blue;
    renderColor.alpha = 0xFFFF;

    XftColorAllocValue (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &renderColor, &xftColor);
    XftDrawChange (xftDraw, window);
    XftDrawStringUtf8 (xftDraw, &xftColor, font, x, y, (FcChar8 *) str, strlen (str));

    XftColorFree (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &xftColor);
}
#else
void OutputString (Window window, XFontSet font, char *str, int x, int y, GC gc)
{
    if (!font || !str)
    return;

    Xutf8DrawString (dpy, window, font, gc, x, y, str, strlen (str));
}
#endif

Bool IsWindowVisible(Window window)
{
    XWindowAttributes attrs;

    XGetWindowAttributes( dpy, window, &attrs );

    if ( attrs.map_state==IsUnmapped)
    return False;

    return True;
}

/* *************下列函数取自于 rfinput-2.0 ************************ */
/*
 * 从xpm图形文件中图形数据填入到XImage变量中
 */
/*int FillImageByXPMData (XImage * pImage, char **apcData)
{
    int             iWidth, iHeight;
    int             iColorCount;
    int             iCharsPerPixel;
    char           *pcNext;
    int             i, j, k;
    StrPixel       *pPixels = NULL;
    char            acColor[3];
    XColor          AColor;
    int             iRV;

    iWidth = strtol (apcData[0], &pcNext, 10);
    iHeight = strtol (pcNext, &pcNext, 10);
    iColorCount = strtol (pcNext, &pcNext, 10);
    iCharsPerPixel = strtol (pcNext, &pcNext, 10);

    // Make the color array
    if (NULL == (pPixels = (StrPixel *) malloc (sizeof (StrPixel) * iColorCount)))
    goto ErrorExit;

    for (i = 0; i < iColorCount; i++)
    pPixels[i].pcStr = NULL;

    //File the Array
    for (i = 0; i < iColorCount; i++) {
    if (NULL == (pPixels[i].pcStr = (char *) malloc (iCharsPerPixel + 1)))
        goto ErrorExit;

    strncpy (pPixels[i].pcStr, apcData[i + 1], iCharsPerPixel);
    pPixels[i].pcStr[iCharsPerPixel] = 0;

    acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 6];
    acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 5];
    acColor[2] = 0;
    AColor.red = strtol (acColor, NULL, 16) * 0xffff / 0xff;
    acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 4];
    acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 3];
    acColor[2] = 0;
    AColor.green = strtol (acColor, NULL, 16) * 0xffff / 0xff;
    acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 2];
    acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 1];
    acColor[2] = 0;
    AColor.blue = strtol (acColor, NULL, 16) * 0xffff / 0xff;

    iRV = XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &AColor);
    pPixels[i].iPixel = AColor.pixel;
    }

    // Fill the image
    for (i = 0; i < iHeight; i++) {
    for (j = 0; j < iWidth; j++) {
        for (k = 0; k < iColorCount; k++) {
        if (!strncmp (pPixels[k].pcStr, apcData[i + 1 + iColorCount] + j * iCharsPerPixel, iCharsPerPixel)) {
            XPutPixel (pImage, j, i, pPixels[k].iPixel);
            break;
        }
        }
    }
    }
    // Free the Array
    if (NULL != pPixels) {
    for (i = 0; i < iColorCount; i++) {
        if (NULL != pPixels[i].pcStr)
        free (pPixels[i].pcStr);
    }
    free (pPixels);
    }
    return 1;
  ErrorExit:
    if (NULL != pPixels) {
    for (i = 0; i < iColorCount; i++) {
        if (NULL != pPixels[i].pcStr)
        free (pPixels[i].pcStr);
    }
    free (pPixels);
    }
    return 0;
}
*/

Bool MouseClick (int *x, int *y, Window window)
{
    XEvent          evtGrabbed;
    Bool            bMoved = False;

    // To motion the  window
    while (1) {
    XMaskEvent (dpy, PointerMotionMask | ButtonReleaseMask | ButtonPressMask, &evtGrabbed);
    if (ButtonRelease == evtGrabbed.xany.type) {
        if (Button1 == evtGrabbed.xbutton.button)
        break;
    }
    else if (MotionNotify == evtGrabbed.xany.type) {
        static Time     LastTime;

        if (evtGrabbed.xmotion.time - LastTime < 20)
        continue;

        XMoveWindow (dpy, window, evtGrabbed.xmotion.x_root - *x, evtGrabbed.xmotion.y_root - *y);
        XRaiseWindow (dpy, window);

        bMoved = True;
        LastTime = evtGrabbed.xmotion.time;
    }
    }

    *x = evtGrabbed.xmotion.x_root - *x;
    *y = evtGrabbed.xmotion.y_root - *y;

    return bMoved;
}
