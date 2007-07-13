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
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#include "ui.h"
#include "version.h"
#include "MainWindow.h"
#include "InputWindow.h"
#include "PYFA.h"
#include "py.h"
#include "sp.h"
#include "ime.h"

extern Display *dpy;
extern int      iScreen;
extern int      MAINWND_WIDTH;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern int      iInputWindowX;
extern int      iInputWindowY;
extern int      iTempInputWindowX;
extern int      iTempInputWindowY;
extern int      iInputWindowWidth;
extern int      iInputWindowHeight;

extern int      iMaxCandWord;
extern Bool     _3DEffectMainWindow;
extern _3D_EFFECT _3DEffectInputWindow;
extern WINDOW_COLOR inputWindowColor;
extern WINDOW_COLOR mainWindowColor;
extern MESSAGE_COLOR IMNameColor[];
extern MESSAGE_COLOR messageColor[];
extern MESSAGE_COLOR inputWindowLineColor;
extern MESSAGE_COLOR mainWindowLineColor;
extern MESSAGE_COLOR cursorColor;
extern WINDOW_COLOR VKWindowColor;
extern MESSAGE_COLOR VKWindowFontColor;
extern MESSAGE_COLOR VKWindowAlphaColor;
extern ENTER_TO_DO enterToDo;

extern HOTKEYS  hkTrigger[];
extern HOTKEYS  hkGBK[];
extern HOTKEYS  hkCorner[];
extern HOTKEYS  hkPunc[];
extern HOTKEYS  hkPrevPage[];
extern HOTKEYS  hkNextPage[];
extern HOTKEYS  hkWBAddPhrase[];
extern HOTKEYS  hkWBDelPhrase[];
extern HOTKEYS  hkWBAdjustOrder[];
extern HOTKEYS  hkPYAddFreq[];
extern HOTKEYS  hkPYDelFreq[];
extern HOTKEYS  hkPYDelUserPhr[];
extern HOTKEYS  hkLegend[];
extern HOTKEYS  hkTrack[];
extern HOTKEYS  hkGetPY[];

extern KEYCODE  switchKey;
extern XIMTriggerKey *Trigger_Keys;
extern INT8     iTriggerKeyCount;

extern Bool     bUseGBK;
extern Bool     bEngPuncAfterNumber;
extern Bool     bAutoHideInputWindow;
extern XColor   colorArrow;
extern Bool     bTrackCursor;
extern Bool     bCenterInputWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern Bool     bCompactMainWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern int      iFontSize;
extern int      iMainWindowFontSize;

extern Bool     bChnPunc;
extern Bool     bCorner;
extern Bool     bUseLegend;

extern Bool     bPYCreateAuto;
extern Bool     bPYSaveAutoAsPhrase;
extern Bool     bPhraseTips;
extern SEMICOLON_TO_DO     semicolonToDo;
extern Bool     bEngAfterCap;
extern Bool     bShowUserSpeed;
extern Bool	bShowVK;

extern Bool     bFullPY;
extern Bool     bDisablePagingInLegend;

extern int      i2ndSelectKey;
extern int      i3rdSelectKey;

extern char     strFontName[];
extern char     strFontEnName[];

extern ADJUSTORDER baseOrder;
extern ADJUSTORDER phraseOrder;
extern ADJUSTORDER freqOrder;

extern INT8     iIMIndex;
extern Bool     bLocked;

extern MHPY     MHPY_C[];
extern MHPY     MHPY_S[];

extern Bool     bUsePinyin;
extern Bool     bUseSP;
extern Bool     bUseQW;
extern Bool     bUseTable;

//extern Bool     bLumaQQ;
extern char     cPYYCDZ[];

extern Bool     bDoubleSwitchKey;
extern Bool     bPointAfterNumber;
extern Bool     bConvertPunc;
extern unsigned int iTimeInterval;
extern uint     iFixedInputWindowWidth;
extern Bool	bShowInputWindowTriggering;

#ifdef _USE_XFT
extern Bool     bUseAA;
#else
extern char     strUserLocale[];
#endif

/*
#if defined(DARWIN)*/
/* function to reverse byte order for integer
this is required for Mac machine*/
/*int ReverseInt (unsigned int pc_int)
{
    int             mac_int;
    unsigned char  *p;

    mac_int = pc_int;
    p = (unsigned char *) &pc_int;
    mac_int = (p[3] << 24) + (p[2] << 16) + (p[1] << 8) + p[0];
    return mac_int;
}
#endif
*/

Bool MyStrcmp (char *str1, char *str2)
{
    return !strncmp (str1, str2, strlen (str2));
}

/*
 * ��ȡ�û��������ļ�
 */
