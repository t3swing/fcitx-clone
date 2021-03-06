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

#include "InputWindow.h"

#include <string.h>
#include <ctype.h>
#include "version.h"
#include <time.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#include <iconv.h>
#endif

#include "ui.h"
#include "ime.h"

#include "tools.h"

#include "DBus.h"

//下面的顺序不能颠倒
#include "next.xpm"
#include "prev.xpm"

Window          inputWindow;
int             iInputWindowX = INPUTWND_STARTX;
int             iInputWindowY = INPUTWND_STARTY;

uint            iInputWindowHeight = INPUTWND_HEIGHT;
int		iFixedInputWindowWidth = 0;
uint            iInputWindowWidth = INPUTWND_WIDTH;
uint            iInputWindowUpWidth = INPUTWND_WIDTH;
uint            iInputWindowDownWidth = INPUTWND_WIDTH;

uint            INPUTWND_START_POS_UP = 8;

MESSAGE_COLOR   messageColor[MESSAGE_TYPE_COUNT] = {
    {NULL, {0, 255 << 8, 0, 0}},
    {NULL, {0, 0, 0, 255 << 8}},
    {NULL, {0, 200 << 8, 0, 0}},
    {NULL, {0, 0, 150 << 8, 100 << 8}},
    {NULL, {0, 0, 0, 255 << 8}},
    {NULL, {0, 100 << 8, 100 << 8, 255 << 8}},
    {NULL, {0, 0, 0, 0}}
};

MESSAGE_COLOR   inputWindowLineColor = { NULL, {0, 90 << 8, 160 << 8, 90 << 8} };	//输入框中的线条色
XColor          colorArrow = { 0, 255 << 8, 0, 0 };	//箭头的颜色

WINDOW_COLOR    inputWindowColor = { NULL, NULL, {0, 240 << 8, 255 << 8, 240 << 8} };
MESSAGE_COLOR   cursorColor = { NULL, {0, 92 << 8, 210 << 8, 131 << 8} };

// *************************************************************
MESSAGE         messageUp[32];	//输入条上部分显示的内容
uint            uMessageUp = 0;

// *************************************************************
MESSAGE         messageDown[32];	//输入条下部分显示的内容
uint            uMessageDown = 0;

XImage         *pNext = NULL, *pPrev = NULL;

Bool            bShowPrev = False;
Bool            bShowNext = False;
Bool            bTrackCursor = True;
Bool            bCenterInputWindow = True;
Bool            bShowInputWindowTriggering = True;
Bool		bIsDisplaying = False;

int             iCursorPos = 0;
Bool            bShowCursor = False;

_3D_EFFECT      _3DEffectInputWindow = _3D_LOWER;

//这两个变量是GTK+ OverTheSpot光标跟随的临时解决方案
/* Issue 11: piaoairy: 为适应generic_config_integer(), 改INT8 为int */
int		iOffsetX = 0;
int		iOffsetY = 16;

extern Display *dpy;
extern int      iScreen;

#ifdef _USE_XFT
extern XftFont *xftFont;
extern XftFont *xftFontEn;
#else
extern XFontSet fontSet;
#endif

extern GC       dimGC;
extern GC       lightGC;

extern Bool     bUseGBKT;

//计算速度
extern Bool     bStartRecordType;
extern Bool     bShowUserSpeed;
extern Bool     bShowVersion;
extern time_t   timeStart;
extern uint     iHZInputed;
extern Bool	bUseDBus;

extern CARD16	connect_id;

extern int	iClientCursorX;
extern int	iClientCursorY;

#ifdef _DEBUG
extern char     strUserLocale[];
extern char     strXModifiers[];
#endif

#ifdef _ENABLE_RECORDING
extern FILE	*fpRecord;
extern Bool	bRecording;
#endif

