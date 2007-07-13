#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui.h"

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

#include "MyErrorsHandlers.h"
#include "MainWindow.h"
#include "InputWindow.h"
#include "IC.h"
#include "tools.h"
#include "sp.h"

Display        *dpy;
int             iScreen;

#ifdef _USE_XFT
XftFont        *xftFont;
XftDraw        *xftDraw;
XftFont        *xftMainWindowFont;
Bool            bUseAA = True;
int		iDefaultWidth = 0;
#else
XFontSet        fontSet;
XFontSet        fontSetMainWindow;
char		strUserLocale[50]="zh_CN.gb2312";
#endif

iconv_t         convUTF8;	//��ֻ����XFT���UTF8�����ִ�������ñ���������UTF8֧��

int             iFontSize = 16;
int             iMainWindowFontSize = 12;

GC              dimGC;
GC              lightGC;

Bool            bIsUtf8 = False;

char            strFontName[100] = "*";

extern Window   mainWindow;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern Window   inputWindow;
extern int      iInputWindowX;
extern int      iInputWindowY;
extern WINDOW_COLOR mainWindowColor;
extern WINDOW_COLOR inputWindowColor;
extern Bool     bIsResizingInputWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern IC      *CurrentIC;
extern int      MAINWND_WIDTH;
extern Bool     bCompactMainWindow;
extern INT8     iIMIndex;
extern CARD16	connect_id;

Bool InitX (void)
{
    if ((dpy = XOpenDisplay ((char *) NULL)) == NULL) {
	fprintf (stderr, "Error: FCITX can only run under X\n");
	return False;
    }

    SetMyXErrorHandler ();
    iScreen = DefaultScreen (dpy);

    convUTF8 = iconv_open ("UTF-8", "GBK");

    return True;
}

void InitGC (Window window)
{
    XGCValues       values;
    XColor          color;
    int             iPixel;

    lightGC = XCreateGC (dpy, window, 0, &values);
    color.red = LIGHT_COLOR;
    color.green = LIGHT_COLOR;
    color.blue = LIGHT_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
	iPixel = color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, lightGC, iPixel);

    dimGC = XCreateGC (dpy, window, 0, &values);
    color.red = DIM_COLOR;
    color.green = DIM_COLOR;
    color.blue = DIM_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
	iPixel = color.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, dimGC, iPixel);

    mainWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowColor.backColor)))
	iPixel = mainWindowColor.backColor.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, mainWindowColor.backGC, iPixel);

    inputWindowColor.foreGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.foreColor)))
	iPixel = inputWindowColor.foreColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.foreGC, iPixel);

    inputWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.backColor)))
	iPixel = inputWindowColor.backColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.backGC, iPixel);
}

/*
 * ��localeΪUTF-8ʱ��setlocale(LC_CTYPE,"")�ᵼ�³������
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
	
	//���鷢�֣���localeΪzh_CN.UTF-8ʱ��setlocale����������zh_CN.UTF-8��������zh_CN.utf-8
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
    xftFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, NULL);
    xftMainWindowFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iMainWindowFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, XFT_WEIGHT, XftTypeInteger, XFT_WEIGHT_BOLD, NULL);

    xftDraw = XftDrawCreate (dpy, inputWindow, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)));

    iDefaultWidth = StringWidth("��", xftFont)/2 - 2;
}
#else
void CreateFont (void)
{
    char          **missing_charsets;
    int             num_missing_charsets = 0;
    char           *default_string;
    char            strFont[256];

    if ( strUserLocale[0] )
    	setlocale (LC_CTYPE, strUserLocale);
    else
    	setlocale(LC_CTYPE,"");
    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", strFontName, iMainWindowFontSize, iMainWindowFontSize);
    fontSetMainWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0)
	fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\n", strFont);

    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", strFontName, iFontSize, iFontSize);
    fontSet = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0)
    	fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\n", strFont);

    setlocale(LC_CTYPE,"");
/*
   SetLocale();*/
}
#endif