void LoadConfig (Bool bMode)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    int             r, g, b;	//���������

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/config");

    fp = fopen (strPath, "rt");

    if (!fp) {
	SaveConfig ();
	LoadConfig (True);	//����Ĭ��ֵ
	return;
    }

    for (;;) {
	if (!fgets (str, PATH_MAX, fp))
	    break;

	i = strlen (str) - 1;
	while (str[i] == ' ' || str[i] == '\n')
	    str[i--] = '\0';

	pstr = str;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (MyStrcmp (pstr, "��ʾ����(��)=")) {
	    pstr += 13;
	    strcpy (strFontName, pstr);
	}
	if (MyStrcmp (pstr, "��ʾ����(Ӣ)=")) {
	    pstr += 13;
	    strcpy (strFontEnName, pstr);
	}
	else if (MyStrcmp (pstr, "��ʾ�����С=")) {
	    pstr += 13;
	    iFontSize = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "�����������С=")) {
	    pstr += 15;
	    iMainWindowFontSize = atoi (pstr);
	}
#ifdef _USE_XFT
	else if (MyStrcmp (pstr, "�Ƿ�ʹ��AA����=")) {
	    pstr += 15;
	    bUseAA = atoi (pstr);
	}
#else
	else if (MyStrcmp (pstr, "��������=")) {
	    pstr += 9;
	    strcpy (strUserLocale, pstr);
	}