Bool CreateInputWindow (void)
{
    XSetWindowAttributes	attrib;
    unsigned long	attribmask;
    int			iBackPixel;
    XTextProperty	tp;
    char		strWindowName[]="Fcitx Input Window";

    //根据窗口的背景色来设置XPM的色彩
    sprintf (strXPMBackColor, ". c #%02x%02x%02x", inputWindowColor.backColor.red >> 8, inputWindowColor.backColor.green >> 8, inputWindowColor.backColor.blue >> 8);
    //设置箭头的颜色
    sprintf (strXPMColor, "# c #%02x%02x%02x", colorArrow.red >> 8, colorArrow.green >> 8, colorArrow.blue >> 8);

    CalculateInputWindowHeight ();

    attrib.override_redirect = True;
    attribmask = CWOverrideRedirect;

    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.backColor)))
	iBackPixel = inputWindowColor.backColor.pixel;
    else
	iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));

    inputWindow = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), iInputWindowX, iInputWindowY, INPUTWND_WIDTH, iInputWindowHeight, 0, WhitePixel (dpy, DefaultScreen (dpy)), iBackPixel);
    if (inputWindow == (Window) NULL)
	return False;

    XChangeWindowAttributes (dpy, inputWindow, attribmask, &attrib);
    XSelectInput (dpy, inputWindow, ButtonPressMask | ButtonReleaseMask  | PointerMotionMask | ExposureMask);

    //Set the name of the window
    tp.value = (void *)strWindowName;
    tp.encoding = XA_STRING;
    tp.format = 16;
    tp.nitems = strlen(strWindowName);
    XSetWMName (dpy, inputWindow, &tp);

    InitInputWindowColor ();

    return True;
}

/*
 * 根据字体的大小来调整窗口的高度
 */
void CalculateInputWindowHeight (void)
{
    int             iHeight;

#ifdef _USE_XFT
    iHeight = FontHeight (xftFont);
#else
    iHeight = FontHeight (fontSet);
#endif

    iInputWindowHeight = iHeight * 2 + iHeight / 2 + 8;
}

void DisplayInputWindow (void)
{
#ifdef _DEBUG
    fprintf (stderr, "DISPLAY InputWindow\n");
#endif
    CalInputWindow();
    MoveInputWindow(connect_id);
    if (uMessageUp || uMessageDown)
	{
		if (!bUseDBus)
			XMapRaised (dpy, inputWindow);
#ifdef _ENABLE_DBUS
		else
			updateMessages();
#endif
	}
}

void InitInputWindowColor (void)
{
    XGCValues       values;
    int             iPixel;
    int             i;

    for (i = 0; i < MESSAGE_TYPE_COUNT; i++) {
	if (messageColor[i].gc)
	    XFreeGC (dpy, messageColor[i].gc);
	messageColor[i].gc = XCreateGC (dpy, inputWindow, 0, &values);
	if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(messageColor[i].color)))
	    iPixel = messageColor[i].color.pixel;
	else
	    iPixel = WhitePixel (dpy, DefaultScreen (dpy));
	XSetForeground (dpy, messageColor[i].gc, iPixel);
    }

    if (inputWindowLineColor.gc)
	XFreeGC (dpy, inputWindowLineColor.gc);
    inputWindowLineColor.gc = XCreateGC (dpy, inputWindow, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowLineColor.color)))
	iPixel = inputWindowLineColor.color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowLineColor.gc, iPixel);

    cursorColor.color.red = cursorColor.color.red ^ inputWindowColor.backColor.red;
    cursorColor.color.green = cursorColor.color.green ^ inputWindowColor.backColor.green;
    cursorColor.color.blue = cursorColor.color.blue ^ inputWindowColor.backColor.blue;

    if (cursorColor.gc)
	XFreeGC (dpy, cursorColor.gc);
    cursorColor.gc = XCreateGC (dpy, inputWindow, 0, &values);
    //为了画绿色光标
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &cursorColor.color))
	iPixel = cursorColor.color.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));

    XSetForeground (dpy, cursorColor.gc, iPixel);
    XSetFunction (dpy, cursorColor.gc, GXxor);
}

void ResetInputWindow (void)
{
    uMessageDown = 0;
    uMessageUp = 0;
}