/*
 * ��ָ����������ʾ��άЧ��
 * effect:
 *	_3D_UPPER:��ʾΪ͹��Ч��
 *	_3D_LOWER:��ʾΪ����Ч��
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
    else {			//   _3D_FLAT
	XDrawRectangle (dpy, window, dimGC, x, y, width - 1, height - 1);
	return;
    }
}

/*
 * �йؽ������Ϣ�������ﴦ��
 */
void MyXEventHandler (XEvent * event)
{
    switch (event->type) {
    case Expose:
	if (event->xexpose.count > 0)
	    break;
	if (event->xexpose.window == mainWindow) {
	    DisplayMainWindow ();
	    break;
	}
	if (event->xexpose.window == inputWindow) {
	    if (!bIsResizingInputWindow) {
		DisplayInputWindow ();
		bIsResizingInputWindow = False;
	    }
	}
	break;
    case DestroyNotify:
	break;
    case ButtonPress:
	switch (event->xbutton.button) {
	case Button1:
	    if (event->xbutton.window == inputWindow) {
		iInputWindowX = event->xbutton.x;
		iInputWindowY = event->xbutton.y;
		MouseClick (&iInputWindowX, &iInputWindowY, 1);
		SaveProfile ();
	    }
	    else if (event->xbutton.window == mainWindow) {
		if (IsInBox (event->xbutton.x, event->xbutton.y, 1, 1, 16, 17)) {
		    iMainWindowX = event->xbutton.x;
		    iMainWindowY = event->xbutton.y;
		    MouseClick (&iMainWindowX, &iMainWindowY, 0);
		}
	    }
	    break;
	}
	break;
    case ButtonRelease:
	switch (event->xbutton.button) {
	case Button1:
	    if (event->xbutton.window == mainWindow) {
		INT8            iPos;

		iPos = 20;
		if (!bCompactMainWindow) {
		    if (IsInBox (event->xbutton.x, event->xbutton.y, 20, 1, 38, 19))
			ChangePunc ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 38, 1, 55, 19))
			ChangeCorner ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 56, 1, 73, 19))
			ChangeGBK ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 74, 1, 91, 19))
			ChangeLegend ();
		    iPos = 92;
		}
		if (IsInBox (event->xbutton.x, event->xbutton.y, iPos, 1, iPos + 10, 19))
		    ChangeLock ();
		else if (IsInBox (event->xbutton.x, event->xbutton.y, iPos + 11, 1, MAINWND_WIDTH, 19))
		    SwitchIM (-1);
	    }
	    break;
	case Button3:
	    bCompactMainWindow = !bCompactMainWindow;
	    SwitchIM (iIMIndex);
	}

	break;
    case FocusIn:
	if (ConnectIDGetState(connect_id) == IS_CHN)
		DisplayInputWindow ();
	if (hideMainWindow != HM_HIDE)
	    XMapRaised (dpy, mainWindow);
	break;
    default:
	break;
    }
}

/*
 * �ж���������Ƿ���ָ����������
 */
Bool IsInBox (int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (x0 >= x1 && x0 <= x2 && y0 >= y1 && y0 <= y2)
	return True;
    else
	return False;
}

#ifdef _USE_XFT
int StringWidth (char *str, XftFont * font)
{
    XGlyphInfo      extents;
    char	    str1[100];
    char           *ps;
    size_t          l1, l2;
    int		il;


    if (!font)
	return 0;

    il = l1 = strlen (str);
    l2 = 99;
    ps = str1;

    l1 = iconv (convUTF8, &str, &l1, &ps, &l2);
    *ps = '\0';
    XftTextExtentsUtf8 (dpy, font, (FcChar8*)str1, strlen (str1), &extents);
    if ( font!=xftFont)
    	return extents.width;

    return ((extents.width<(il*iDefaultWidth))?(il*iDefaultWidth):extents.width);
}