#endif
	else if (MyStrcmp (pstr, "��ѡ�ʸ���=")) {
	    pstr += 11;
	    iMaxCandWord = atoi (pstr);
	    if (iMaxCandWord > 10)
		iMaxCandWord = MAX_CAND_WORD;
	}
	else if (MyStrcmp (pstr, "���ֺ����Ƿ���=")) {
	    pstr += 17;
	    bEngPuncAfterNumber = atoi (pstr);
	}
	/*else if (MyStrcmp (pstr, "LumaQQ֧��=")) {
	    pstr += 11;
	    bLumaQQ = atoi (pstr);
	}*/
	else if (MyStrcmp (pstr, "�������Ƿ�ʹ��3D����=")) {
	    pstr += 21;
	    _3DEffectMainWindow = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "������ʹ��3D����=")) {
	    pstr += 17;
	    _3DEffectInputWindow = (_3D_EFFECT) atoi (pstr);
	}
	else if (MyStrcmp (pstr, "�Ƿ��Զ�����������=")) {
	    pstr += 19;
	    bAutoHideInputWindow = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "�������Ƿ����=")) {
	    pstr += 15;
	    bCenterInputWindow = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "�������̶����=")) {
	    pstr += 15;
	    iFixedInputWindowWidth = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "�״���ʾ������=")) {
	    pstr += 15;
	    bShowInputWindowTriggering = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "��ź�ӵ�=")) {
	    pstr += 11;
	    bPointAfterNumber = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "��ʾ�����ٶ�=")) {
	    pstr += 13;
	    bShowUserSpeed = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "����������ģʽ=")) {
	    pstr += 15;
	    hideMainWindow = (HIDE_MAINWINDOW) atoi (pstr);
	}
	else if (MyStrcmp (pstr, "��ʾ�������=")) {
		pstr += 13;
		bShowVK = atoi(pstr);
	}
	else if (MyStrcmp (pstr, "���ɫ=") && bMode) {
	    pstr += 7;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    cursorColor.color.red = (r << 8);
	    cursorColor.color.green = (g << 8);
	    cursorColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "�����ڱ���ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowColor.backColor.red = (r << 8);
	    mainWindowColor.backColor.green = (g << 8);
	    mainWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "����������ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowLineColor.color.red = (r << 8);
	    mainWindowLineColor.color.green = (g << 8);
	    mainWindowLineColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���������뷨����ɫ=") && bMode) {
	    int             r1, r2, g1, g2, b1, b2;

	    pstr += 19;
	    sscanf (pstr, "%d %d %d %d %d %d %d %d %d", &r, &g, &b, &r1, &g1, &b1, &r2, &g2, &b2);
	    IMNameColor[0].color.red = (r << 8);
	    IMNameColor[0].color.green = (g << 8);
	    IMNameColor[0].color.blue = (b << 8);
	    IMNameColor[1].color.red = (r1 << 8);
	    IMNameColor[1].color.green = (g1 << 8);
	    IMNameColor[1].color.blue = (b1 << 8);
	    IMNameColor[2].color.red = (r2 << 8);
	    IMNameColor[2].color.green = (g2 << 8);
	    IMNameColor[2].color.blue = (b2 << 8);
	}
	else if (MyStrcmp (pstr, "���봰����ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowColor.backColor.red = (r << 8);
	    inputWindowColor.backColor.green = (g << 8);
	    inputWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰��ʾɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[0].color.red = (r << 8);
	    messageColor[0].color.green = (g << 8);
	    messageColor[0].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[1].color.red = (r << 8);
	    messageColor[1].color.green = (g << 8);
	    messageColor[1].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰���ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[2].color.red = (r << 8);
	    messageColor[2].color.green = (g << 8);
	    messageColor[2].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰��һ����ѡ��ɫ=") && bMode) {
	    pstr += 21;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[3].color.red = (r << 8);
	    messageColor[3].color.green = (g << 8);
	    messageColor[3].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[4].color.red = (r << 8);
	    messageColor[4].color.green = (g << 8);
	    messageColor[4].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰��ʾ����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[5].color.red = (r << 8);
	    messageColor[5].color.green = (g << 8);
	    messageColor[5].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰�����ı�ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[6].color.red = (r << 8);
	    messageColor[6].color.green = (g << 8);
	    messageColor[6].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰����ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowLineColor.color.red = (r << 8);
	    inputWindowLineColor.color.green = (g << 8);
	    inputWindowLineColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "���봰��ͷɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    colorArrow.red = (r << 8);
	    colorArrow.green = (g << 8);
	    colorArrow.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "������̴�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowColor.backColor.red = (r << 8);
	    VKWindowColor.backColor.green = (g << 8);
	    VKWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "������̴���ĸɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowAlphaColor.color.red = (r << 8);
	    VKWindowAlphaColor.color.green = (g << 8);
	    VKWindowAlphaColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "������̴�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowFontColor.color.red = (r << 8);
	    VKWindowFontColor.color.green = (g << 8);
	    VKWindowFontColor.color.blue = (b << 8);
	}

	else if (MyStrcmp (pstr, "��/�ر����뷨=") && bMode) {
	    pstr += 16;
	    SetTriggerKeys (pstr);
	    SetHotKey (pstr, hkTrigger);
	}
	else if (MyStrcmp (pstr, "��Ӣ�Ŀ����л���=")) {
	    pstr += 17;
	    SetSwitchKey (pstr);
	}
	else if (MyStrcmp (pstr, "˫����Ӣ���л�=")) {
	    pstr += 15;
	    bDoubleSwitchKey = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "����ʱ����=")) {
	    pstr += 13;
	    iTimeInterval = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "������=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkTrack);
	}
	else if (MyStrcmp (pstr, "GBK֧��=")) {
	    pstr += 8;
	    SetHotKey (pstr, hkGBK);
	}
	else if (MyStrcmp (pstr, "�ֺż���Ϊ=")) {
	    pstr += 11;
	    semicolonToDo = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "��д��ĸ����Ӣ��=")) {
	    pstr += 17;
	    bEngAfterCap = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "ת��Ӣ���еı��=")) {
	    pstr += 17;
	    bConvertPunc = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "���뷽ʽ��ֹ��ҳ=")) {
	    pstr += 17;
	    bDisablePagingInLegend = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "����֧��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkLegend);
	}
	else if (MyStrcmp (pstr, "����ƴ��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkGetPY);
	}
	else if (MyStrcmp (pstr, "Enter����Ϊ=")) {
	    pstr += 12;
	    enterToDo = (ENTER_TO_DO) atoi (pstr);
	}
	else if (MyStrcmp (pstr, "ȫ���=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkCorner);
	}
	else if (MyStrcmp (pstr, "���ı��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkPunc);
	}
	else if (MyStrcmp (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkPrevPage);
	}
	else if (MyStrcmp (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkNextPage);
	}
	else if (MyStrcmp (pstr, "�ڶ�����ѡ��ѡ���=")) {
	    pstr += 19;
	    if (!strcasecmp (pstr, "SHIFT")) {
		i2ndSelectKey = 50;	//��SHIFT��ɨ����
		i3rdSelectKey = 62;	//��SHIFT��ɨ����
	    }
	    else if (!strcasecmp (pstr, "CTRL")) {
		i2ndSelectKey = 37;	//��CTRL��ɨ����
		i3rdSelectKey = 109;	//��CTRL��ɨ����
	    }
	}

	else if (MyStrcmp (pstr, "ʹ��ƴ��=")) {
	    pstr += 9;
	    bUsePinyin = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "ʹ��˫ƴ=")) {
	    pstr += 9;
	    bUseSP = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "ʹ����λ=")) {
	    pstr += 9;
	    bUseQW = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "ʹ�����=")) {
	    pstr += 9;
	    bUseTable = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "��ʾ�ʿ��еĴ���=")) {
	    pstr += 17;
	    bPhraseTips = atoi (pstr);
	}

	else if (MyStrcmp (str, "ʹ��ȫƴ=")) {
	    pstr += 9;
	    bFullPY = atoi (pstr);
	}
	else if (MyStrcmp (str, "ƴ���Զ����=")) {
	    pstr += 13;
	    bPYCreateAuto = atoi (pstr);
	}
	else if (MyStrcmp (str, "�����Զ����=")) {
	    pstr += 13;
	    bPYSaveAutoAsPhrase = atoi (pstr);
	}
	else if (MyStrcmp (str, "����ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYAddFreq);
	}
	else if (MyStrcmp (str, "ɾ��ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYDelFreq);
	}
	else if (MyStrcmp (str, "ɾ��ƴ���û�����=")) {
	    pstr += 17;
	    SetHotKey (pstr, hkPYDelUserPhr);
	}
	else if (MyStrcmp (str, "ƴ���Դʶ��ּ�=")) {
	    pstr += 15;
	    cPYYCDZ[0] = pstr[0];
	    cPYYCDZ[1] = pstr[1];
	}
	else if (MyStrcmp (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    baseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    phraseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "ƴ�����ô����������ʽ=")) {
	    pstr += 23;
	    freqOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��an��ang=")) {
	    pstr += 16;
	    MHPY_C[0].bMode = atoi (pstr);
	    MHPY_S[5].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��en��eng=")) {
	    pstr += 16;
	    MHPY_C[1].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��ian��iang=")) {
	    pstr += 18;
	    MHPY_C[2].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��in��ing=")) {
	    pstr += 16;
	    MHPY_C[3].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��ou��u=")) {
	    pstr += 14;
	    MHPY_C[4].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��uan��uang=")) {
	    pstr += 18;
	    MHPY_C[5].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��c��ch=")) {
	    pstr += 14;
	    MHPY_S[0].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��f��h=")) {
	    pstr += 13;
	    MHPY_S[1].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��l��n=")) {
	    pstr += 13;
	    MHPY_S[2].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��s��sh=")) {
	    pstr += 14;
	    MHPY_S[3].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "�Ƿ�ģ��z��zh=")) {
	    pstr += 14;
	    MHPY_S[4].bMode = atoi (pstr);
	}
    }

    fclose (fp);

    if (!Trigger_Keys) {
	iTriggerKeyCount = 0;
	Trigger_Keys = (XIMTriggerKey *) malloc (sizeof (XIMTriggerKey) * (iTriggerKeyCount + 2));
	Trigger_Keys[0].keysym = XK_space;
	Trigger_Keys[0].modifier = ControlMask;
	Trigger_Keys[0].modifier_mask = ControlMask;
	Trigger_Keys[1].keysym = 0;
	Trigger_Keys[1].modifier = 0;
	Trigger_Keys[1].modifier_mask = 0;
    }
}