void CalInputWindow (void)
{
    int             i;

#ifdef _USE_XFT
    char            strTemp[MESSAGE_MAX_LENGTH];
    char           *p1, *p2;
    Bool            bEn;
#endif

#ifdef _DEBUG
    fprintf (stderr, "CAL InputWindow\n");
#endif

    if (!uMessageUp && !uMessageDown) {
	bShowCursor = False;
	if (bShowVersion) {
	    uMessageUp = 1;
	    strcpy (messageUp[0].strMsg, "FCITX ");
	    strcat (messageUp[0].strMsg, FCITX_VERSION);
	    messageUp[0].type = MSG_TIPS;
	}

#ifdef _DEBUG
	uMessageDown = 1;
	strcpy (messageDown[0].strMsg, strUserLocale);
	strcat (messageDown[0].strMsg, " - ");
	strcat (messageDown[0].strMsg, strXModifiers);
	messageDown[0].type = MSG_CODE;
#else
	//显示打字速度
	if (bStartRecordType && bShowUserSpeed) {
	    double          timePassed;

	    timePassed = difftime (time (NULL), timeStart);
	    if (((int) timePassed) == 0)
		timePassed = 1.0;

	    uMessageDown = 6;
	    strcpy (messageDown[0].strMsg, "打字速度：");
	    messageDown[0].type = MSG_OTHER;
	    sprintf (messageDown[1].strMsg, "%d", (int) (iHZInputed * 60 / timePassed));
	    messageDown[1].type = MSG_CODE;
	    strcpy (messageDown[2].strMsg, "/分  用时：");
	    messageDown[2].type = MSG_OTHER;
	    sprintf (messageDown[3].strMsg, "%d", (int) timePassed);
	    messageDown[3].type = MSG_CODE;
	    strcpy (messageDown[4].strMsg, "秒  字数：");
	    messageDown[4].type = MSG_OTHER;
	    sprintf (messageDown[5].strMsg, "%u", iHZInputed);
	    messageDown[5].type = MSG_CODE;
	}
#endif
    }

#ifdef _ENABLE_RECORDING
    if ( bRecording && fpRecord ) {
	if ( uMessageUp > 0 ) {
	    if ( messageUp[uMessageUp-1].type==MSG_RECORDING )
	        uMessageUp --;
	}
	strcpy(messageUp[uMessageUp].strMsg,"  [记录模式]");
	messageUp[uMessageUp].type = MSG_RECORDING;
	uMessageUp ++;
    }
#endif

    iInputWindowUpWidth = 2 * INPUTWND_START_POS_UP + 1;
    for (i = 0; i < uMessageUp; i++) {
#ifdef _USE_XFT
	p1 = messageUp[i].strMsg;
	while (*p1) {
	    p2 = strTemp;
	    if (isprint (*p1))	//使用中文字体
		bEn = True;
	    else {
		*p2++ = *p1++;
		*p2++ = *p1++;
		bEn = False;
	    }
	    while (*p1) {
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		    *p2++ = *p1++;
		}
		else {
		    if (bEn)
			break;
		    *p2++ = *p1++;
		    *p2++ = *p1++;
		}
	    }
	    *p2 = '\0';

	    iInputWindowUpWidth += StringWidth (strTemp, (bEn) ? xftFontEn : xftFont);
	}
#else
	iInputWindowUpWidth += StringWidth (messageUp[i].strMsg, fontSet);
#endif
    }

    if (bShowPrev)
	iInputWindowUpWidth += 16;
    else if (bShowNext)
	iInputWindowUpWidth += 8;

    iInputWindowDownWidth = 2 * INPUTWND_START_POS_DOWN + 1;
    for (i = 0; i < uMessageDown; i++) {
#ifdef _USE_XFT
	p1 = messageDown[i].strMsg;
	while (*p1) {
	    p2 = strTemp;
	    if (isprint (*p1))	//使用中文字体
		bEn = True;
	    else {
		*p2++ = *p1++;
		*p2++ = *p1++;
		bEn = False;
	    }
	    while (*p1) {
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		    *p2++ = *p1++;
		}
		else {
		    if (bEn)
			break;
		    *p2++ = *p1++;
		    *p2++ = *p1++;
		}
	    }
	    *p2 = '\0';

	    iInputWindowDownWidth += StringWidth (strTemp, (bEn) ? xftFontEn : xftFont);
	}
#else
	iInputWindowDownWidth += StringWidth (messageDown[i].strMsg, fontSet);
#endif
    }

    if (iInputWindowUpWidth < iInputWindowDownWidth)
	iInputWindowWidth = iInputWindowDownWidth;
    else
	iInputWindowWidth = iInputWindowUpWidth;

    if (iInputWindowWidth < INPUTWND_WIDTH)
	iInputWindowWidth = INPUTWND_WIDTH;
    if (iFixedInputWindowWidth) {
	if (iInputWindowWidth < iFixedInputWindowWidth)
	    iInputWindowWidth = iFixedInputWindowWidth;
    }
}