int FontHeight (XftFont * font)
{
    XGlyphInfo      extents;
    char            str1[] = "Ay��";
    char            str2[10];
    char           *ps1, *ps2;
    size_t          l1, l2;

    if (!font)
	return 0;

    l1 = strlen (str1);
    l2 = 9;
    ps2 = str2;
    ps1 = str1;

    l1 = iconv (convUTF8, &ps1, &l1, &ps2, &l2);
    *ps2='\0';

    XftTextExtentsUtf8 (dpy, font, (FcChar8 *)str2, strlen (str2), &extents);

    return extents.height;
}
#else
int StringWidth (char *str, XFontSet font)
{
    XRectangle      InkBox, LogicalBox;

    if (!font || !str)
	return 0;

    XmbTextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.width;
}

int FontHeight (XFontSet font)
{
    XRectangle      InkBox, LogicalBox;
    char            str[] = "Ay��";

    if (!font)
	return 0;

    XmbTextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.height;
}
#endif

/*
 * ��ָ������ɫ�ڴ��ڵ�ָ��λ������ִ�
 */
#ifdef _USE_XFT
void OutputString (Window window, XftFont * font, char *str, int x, int y, XColor color)
{
    char            strOutput[100] = "\0";	//�ó���Ӧ�ù�����
    size_t          l1, l2;
    char           *ps;
    XftColor        xftColor;
    XRenderColor    renderColor;

    if (!font || !str)
	return;

    //ʹ��UTF8��
    l1 = strlen (str);
    l2 = 99;
    ps = strOutput;

    l1 = iconv (convUTF8, (char **) (&str), &l1, &ps, &l2);
    *ps='\0';

    renderColor.red = color.red;
    renderColor.green = color.green;
    renderColor.blue = color.blue;
    renderColor.alpha = 0xFFFF;

    XftColorAllocValue (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &renderColor, &xftColor);
    XftDrawChange (xftDraw, window);
    XftDrawStringUtf8 (xftDraw, &xftColor, font, x, y, (FcChar8 *)strOutput, strlen (strOutput));

    XftColorFree (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &xftColor);
}
#else
void OutputString (Window window, XFontSet font, char *str, int x, int y, GC gc)
{
    if (!font || !str)
	return;

    XmbDrawString (dpy, window, font, gc, x, y, str, strlen (str));
}
#endif

/* *************���к���ȡ���� rfinput-2.0 ************************ */
/*
 * ��xpmͼ���ļ���ͼ���������뵽XImage������
 */
int FillImageByXPMData (XImage * pImage, char **apcData)
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

XPoint MouseClick (int *x, int *y, int iWnd)
{
    XEvent          evtGrabbed;
    XPoint          point;

    // To motion the  window
    while (1) {
	XMaskEvent (dpy, PointerMotionMask | ButtonReleaseMask | ButtonPressMask, &evtGrabbed);
	if (ButtonRelease == evtGrabbed.xany.type) {
	    if (Button1 == evtGrabbed.xbutton.button) {
		break;
	    }
	}
	else if (MotionNotify == evtGrabbed.xany.type) {
	    static Time     LastTime;

	    if (evtGrabbed.xmotion.time - LastTime < 20)
		continue;

	    switch (iWnd) {
	    case 0:
		XMoveWindow (dpy, mainWindow, evtGrabbed.xmotion.x_root - *x, evtGrabbed.xmotion.y_root - *y);
		XRaiseWindow (dpy, mainWindow);
		break;
	    case 1:
		XMoveWindow (dpy, inputWindow, evtGrabbed.xmotion.x_root - *x, evtGrabbed.xmotion.y_root - *y);
		XRaiseWindow (dpy, inputWindow);
		break;
	    }

	    LastTime = evtGrabbed.xmotion.time;
	}

	switch (iWnd) {
	case 0:
	    DisplayMainWindow ();
	    break;
	case 1:
	    DisplayInputWindow ();
	    break;
	}
    }

    *x = evtGrabbed.xmotion.x_root - *x;
    *y = evtGrabbed.xmotion.y_root - *y;

    return point;
}

void WaitButtonRelease (XPoint * point)
{
    XEvent          ev;

    while (1) {
	XNextEvent (dpy, &ev);
	switch (ev.type) {
	case ButtonRelease:
	    point->x = ev.xbutton.x;
	    point->y = ev.xbutton.y;
	    return;
	default:
	    break;
	}
    }
}

/* ****************************************************************** */