void SaveConfig (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "config");
    fp = fopen (strPath, "wt");
    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� config��\n");
	return;
    }

    fprintf (fp, "[����]\n");
    fprintf (fp, "��ʾ����(��)=%s\n", strFontName);
    fprintf (fp, "��ʾ����(Ӣ)=%s\n", strFontEnName);
    fprintf (fp, "��ʾ�����С=%d\n", iFontSize);
    fprintf (fp, "�����������С=%d\n", iMainWindowFontSize);
#ifdef _USE_XFT
    fprintf (fp, "�Ƿ�ʹ��AA����=%d\n", bUseAA);
#else
    if (strUserLocale[0])
	fprintf (fp, "��������=%s\n", strUserLocale);
    else
	fprintf (fp, "#��������=zh_CN.gb2312\n");
#endif

    fprintf (fp, "\n[���]\n");
    fprintf (fp, "���ֺ����Ƿ���=%d\n", bEngPuncAfterNumber);
    fprintf (fp, "Enter����Ϊ=%d\n", enterToDo);
    fprintf (fp, "�ֺż���Ϊ=%d\n", (int)semicolonToDo);
    fprintf (fp, "��д��ĸ����Ӣ��=%d\n", bEngAfterCap);
    fprintf (fp, "ת��Ӣ���еı��=%d\n", bConvertPunc );
    fprintf (fp, "���뷽ʽ��ֹ��ҳ=%d\n", bDisablePagingInLegend);
    //fprintf (fp, "LumaQQ֧��=%d\n", bLumaQQ);

    fprintf (fp, "\n[����]\n");
    fprintf (fp, "��ѡ�ʸ���=%d\n", iMaxCandWord);
    fprintf (fp, "�������Ƿ�ʹ��3D����=%d\n", _3DEffectMainWindow);
    fprintf (fp, "������ʹ��3D����=%d\n", _3DEffectInputWindow);
    fprintf (fp, "����������ģʽ=%d\n", (int) hideMainWindow);
    fprintf (fp, "��ʾ�������=%d\n", (int) bShowVK);
    fprintf (fp, "�Ƿ��Զ�����������=%d\n", bAutoHideInputWindow);
    fprintf (fp, "�������Ƿ����=%d\n", bCenterInputWindow );
    fprintf (fp, "�״���ʾ������=%d\n", bShowInputWindowTriggering );
    fprintf (fp, "#�������̶���Ƚ�������������뷨��0��ʾ���̶����\n");
    fprintf (fp, "�������̶����=%d\n", iFixedInputWindowWidth);
    fprintf (fp, "��ź�ӵ�=%d\n", bPointAfterNumber);
    fprintf (fp, "��ʾ�����ٶ�=%d\n", bShowUserSpeed);

    fprintf (fp, "���ɫ=%d %d %d\n", cursorColor.color.red >> 8, cursorColor.color.green >> 8, cursorColor.color.blue >> 8);
    fprintf (fp, "�����ڱ���ɫ=%d %d %d\n", mainWindowColor.backColor.red >> 8, mainWindowColor.backColor.green >> 8, mainWindowColor.backColor.blue >> 8);
    fprintf (fp, "����������ɫ=%d %d %d\n", mainWindowLineColor.color.red >> 8, mainWindowLineColor.color.green >> 8, mainWindowLineColor.color.blue >> 8);
    fprintf (fp, "���������뷨����ɫ=%d %d %d %d %d %d %d %d %d\n", IMNameColor[0].color.red >> 8, IMNameColor[0].color.green >> 8, IMNameColor[0].color.blue >> 8, IMNameColor[1].color.red >> 8, IMNameColor[1].color.green >> 8,
	     IMNameColor[1].color.blue >> 8, IMNameColor[2].color.red >> 8, IMNameColor[2].color.green >> 8, IMNameColor[2].color.blue >> 8);
    fprintf (fp, "���봰����ɫ=%d %d %d\n", inputWindowColor.backColor.red >> 8, inputWindowColor.backColor.green >> 8, inputWindowColor.backColor.blue >> 8);
    fprintf (fp, "���봰��ʾɫ=%d %d %d\n", messageColor[0].color.red >> 8, messageColor[0].color.green >> 8, messageColor[0].color.blue >> 8);
    fprintf (fp, "���봰�û�����ɫ=%d %d %d\n", messageColor[1].color.red >> 8, messageColor[1].color.green >> 8, messageColor[1].color.blue >> 8);
    fprintf (fp, "���봰���ɫ=%d %d %d\n", messageColor[2].color.red >> 8, messageColor[2].color.green >> 8, messageColor[2].color.blue >> 8);
    fprintf (fp, "���봰��һ����ѡ��ɫ=%d %d %d\n", messageColor[3].color.red >> 8, messageColor[3].color.green >> 8, messageColor[3].color.blue >> 8);
    fprintf (fp, "#����ɫֵֻ����ƴ���е��û������\n");
    fprintf (fp, "���봰�û�����ɫ=%d %d %d\n", messageColor[4].color.red >> 8, messageColor[4].color.green >> 8, messageColor[4].color.blue >> 8);
    fprintf (fp, "���봰��ʾ����ɫ=%d %d %d\n", messageColor[5].color.red >> 8, messageColor[5].color.green >> 8, messageColor[5].color.blue >> 8);
    fprintf (fp, "#��ʡ�ƴ���ĵ���/ϵͳ�����ʹ�ø���ɫ\n");
    fprintf (fp, "���봰�����ı�ɫ=%d %d %d\n", messageColor[6].color.red >> 8, messageColor[6].color.green >> 8, messageColor[6].color.blue >> 8);
    fprintf (fp, "���봰����ɫ=%d %d %d\n", inputWindowLineColor.color.red >> 8, inputWindowLineColor.color.green >> 8, inputWindowLineColor.color.blue >> 8);
    fprintf (fp, "���봰��ͷɫ=%d %d %d\n", colorArrow.red >> 8, colorArrow.green >> 8, colorArrow.blue >> 8);
    fprintf (fp, "������̴�����ɫ=%d %d %d\n", VKWindowColor.backColor.red >> 8, VKWindowColor.backColor.green >> 8, VKWindowColor.backColor.blue >> 8);
    fprintf (fp, "������̴���ĸɫ=%d %d %d\n", VKWindowAlphaColor.color.red >> 8, VKWindowAlphaColor.color.green >> 8, VKWindowAlphaColor.color.blue >> 8);
    fprintf (fp, "������̴�����ɫ=%d %d %d\n", VKWindowFontColor.color.red >> 8, VKWindowFontColor.color.green >> 8, VKWindowFontColor.color.blue >> 8);

    fprintf (fp, "\n#���ˡ���Ӣ�Ŀ����л������⣬�������ȼ���������Ϊ�������м��ÿո�ָ�\n");
    fprintf (fp, "[�ȼ�]\n");
    fprintf (fp, "��/�ر����뷨=CTRL_SPACE\n");
    fprintf (fp, "#��Ӣ�Ŀ����л��� ��������ΪL_CTRL R_CTRL L_SHIFT R_SHIFT\n");
    fprintf (fp, "��Ӣ�Ŀ����л���=L_CTRL\n");
    fprintf (fp, "˫����Ӣ���л�=%d\n", bDoubleSwitchKey);
    fprintf (fp, "����ʱ����=%u\n", iTimeInterval);
    fprintf (fp, "������=CTRL_K\n");
    fprintf (fp, "GBK֧��=CTRL_M\n");
    fprintf (fp, "����֧��=CTRL_L\n");
    fprintf (fp, "����ƴ��=CTRL_ALT_E\n");
    fprintf (fp, "ȫ���=SHIFT_SPACE\n");
    fprintf (fp, "���ı��=ALT_SPACE\n");
    fprintf (fp, "��һҳ=-\n");
    fprintf (fp, "��һҳ==\n");
    fprintf (fp, "�ڶ�����ѡ��ѡ���=SHIFT\n");

    fprintf (fp, "\n[���뷨]\n");
    fprintf (fp, "ʹ��ƴ��=%d\n", bUsePinyin);
    fprintf (fp, "ʹ��˫ƴ=%d\n", bUseSP);
    fprintf (fp, "ʹ����λ=%d\n", bUseQW);
    fprintf (fp, "ʹ�����=%d\n", bUseTable);
    fprintf (fp, "��ʾ�ʿ��еĴ���=%d\n", bPhraseTips);

    fprintf (fp, "\n[ƴ��]\n");
    fprintf (fp, "ʹ��ȫƴ=%d\n", bFullPY);
    fprintf (fp, "ƴ���Զ����=%d\n", bPYCreateAuto);
    fprintf (fp, "�����Զ����=%d\n", bPYSaveAutoAsPhrase);
    fprintf (fp, "����ƴ��������=CTRL_8\n");
    fprintf (fp, "ɾ��ƴ��������=CTRL_7\n");
    fprintf (fp, "ɾ��ƴ���û�����=CTRL_DELETE\n");
    fprintf (fp, "#ƴ���Դʶ��ּ����Ⱥź�����Ӽ�����Ҫ�пո�\n");
    fprintf (fp, "ƴ���Դʶ��ּ�=%c%c\n", cPYYCDZ[0], cPYYCDZ[1]);
    fprintf (fp, "#���������ʽ˵����0-->������  1-->���ٵ���  2-->��Ƶ�ʵ���\n");
    fprintf (fp, "ƴ���������������ʽ=%d\n", baseOrder);
    fprintf (fp, "ƴ���������������ʽ=%d\n", phraseOrder);
    fprintf (fp, "ƴ�����ô����������ʽ=%d\n", freqOrder);
    fprintf (fp, "�Ƿ�ģ��an��ang=%d\n", MHPY_C[0].bMode);
    fprintf (fp, "�Ƿ�ģ��en��eng=%d\n", MHPY_C[1].bMode);
    fprintf (fp, "�Ƿ�ģ��ian��iang=%d\n", MHPY_C[2].bMode);
    fprintf (fp, "�Ƿ�ģ��in��ing=%d\n", MHPY_C[3].bMode);
    fprintf (fp, "�Ƿ�ģ��ou��u=%d\n", MHPY_C[4].bMode);
    fprintf (fp, "�Ƿ�ģ��uan��uang=%d\n", MHPY_C[5].bMode);
    fprintf (fp, "�Ƿ�ģ��c��ch=%d\n", MHPY_S[0].bMode);
    fprintf (fp, "�Ƿ�ģ��f��h=%d\n", MHPY_S[1].bMode);
    fprintf (fp, "�Ƿ�ģ��l��n=%d\n", MHPY_S[2].bMode);
    fprintf (fp, "�Ƿ�ģ��s��sh=%d\n", MHPY_S[3].bMode);
    fprintf (fp, "�Ƿ�ģ��z��zh=%d\n", MHPY_S[4].bMode);

    fclose (fp);
}