void DrawInputWindow(void)
{
    XImage         *mask;
    XpmAttributes   attrib;
    int	i;

    XClearArea (dpy, inputWindow, 2, 2, iInputWindowWidth - 2, iInputWindowHeight / 2 - 2, False);
    XClearArea (dpy, inputWindow, 2, iInputWindowHeight / 2 + 1, iInputWindowWidth - 2, iInputWindowHeight / 2 - 2, False);

    DisplayMessageUp ();
    DisplayMessageDown ();

    //**************************
    attrib.valuemask = 0;

    if (_3DEffectInputWindow == _3D_UPPER)
	Draw3DEffect (inputWindow, 1, 1, iInputWindowWidth - 2, iInputWindowHeight - 2, _3D_UPPER);
    else if (_3DEffectInputWindow == _3D_LOWER)
	Draw3DEffect (inputWindow, 0, 0, iInputWindowWidth, iInputWindowHeight, _3D_LOWER);

    XDrawRectangle (dpy, inputWindow, inputWindowLineColor.gc, 0, 0, iInputWindowWidth - 1, iInputWindowHeight - 1);
    if (_3DEffectInputWindow == _3D_LOWER)
	XDrawLine (dpy, inputWindow, lightGC, 2 + 5, iInputWindowHeight / 2 - 1, iInputWindowWidth - 2 - 5, iInputWindowHeight / 2 - 1);
    else if (_3DEffectInputWindow == _3D_UPPER)
	XDrawLine (dpy, inputWindow, dimGC, 2 + 5, iInputWindowHeight / 2 - 1, iInputWindowWidth - 2 - 5, iInputWindowHeight / 2 - 1);
    XDrawLine (dpy, inputWindow, inputWindowLineColor.gc, 2 + 5, iInputWindowHeight / 2, iInputWindowWidth - 2 - 5, iInputWindowHeight / 2);
    if (_3DEffectInputWindow == _3D_LOWER)
	XDrawLine (dpy, inputWindow, dimGC, 2 + 5, iInputWindowHeight / 2 + 1, iInputWindowWidth - 2 - 5, iInputWindowHeight / 2 + 1);
    else if (_3DEffectInputWindow == _3D_UPPER)
	XDrawLine (dpy, inputWindow, lightGC, 2 + 5, iInputWindowHeight / 2 + 1, iInputWindowWidth - 2 - 5, iInputWindowHeight / 2 + 1);

    if (bShowPrev) {
	if (!pPrev) {
	    i = XpmCreateImageFromData (dpy, xpm_prev, &pPrev, &mask, &attrib);
	    if (i != XpmSuccess)
		fprintf (stderr, "Failed to read xpm file: Prev\n");
	}
	XPutImage (dpy, inputWindow, inputWindowColor.foreGC, pPrev, 0, 0, iInputWindowWidth - 20, (iInputWindowHeight / 2 - 12) / 2, 6, 12);
    }
    if (bShowNext) {
	if (!pNext) {
	    i = XpmCreateImageFromData (dpy, xpm_next, &pNext, &mask, &attrib);
	    if (i != XpmSuccess)
		fprintf (stderr, "Failed to read xpm file: Next\n");
	}
	XPutImage (dpy, inputWindow, inputWindowColor.foreGC, pNext, 0, 0, iInputWindowWidth - 10, (iInputWindowHeight / 2 - 12) / 2, 6, 12);
    }
}

/*
 * 在输入窗的上部分显示信息
 */
void DisplayMessageUp (void)
{
    int             i = 0;
    int             iPos = 0;
    int             iCursorPixPos = 0;
    int             iChar;
    char            strText[MESSAGE_MAX_LENGTH];

#ifdef _USE_XFT
    char            strTemp[MESSAGE_MAX_LENGTH];
    char           *p1, *p2;
    Bool            bEn;
#endif

    char           *strGBKT;

    iInputWindowUpWidth = INPUTWND_START_POS_UP;
    iChar = iCursorPos;

    for (i = 0; i < uMessageUp; i++) {
#ifdef _USE_XFT
	p1 = messageUp[i].strMsg;
	iPos = 0;
	while (*p1) {
	    p2 = strTemp;
	    if (isprint (*p1))	//使用中文字体
		bEn = True;
	    else {
		*p2++ = *p1++;
		*p2++ = *p1++;
		bEn = False;
	    }
	    while (*p1) {
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		    *p2++ = *p1++;
		}
		else {
		    if (bEn)
			break;
		    *p2++ = *p1++;
		    *p2++ = *p1++;
		}
	    }
	    *p2 = '\0';

	    strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition (strTemp) : strTemp;

#ifdef _ENABLE_RECORDING
	    OutputString (inputWindow, (bEn) ? xftFontEn : xftFont, strGBKT, iInputWindowUpWidth + iPos, (2 * iInputWindowHeight - 1) / 5, messageColor[(messageUp[i].type==MSG_RECORDING)? MSG_TIPS:messageUp[i].type].color);
#else
	    OutputString (inputWindow, (bEn) ? xftFontEn : xftFont, strGBKT, iInputWindowUpWidth + iPos, (2 * iInputWindowHeight - 1) / 5, messageColor[messageUp[i].type].color);
#endif
	    iPos += StringWidth (strGBKT, (bEn) ? xftFontEn : xftFont);

	    if (bUseGBKT)
		free (strGBKT);
	}
#else
	strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition (messageUp[i].strMsg) : messageUp[i].strMsg;

#ifdef _ENABLE_RECORDING
	OutputString (inputWindow, fontSet, strGBKT, iInputWindowUpWidth, (2 * iInputWindowHeight - 1) / 5, messageColor[(messageUp[i].type==MSG_RECORDING)? MSG_TIPS:messageUp[i].type].gc);
#else
	OutputString (inputWindow, fontSet, strGBKT, iInputWindowUpWidth, (2 * iInputWindowHeight - 1) / 5, messageColor[messageUp[i].type].gc);
#endif
	iPos = StringWidth (strGBKT, fontSet);

	if (bUseGBKT)
	    free (strGBKT);
#endif
	iInputWindowUpWidth += iPos;

	if (bShowCursor && iChar) {
	    if (strlen (messageUp[i].strMsg) > iChar) {
		strncpy (strText, messageUp[i].strMsg, iChar);
		strText[iChar] = '\0';
#ifdef _USE_XFT
		p1 = strText;
		while (*p1) {
		    p2 = strTemp;
		    if (isprint (*p1))	//使用中文字体
			bEn = True;
		    else {
			*p2++ = *p1++;
			*p2++ = *p1++;
			bEn = False;
		    }
		    while (*p1) {
			if (isprint (*p1)) {
			    if (!bEn)
				break;
			    *p2++ = *p1++;
			}
			else {
			    if (bEn)
				break;
			    *p2++ = *p1++;
			    *p2++ = *p1++;
			}
		    }
		    *p2 = '\0';

		    iCursorPixPos += StringWidth (strTemp, (bEn) ? xftFontEn : xftFont);
		}
#else
		iCursorPixPos += StringWidth (strText, fontSet);
#endif
		iChar = 0;
	    }
	    else {
		iCursorPixPos += iPos;
		iChar -= strlen (messageUp[i].strMsg);
	    }
	}
    }

    if (bShowCursor)
	DrawCursor (INPUTWND_START_POS_UP + iCursorPixPos);
}

/*
 * 在输入窗的下部分显示信息
 */
void DisplayMessageDown (void)
{
    uint            i;
    uint            iPos;

#ifdef _USE_XFT
    char            strTemp[MESSAGE_MAX_LENGTH];
    char           *p1, *p2;
    Bool            bEn;
#endif

    char           *strGBKT;

    iPos = INPUTWND_START_POS_DOWN;
    for (i = 0; i < uMessageDown; i++) {
	//借用iInputWindowDownWidth作为一个临时变量

#ifdef _USE_XFT
	p1 = messageDown[i].strMsg;

	while (*p1) {
	    p2 = strTemp;
	    if (isprint (*p1))	//使用中文字体
		bEn = True;
	    else {
		*p2++ = *p1++;
		*p2++ = *p1++;
		bEn = False;
	    }
	    while (*p1) {
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		    *p2++ = *p1++;
		}
		else {
		    if (bEn)
			break;
		    *p2++ = *p1++;
		    *p2++ = *p1++;
		}
	    }
	    *p2 = '\0';

	    strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition (strTemp) : strTemp;

	    iInputWindowDownWidth = StringWidth (strGBKT, (bEn) ? xftFontEn : xftFont);
	    OutputString (inputWindow, (bEn) ? xftFontEn : xftFont, strGBKT, iPos, (9 * iInputWindowHeight - 12) / 10, messageColor[messageDown[i].type].color);
	    iPos += iInputWindowDownWidth;

	    if (bUseGBKT)
		free (strGBKT);
	}
#else
	strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition (messageDown[i].strMsg) : messageDown[i].strMsg;

	iInputWindowDownWidth = StringWidth (strGBKT, fontSet);
	OutputString (inputWindow, fontSet, strGBKT, iPos, (9 * iInputWindowHeight - 12) / 10, messageColor[messageDown[i].type].gc);
	iPos += iInputWindowDownWidth;

	if (bUseGBKT)
	    free (strGBKT);
#endif
    }
}