void LoadProfile (void)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    Bool            bRetVal;

    iMainWindowX = MAINWND_STARTX;
    iMainWindowY = MAINWND_STARTY;
    iInputWindowX = INPUTWND_STARTX;
    iInputWindowY = INPUTWND_STARTY;

    bRetVal = False;
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/profile");

    fp = fopen (strPath, "rt");

    if (fp) {
	for (;;) {
	    if (!fgets (str, PATH_MAX, fp))
		break;

	    i = strlen (str) - 1;
	    while (str[i] == ' ' || str[i] == '\n')
		str[i--] = '\0';

	    pstr = str;

	    if (MyStrcmp (str, "�汾=")) {
		pstr += 5;

		if (!strcasecmp (FCITX_VERSION, pstr))
		    bRetVal = True;
	    }
	    else if (MyStrcmp (str, "������λ��X=")) {
		pstr += 12;
		iMainWindowX = atoi (pstr);
		if (iMainWindowX < 0)
		    iMainWindowX = 0;
		else if ((iMainWindowX + MAINWND_WIDTH) > DisplayWidth (dpy, iScreen))
		    iMainWindowX = DisplayWidth (dpy, iScreen) - MAINWND_WIDTH;
	    }
	    else if (MyStrcmp (str, "������λ��Y=")) {
		pstr += 12;
		iMainWindowY = atoi (pstr);
		if (iMainWindowY < 0)
		    iMainWindowY = 0;
		else if ((iMainWindowY + MAINWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iMainWindowY = DisplayHeight (dpy, iScreen) - MAINWND_HEIGHT;
	    }
	    else if (MyStrcmp (str, "���봰��λ��X=")) {
		pstr += 14;
		iInputWindowX = atoi (pstr);
		if (iInputWindowX < 0)
		    iInputWindowX = 0;
		else if ((iInputWindowX + iInputWindowWidth) > DisplayWidth (dpy, iScreen))
		    iInputWindowX = DisplayWidth (dpy, iScreen) - iInputWindowWidth - 3;
	    }
	    else if (MyStrcmp (str, "���봰��λ��Y=")) {
		pstr += 14;
		iInputWindowY = atoi (pstr);
		if (iInputWindowY < 0)
		    iInputWindowY = 0;
		else if ((iInputWindowY + INPUTWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iInputWindowY = DisplayHeight (dpy, iScreen) - iInputWindowHeight;
	    }
	    else if (MyStrcmp (str, "�Ƿ�ȫ��=")) {
		pstr += 9;
		bCorner = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "�Ƿ����ı��=")) {
		pstr += 13;
		bChnPunc = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "�Ƿ�GBK=")) {
		pstr += 8;
		bUseGBK = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "�Ƿ������=")) {
		pstr += 13;
		bTrackCursor = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "�Ƿ�����=")) {
		pstr += 9;
		bUseLegend = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "��ǰ���뷨=")) {
		pstr += 11;
		iIMIndex = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "��ֹ�ü����л�=")) {
		pstr += 15;
		bLocked = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "���ģʽ=")) {
		pstr += 9;
		bCompactMainWindow = atoi (pstr);
	    }
	}

	fclose (fp);
    }

    if (!bRetVal) {
	SaveConfig ();
	SaveProfile ();
    }
}