void DrawCursor (int iPos)
{
    XDrawLine (dpy, inputWindow, cursorColor.gc, iPos, 8, iPos, iInputWindowHeight / 2 - 4);
    XDrawLine (dpy, inputWindow, cursorColor.gc, iPos + 1, 8, iPos + 1, iInputWindowHeight / 2 - 4);
}

void MoveInputWindow(CARD16 connect_id)
{
    if (ConnectIDGetTrackCursor (connect_id) && bTrackCursor) {
        int iTempInputWindowX, iTempInputWindowY;

	if (iClientCursorX < 0)
	    iTempInputWindowX = 0;
	else
	    iTempInputWindowX = iClientCursorX + iOffsetX;

	if (iClientCursorY < 0)
	    iTempInputWindowY = 0;
	else
	    iTempInputWindowY = iClientCursorY + iOffsetY;

	if ((iTempInputWindowX + iInputWindowWidth) > DisplayWidth (dpy, iScreen))
	    iTempInputWindowX = DisplayWidth (dpy, iScreen) - iInputWindowWidth;

	if ((iTempInputWindowY + iInputWindowHeight) > DisplayHeight (dpy, iScreen)) {
	    if ( iTempInputWindowY > DisplayHeight (dpy, iScreen) )
	        iTempInputWindowY = DisplayHeight (dpy, iScreen) - 2 * iInputWindowHeight;
	    else
	        iTempInputWindowY = iTempInputWindowY - 2 * iInputWindowHeight;
	}

	if (!bUseDBus)
	    XMoveResizeWindow (dpy, inputWindow, iTempInputWindowX, iTempInputWindowY, iInputWindowWidth, iInputWindowHeight);
#ifdef _ENABLE_DBUS
	else
	{
	if (iClientCursorX < 0)
	    iTempInputWindowX = 0;
	else
	    iTempInputWindowX = iClientCursorX + iOffsetX;

	if (iClientCursorY < 0)
	    iTempInputWindowY = 0;
	else
	    iTempInputWindowY = iClientCursorY + iOffsetY;

	    KIMUpdateSpotLocation(iTempInputWindowX, iTempInputWindowY);
	}
#endif
	ConnectIDSetPos (connect_id, iTempInputWindowX - iOffsetX, iTempInputWindowY - iOffsetY);
    }
    else {
	position * pos = ConnectIDGetPos(connect_id);
	if (bCenterInputWindow) {
	    iInputWindowX = (DisplayWidth (dpy, iScreen) - iInputWindowWidth) / 2;
	    if (iInputWindowX < 0)
		iInputWindowX = 0;
	}
	else
	    iInputWindowX = pos ? pos->x : iInputWindowX;

	if (!bUseDBus)
	    XMoveResizeWindow (dpy, inputWindow, iInputWindowX, pos ? pos->y : iInputWindowY, iInputWindowWidth, iInputWindowHeight);
#ifdef _ENABLE_DBUS
	else
	    KIMUpdateSpotLocation(iInputWindowX, pos ? pos->y : iInputWindowY);
#endif
    }
    
}

void CloseInputWindow()
{
	XUnmapWindow (dpy, inputWindow);
#ifdef _ENABLE_DBUS
	if (bUseDBus)
	{
		KIMShowAux(False);
		KIMShowPreedit(False);
		KIMShowLookupTable(False);
	}
#endif
}