void SaveProfile (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "profile");
    fp = fopen (strPath, "wt");

    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� profile!\n");
	return;
    }

    fprintf (fp, "�汾=%s\n", FCITX_VERSION);
    fprintf (fp, "������λ��X=%d\n", iMainWindowX);
    fprintf (fp, "������λ��Y=%d\n", iMainWindowY);
    fprintf (fp, "���봰��λ��X=%d\n", iInputWindowX);
    fprintf (fp, "���봰��λ��Y=%d\n", iInputWindowY);
    fprintf (fp, "�Ƿ�ȫ��=%d\n", bCorner);
    fprintf (fp, "�Ƿ����ı��=%d\n", bChnPunc);
    fprintf (fp, "�Ƿ�GBK=%d\n", bUseGBK);
    fprintf (fp, "�Ƿ������=%d\n", bTrackCursor);
    fprintf (fp, "�Ƿ�����=%d\n", bUseLegend);
    fprintf (fp, "��ǰ���뷨=%d\n", iIMIndex);
    fprintf (fp, "��ֹ�ü����л�=%d\n", bLocked);
    fprintf (fp, "���ģʽ=%d\n", bCompactMainWindow);

    iTempInputWindowX = iInputWindowX;
    iTempInputWindowY = iInputWindowY;

    fclose (fp);
}

void SetHotKey (char *strKeys, HOTKEYS * hotkey)
{
    char           *p;
    char            strKey[30];
    int             i, j;

    p = strKeys;

    while (*p == ' ')
	p++;
    i = 0;
    while (p[i] != ' ' && p[i] != '\0')
	i++;
    strncpy (strKey, p, i);
    strKey[i] = '\0';
    p += i + 1l;
    j = ParseKey (strKey);
    if (j != -1)
	hotkey[0] = j;
    if (j == -1)
	j = 0;
    else
	j = 1;

    i = 0;
    while (p[i] != ' ' && p[i] != '\0')
	i++;
    if (p[0]) {
	strncpy (strKey, p, i);
	strKey[i] = '\0';

	i = ParseKey (strKey);
	if (i == -1)
	    i = 0;
    }
    else
	i = 0;

    hotkey[j] = i;
}

/*
 * �����ļ����ж�����
 * ע��:�ļ��еĿ���Ҳ��Ϊһ�д���
 */
int CalculateRecordNumber (FILE * fpDict)
{
    char            strText[101];
    int             nNumber = 0;

    for (;;) {
	if (!fgets (strText, 100, fpDict))
	    break;

	nNumber++;
    }
    rewind (fpDict);

    return nNumber;
}

void SetSwitchKey (char *str)
{
    if (!strcasecmp (str, "R_CTRL"))
	switchKey = R_CTRL;
    else if (!strcasecmp (str, "R_SHIFT"))
	switchKey = R_SHIFT;
    else if (!strcasecmp (str, "L_SHIFT"))
	switchKey = L_SHIFT;
    else
	switchKey = L_CTRL;
}

void SetTriggerKeys (char *str)
{
    int             i;
    char            strKey[2][30];
    char           *p;

    //�������ж��û������˼����ȼ������Ϊ2    
    p = str;

    i = 0;
    iTriggerKeyCount = 0;
    while (*p) {
	if (*p == ' ') {
	    iTriggerKeyCount++;
	    while (*p == ' ')
		p++;
	    strcpy (strKey[1], p);
	    break;
	}
	else
	    strKey[0][i++] = *p++;
    }
    strKey[0][i] = '\0';

    Trigger_Keys = (XIMTriggerKey *) malloc (sizeof (XIMTriggerKey) * (iTriggerKeyCount + 2));
    for (i = 0; i <= (iTriggerKeyCount + 1); i++) {
	Trigger_Keys[i].keysym = 0;
	Trigger_Keys[i].modifier = 0;
	Trigger_Keys[i].modifier_mask = 0;
    }

    for (i = 0; i <= iTriggerKeyCount; i++) {
	if (MyStrcmp (strKey[i], "CTRL_")) {
	    Trigger_Keys[i].modifier = Trigger_Keys[i].modifier | ControlMask;
	    Trigger_Keys[i].modifier_mask = Trigger_Keys[i].modifier_mask | ControlMask;
	}
	else if (MyStrcmp (strKey[i], "SHIFT_")) {
	    Trigger_Keys[i].modifier = Trigger_Keys[i].modifier | ShiftMask;
	    Trigger_Keys[i].modifier_mask = Trigger_Keys[i].modifier_mask | ShiftMask;
	}
	else if (MyStrcmp (strKey[i], "ALT_")) {
	    Trigger_Keys[i].modifier = Trigger_Keys[i].modifier | Mod1Mask;
	    Trigger_Keys[i].modifier_mask = Trigger_Keys[i].modifier_mask | Mod1Mask;
	}

	if (Trigger_Keys[i].modifier == 0) {
	    Trigger_Keys[i].modifier = ControlMask;
	    Trigger_Keys[i].modifier_mask = ControlMask;
	}

	p = strKey[i] + strlen (strKey[i]) - 1;
	while (*p != '_') {
	    p--;
	    if (p == strKey[i] || (p == strKey[i] + strlen (strKey[i]) - 1)) {
		Trigger_Keys = (XIMTriggerKey *) malloc (sizeof (XIMTriggerKey) * (iTriggerKeyCount + 2));
		Trigger_Keys[i].keysym = XK_space;
		return;
	    }
	}
	p++;

	if (strlen (p) == 1)
	    Trigger_Keys[i].keysym = tolower (*p);
	else if (!strcasecmp (p, "LCTRL"))
	    Trigger_Keys[i].keysym = XK_Control_L;
	else if (!strcasecmp (p, "RCTRL"))
	    Trigger_Keys[i].keysym = XK_Control_R;
	else if (!strcasecmp (p, "LSHIFT"))
	    Trigger_Keys[i].keysym = XK_Shift_L;
	else if (!strcasecmp (p, "RSHIFT"))
	    Trigger_Keys[i].keysym = XK_Shift_R;
	else if (!strcasecmp (p, "LALT"))
	    Trigger_Keys[i].keysym = XK_Alt_L;
	else if (!strcasecmp (p, "RALT"))
	    Trigger_Keys[i].keysym = XK_Alt_R;
	else
	    Trigger_Keys[i].keysym = XK_space;
    }
}

Bool CheckHZCharset (char *strHZ)
{
    if (!bUseGBK) {
	//GB2312�ĺ��ֱ������Ϊ����һ���ֽڵ�ֵ��0xA1��0xFE֮��(ʵ��Ϊ0xF7)���ڶ����ֽڵ�ֵ��0xA1��0xFE֮��
	//���ڲ鵽������˵����һ�����ú�ʵ����������
	int             i;

	for (i = 0; i < strlen (strHZ); i++) {
	    if ((unsigned char) strHZ[i] < (unsigned char) 0xA1 || (unsigned char) strHZ[i] > (unsigned char) 0xF7 || (unsigned char) strHZ[i + 1] < (unsigned char) 0xA1 || (unsigned char) strHZ[i + 1] > (unsigned char) 0xFE)
		return False;
	    i++;
	}
    }

    return True;
}
