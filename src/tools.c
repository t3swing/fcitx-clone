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

extern char     strUserLocale[];
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
extern HOTKEYS  hkGBT[];

extern KEY_CODE switchKey;
extern XIMTriggerKey *Trigger_Keys;
extern INT8     iTriggerKeyCount;

extern Bool     bUseGBK;
extern Bool     bEngPuncAfterNumber;
//extern Bool     bAutoHideInputWindow;
extern XColor   colorArrow;
extern Bool     bTrackCursor;
extern Bool     bCenterInputWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern Bool     bCompactMainWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern int      iFontSize;
extern int      iMainWindowFontSize;

extern Bool     bUseGBKT;

extern Bool     bChnPunc;
extern Bool     bCorner;
extern Bool     bUseLegend;

extern Bool     bPYCreateAuto;
extern Bool     bPYSaveAutoAsPhrase;
extern Bool     bPhraseTips;
extern SEMICOLON_TO_DO semicolonToDo;
extern Bool     bEngAfterCap;

//显示打字速度
extern Bool     bShowUserSpeed;
extern Bool     bShowVersion;
extern Bool     bShowVK;

extern char     strNameOfPinyin[];
extern char     strNameOfShuangpin[];;
extern char     strNameOfQuwei[];

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

extern char     strDefaultSP[];
extern SP_FROM  iSPFrom;

//extern Bool     bLumaQQ;
extern char     cPYYCDZ[];

extern Bool     bDoubleSwitchKey;
extern Bool     bPointAfterNumber;
extern Bool     bConvertPunc;
extern unsigned int iTimeInterval;
extern uint     iFixedInputWindowWidth;
extern Bool     bShowInputWindowTriggering;

#ifdef _USE_XFT
extern Bool     bUseAA;
#else
extern char     strUserLocale[];
#endif
extern Bool     bUseBold;

extern INT8     iOffsetX;
extern INT8     iOffsetY;

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
 * 读取用户的设置文件
 */
void LoadConfig (Bool bMode)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    int             r, g, b;	//代表红绿蓝
    Bool            bFromUser = True;

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/config");

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/config");
	bFromUser = False;
    }

    fp = fopen (strPath, "rt");

    if (!fp || !bFromUser)
	SaveConfig ();

    if (!fp) {
	LoadConfig (True);	//读入默认值
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

	if (MyStrcmp (pstr, "字体区域=")) {
	    pstr += 9;
	    if (*pstr == ' ')
		pstr++;
	    strcpy (strUserLocale, pstr);
	}
	else if (MyStrcmp (pstr, "显示字体(中)=")) {
	    pstr += 13;
	    strcpy (strFontName, pstr);
	}
	else if (MyStrcmp (pstr, "显示字体(英)=")) {
	    pstr += 13;
	    strcpy (strFontEnName, pstr);
	}
	else if (MyStrcmp (pstr, "显示字体大小=")) {
	    pstr += 13;
	    iFontSize = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "主窗口字体大小=")) {
	    pstr += 15;
	    iMainWindowFontSize = atoi (pstr);
	}
#ifdef _USE_XFT
	else if (MyStrcmp (pstr, "使用AA字体=")) {
	    pstr += 11;
	    bUseAA = atoi (pstr);
	}
#else
	else if (MyStrcmp (pstr, "字体区域=")) {
	    pstr += 9;
	    strcpy (strUserLocale, pstr);
	}
#endif
	else if (MyStrcmp (pstr, "使用粗体=")) {
	    pstr += 9;
	    bUseBold = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "候选词个数=")) {
	    pstr += 11;
	    iMaxCandWord = atoi (pstr);
	    if (iMaxCandWord > 10)
		iMaxCandWord = MAX_CAND_WORD;
	}
	else if (MyStrcmp (pstr, "数字后跟半角符号=")) {
	    pstr += 17;
	    bEngPuncAfterNumber = atoi (pstr);
	}
	/*else if (MyStrcmp (pstr, "LumaQQ支持=")) {
	   pstr += 11;
	   bLumaQQ = atoi (pstr);
	   } */
	else if (MyStrcmp (pstr, "主窗口使用3D界面=")) {
	    pstr += 17;
	    _3DEffectMainWindow = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "输入条使用3D界面=")) {
	    pstr += 17;
	    _3DEffectInputWindow = (_3D_EFFECT) atoi (pstr);
	}
	/*else if (MyStrcmp (pstr, "自动隐藏输入条=")) {
	    pstr += 15;
	    bAutoHideInputWindow = atoi (pstr);
	}*/
	else if (MyStrcmp (pstr, "输入条居中=")) {
	    pstr += 11;
	    bCenterInputWindow = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "输入条固定宽度=")) {
	    pstr += 15;
	    iFixedInputWindowWidth = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "首次显示输入条=")) {
	    pstr += 15;
	    bShowInputWindowTriggering = atoi (pstr);
	}
	// 临时解决方案
	else if (MyStrcmp (pstr, "输入条偏移量X=")) {
	    pstr += 14;
	    iOffsetX = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "输入条偏移量Y=")) {
	    pstr += 14;
	    iOffsetY = atoi (pstr);
	}
	//********************************************
	else if (MyStrcmp (pstr, "序号后加点=")) {
	    pstr += 11;
	    bPointAfterNumber = atoi (pstr);
	}
	//显示速度
	else if (MyStrcmp (pstr, "显示打字速度=")) {
	    pstr += 13;
	    bShowUserSpeed = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "显示版本=")) {
	    pstr += 13;
	    bShowVersion = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "主窗口隐藏模式=")) {
	    pstr += 15;
	    hideMainWindow = (HIDE_MAINWINDOW) atoi (pstr);
	}
	else if (MyStrcmp (pstr, "显示虚拟键盘=")) {
	    pstr += 13;
	    bShowVK = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "光标色=")) {
	    pstr += 7;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    cursorColor.color.red = (r << 8);
	    cursorColor.color.green = (g << 8);
	    cursorColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "主窗口背景色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowColor.backColor.red = (r << 8);
	    mainWindowColor.backColor.green = (g << 8);
	    mainWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "主窗口线条色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowLineColor.color.red = (r << 8);
	    mainWindowLineColor.color.green = (g << 8);
	    mainWindowLineColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "主窗口输入法名称色=")) {
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
	else if (MyStrcmp (pstr, "输入窗背景色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowColor.backColor.red = (r << 8);
	    inputWindowColor.backColor.green = (g << 8);
	    inputWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗提示色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[0].color.red = (r << 8);
	    messageColor[0].color.green = (g << 8);
	    messageColor[0].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗用户输入色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[1].color.red = (r << 8);
	    messageColor[1].color.green = (g << 8);
	    messageColor[1].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗序号色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[2].color.red = (r << 8);
	    messageColor[2].color.green = (g << 8);
	    messageColor[2].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗第一个候选字色=")) {
	    pstr += 21;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[3].color.red = (r << 8);
	    messageColor[3].color.green = (g << 8);
	    messageColor[3].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗用户词组色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[4].color.red = (r << 8);
	    messageColor[4].color.green = (g << 8);
	    messageColor[4].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗提示编码色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[5].color.red = (r << 8);
	    messageColor[5].color.green = (g << 8);
	    messageColor[5].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗其它文本色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[6].color.red = (r << 8);
	    messageColor[6].color.green = (g << 8);
	    messageColor[6].color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗线条色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowLineColor.color.red = (r << 8);
	    inputWindowLineColor.color.green = (g << 8);
	    inputWindowLineColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "输入窗箭头色=")) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    colorArrow.red = (r << 8);
	    colorArrow.green = (g << 8);
	    colorArrow.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "虚拟键盘窗背景色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowColor.backColor.red = (r << 8);
	    VKWindowColor.backColor.green = (g << 8);
	    VKWindowColor.backColor.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "虚拟键盘窗字母色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowAlphaColor.color.red = (r << 8);
	    VKWindowAlphaColor.color.green = (g << 8);
	    VKWindowAlphaColor.color.blue = (b << 8);
	}
	else if (MyStrcmp (pstr, "虚拟键盘窗符号色=")) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    VKWindowFontColor.color.red = (r << 8);
	    VKWindowFontColor.color.green = (g << 8);
	    VKWindowFontColor.color.blue = (b << 8);
	}

	else if (MyStrcmp (pstr, "打开/关闭输入法=") && bMode) {
	    pstr += 16;
	    SetTriggerKeys (pstr);
	    SetHotKey (pstr, hkTrigger);
	}
	else if (MyStrcmp (pstr, "中英文快速切换键=")) {
	    pstr += 17;
	    SetSwitchKey (pstr);
	}
	else if (MyStrcmp (pstr, "GBK繁体切换键=")) {
	    pstr += 14;
	    SetHotKey (pstr, hkGBT);
	}
	else if (MyStrcmp (pstr, "双击中英文切换=")) {
	    pstr += 15;
	    bDoubleSwitchKey = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "击键时间间隔=")) {
	    pstr += 13;
	    iTimeInterval = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "光标跟随=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkTrack);
	}
	else if (MyStrcmp (pstr, "GBK支持=")) {
	    pstr += 8;
	    SetHotKey (pstr, hkGBK);
	}
	else if (MyStrcmp (pstr, "分号键行为=")) {
	    pstr += 11;
	    semicolonToDo = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "大写字母输入英文=")) {
	    pstr += 17;
	    bEngAfterCap = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "转换英文中的标点=")) {
	    pstr += 17;
	    bConvertPunc = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "联想方式禁止翻页=")) {
	    pstr += 17;
	    bDisablePagingInLegend = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "联想=")) {
	    pstr += 5;
	    SetHotKey (pstr, hkLegend);
	}
	else if (MyStrcmp (pstr, "反查拼音=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkGetPY);
	}
	else if (MyStrcmp (pstr, "Enter键行为=")) {
	    pstr += 12;
	    enterToDo = (ENTER_TO_DO) atoi (pstr);
	}
	else if (MyStrcmp (pstr, "全半角=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkCorner);
	}
	else if (MyStrcmp (pstr, "中文标点=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkPunc);
	}
	else if (MyStrcmp (pstr, "上一页=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkPrevPage);
	}
	else if (MyStrcmp (pstr, "下一页=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkNextPage);
	}
	else if (MyStrcmp (pstr, "第二三候选词选择键=")) {
	    pstr += 19;
	    if (!strcasecmp (pstr, "SHIFT")) {
		i2ndSelectKey = 50;	//左SHIFT的扫描码
		i3rdSelectKey = 62;	//右SHIFT的扫描码
	    }
	    else if (!strcasecmp (pstr, "CTRL")) {
		i2ndSelectKey = 37;	//左CTRL的扫描码
		i3rdSelectKey = 109;	//右CTRL的扫描码
	    }
	    else {
		i2ndSelectKey = pstr[0] ^ 0xFF;
		i3rdSelectKey = pstr[1] ^ 0xFF;
	    }
	}

	else if (MyStrcmp (pstr, "使用拼音=")) {
	    pstr += 9;
	    bUsePinyin = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "拼音名称=")) {
	    pstr += 9;
	    strcpy (strNameOfPinyin, pstr);
	}

	else if (MyStrcmp (pstr, "使用双拼=")) {
	    pstr += 9;
	    bUseSP = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "双拼名称=")) {
	    pstr += 9;
	    strcpy (strNameOfShuangpin, pstr);
	}
	else if (MyStrcmp (pstr, "默认双拼方案=")) {
	    pstr += 13;
	    if (*pstr == ' ')
		pstr++;
	    strcpy (strDefaultSP, pstr);
	    iSPFrom = SP_FROM_SYSTEM_CONFIG;
	}
	else if (MyStrcmp (pstr, "使用区位=")) {
	    pstr += 9;
	    bUseQW = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "区位名称=")) {
	    pstr += 9;
	    strcpy (strNameOfQuwei, pstr);
	}
	else if (MyStrcmp (pstr, "使用码表=")) {
	    pstr += 9;
	    bUseTable = atoi (pstr);
	}
	else if (MyStrcmp (pstr, "提示词库中的词组=")) {
	    pstr += 17;
	    bPhraseTips = atoi (pstr);
	}

	else if (MyStrcmp (str, "使用全拼=")) {
	    pstr += 9;
	    bFullPY = atoi (pstr);
	}
	else if (MyStrcmp (str, "拼音自动组词=")) {
	    pstr += 13;
	    bPYCreateAuto = atoi (pstr);
	}
	else if (MyStrcmp (str, "保存自动组词=")) {
	    pstr += 13;
	    bPYSaveAutoAsPhrase = atoi (pstr);
	}
	else if (MyStrcmp (str, "增加拼音常用字=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYAddFreq);
	}
	else if (MyStrcmp (str, "删除拼音常用字=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYDelFreq);
	}
	else if (MyStrcmp (str, "删除拼音用户词组=")) {
	    pstr += 17;
	    SetHotKey (pstr, hkPYDelUserPhr);
	}
	else if (MyStrcmp (str, "拼音以词定字键=")) {
	    pstr += 15;
	    cPYYCDZ[0] = pstr[0];
	    cPYYCDZ[1] = pstr[1];
	}
	else if (MyStrcmp (str, "拼音单字重码调整方式=")) {
	    pstr += 21;
	    baseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "拼音词组重码调整方式=")) {
	    pstr += 21;
	    phraseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "拼音常用词重码调整方式=")) {
	    pstr += 23;
	    freqOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊an和ang=")) {
	    pstr += 12;
	    MHPY_C[0].bMode = atoi (pstr);
	    MHPY_S[5].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊en和eng=")) {
	    pstr += 12;
	    MHPY_C[1].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊ian和iang=")) {
	    pstr += 14;
	    MHPY_C[2].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊in和ing=")) {
	    pstr += 12;
	    MHPY_C[3].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊ou和u=")) {
	    pstr += 10;
	    MHPY_C[4].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊uan和uang=")) {
	    pstr += 14;
	    MHPY_C[5].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊c和ch=")) {
	    pstr += 10;
	    MHPY_S[0].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊f和h=")) {
	    pstr += 9;
	    MHPY_S[1].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊l和n=")) {
	    pstr += 9;
	    MHPY_S[2].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊s和sh=")) {
	    pstr += 10;
	    MHPY_S[3].bMode = atoi (pstr);
	}
	else if (MyStrcmp (str, "模糊z和zh=")) {
	    pstr += 10;
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
	fprintf (stderr, "\n无法创建文件 config！\n");
	return;
    }

    fprintf (fp, "[程序]\n");
    fprintf (fp, "显示字体(中)=%s\n", strFontName);
    fprintf (fp, "显示字体(英)=%s\n", strFontEnName);
    fprintf (fp, "显示字体大小=%d\n", iFontSize);
    fprintf (fp, "主窗口字体大小=%d\n", iMainWindowFontSize);
    if (strUserLocale[0])
	fprintf (fp, "字体区域=%s\n", strUserLocale);
    else
	fprintf (fp, "#字体区域=zh_CN.gb18030\n");
#ifdef _USE_XFT
    fprintf (fp, "使用AA字体=%d\n", bUseAA);
#endif
    fprintf (fp, "使用粗体=%d\n", bUseBold);

    fprintf (fp, "\n[输出]\n");
    fprintf (fp, "数字后跟半角符号=%d\n", bEngPuncAfterNumber);
    fprintf (fp, "Enter键行为=%d\n", enterToDo);
    fprintf (fp, "分号键行为=%d\n", (int) semicolonToDo);
    fprintf (fp, "大写字母输入英文=%d\n", bEngAfterCap);
    fprintf (fp, "转换英文中的标点=%d\n", bConvertPunc);
    fprintf (fp, "联想方式禁止翻页=%d\n", bDisablePagingInLegend);
    //fprintf (fp, "LumaQQ支持=%d\n", bLumaQQ);

    fprintf (fp, "\n[界面]\n");
    fprintf (fp, "候选词个数=%d\n", iMaxCandWord);
    fprintf (fp, "主窗口使用3D界面=%d\n", _3DEffectMainWindow);
    fprintf (fp, "输入条使用3D界面=%d\n", _3DEffectInputWindow);
    fprintf (fp, "主窗口隐藏模式=%d\n", (int) hideMainWindow);
    fprintf (fp, "显示虚拟键盘=%d\n", (int) bShowVK);
    //fprintf (fp, "自动隐藏输入条=%d\n", bAutoHideInputWindow);
    fprintf (fp, "输入条居中=%d\n", bCenterInputWindow);
    fprintf (fp, "首次显示输入条=%d\n", bShowInputWindowTriggering);
    fprintf (fp, "#输入条固定宽度(仅适用于码表输入法)，0表示不固定宽度\n");
    fprintf (fp, "输入条固定宽度=%d\n", iFixedInputWindowWidth);
    fprintf (fp, "输入条偏移量X=%d\n", iOffsetX);
    fprintf (fp, "输入条偏移量Y=%d\n", iOffsetY);
    fprintf (fp, "序号后加点=%d\n", bPointAfterNumber);
    fprintf (fp, "显示打字速度=%d\n", bShowUserSpeed);
    fprintf (fp, "显示版本=%d\n", bShowVersion);

    fprintf (fp, "光标色=%d %d %d\n", cursorColor.color.red >> 8, cursorColor.color.green >> 8, cursorColor.color.blue >> 8);
    fprintf (fp, "主窗口背景色=%d %d %d\n", mainWindowColor.backColor.red >> 8, mainWindowColor.backColor.green >> 8, mainWindowColor.backColor.blue >> 8);
    fprintf (fp, "主窗口线条色=%d %d %d\n", mainWindowLineColor.color.red >> 8, mainWindowLineColor.color.green >> 8, mainWindowLineColor.color.blue >> 8);
    fprintf (fp, "主窗口输入法名称色=%d %d %d %d %d %d %d %d %d\n", IMNameColor[0].color.red >> 8, IMNameColor[0].color.green >> 8, IMNameColor[0].color.blue >> 8, IMNameColor[1].color.red >> 8, IMNameColor[1].color.green >> 8,
	     IMNameColor[1].color.blue >> 8, IMNameColor[2].color.red >> 8, IMNameColor[2].color.green >> 8, IMNameColor[2].color.blue >> 8);
    fprintf (fp, "输入窗背景色=%d %d %d\n", inputWindowColor.backColor.red >> 8, inputWindowColor.backColor.green >> 8, inputWindowColor.backColor.blue >> 8);
    fprintf (fp, "输入窗提示色=%d %d %d\n", messageColor[0].color.red >> 8, messageColor[0].color.green >> 8, messageColor[0].color.blue >> 8);
    fprintf (fp, "输入窗用户输入色=%d %d %d\n", messageColor[1].color.red >> 8, messageColor[1].color.green >> 8, messageColor[1].color.blue >> 8);
    fprintf (fp, "输入窗序号色=%d %d %d\n", messageColor[2].color.red >> 8, messageColor[2].color.green >> 8, messageColor[2].color.blue >> 8);
    fprintf (fp, "输入窗第一个候选字色=%d %d %d\n", messageColor[3].color.red >> 8, messageColor[3].color.green >> 8, messageColor[3].color.blue >> 8);
    fprintf (fp, "#该颜色值只用于拼音中的用户自造词\n");
    fprintf (fp, "输入窗用户词组色=%d %d %d\n", messageColor[4].color.red >> 8, messageColor[4].color.green >> 8, messageColor[4].color.blue >> 8);
    fprintf (fp, "输入窗提示编码色=%d %d %d\n", messageColor[5].color.red >> 8, messageColor[5].color.green >> 8, messageColor[5].color.blue >> 8);
    fprintf (fp, "#五笔、拼音的单字/系统词组均使用该颜色\n");
    fprintf (fp, "输入窗其它文本色=%d %d %d\n", messageColor[6].color.red >> 8, messageColor[6].color.green >> 8, messageColor[6].color.blue >> 8);
    fprintf (fp, "输入窗线条色=%d %d %d\n", inputWindowLineColor.color.red >> 8, inputWindowLineColor.color.green >> 8, inputWindowLineColor.color.blue >> 8);
    fprintf (fp, "输入窗箭头色=%d %d %d\n", colorArrow.red >> 8, colorArrow.green >> 8, colorArrow.blue >> 8);
    fprintf (fp, "虚拟键盘窗背景色=%d %d %d\n", VKWindowColor.backColor.red >> 8, VKWindowColor.backColor.green >> 8, VKWindowColor.backColor.blue >> 8);
    fprintf (fp, "虚拟键盘窗字母色=%d %d %d\n", VKWindowAlphaColor.color.red >> 8, VKWindowAlphaColor.color.green >> 8, VKWindowAlphaColor.color.blue >> 8);
    fprintf (fp, "虚拟键盘窗符号色=%d %d %d\n", VKWindowFontColor.color.red >> 8, VKWindowFontColor.color.green >> 8, VKWindowFontColor.color.blue >> 8);

    fprintf (fp, "\n#除了“中英文快速切换键”外，其它的热键均可设置为两个，中间用空格分隔\n");
    fprintf (fp, "[热键]\n");
    fprintf (fp, "打开/关闭输入法=CTRL_SPACE\n");
    fprintf (fp, "#中英文快速切换键 可以设置为L_CTRL R_CTRL L_SHIFT R_SHIFT L_SUPER R_SUPER\n");
    fprintf (fp, "中英文快速切换键=L_CTRL\n");
    fprintf (fp, "双击中英文切换=%d\n", bDoubleSwitchKey);
    fprintf (fp, "击键时间间隔=%u\n", iTimeInterval);
    fprintf (fp, "光标跟随=CTRL_K\n");
    fprintf (fp, "GBK支持=CTRL_M\n");
    fprintf (fp, "GBK繁体切换键=CTRL_ALT_F\n");
    fprintf (fp, "联想=CTRL_L\n");
    fprintf (fp, "反查拼音=CTRL_ALT_E\n");
    fprintf (fp, "全半角=SHIFT_SPACE\n");
    fprintf (fp, "中文标点=ALT_SPACE\n");
    fprintf (fp, "上一页=- ,\n");
    fprintf (fp, "下一页== .\n");
    fprintf (fp, "第二三候选词选择键=SHIFT\n");

    fprintf (fp, "\n[输入法]\n");
    fprintf (fp, "使用拼音=%d\n", bUsePinyin);
    fprintf (fp, "拼音名称=%s\n", strNameOfPinyin);
    fprintf (fp, "使用双拼=%d\n", bUseSP);
    fprintf (fp, "双拼名称=%s\n", strNameOfShuangpin);
    fprintf (fp, "默认双拼方案=%s\n", strDefaultSP);
    fprintf (fp, "使用区位=%d\n", bUseQW);
    fprintf (fp, "区位名称=%s\n", strNameOfQuwei);
    fprintf (fp, "使用码表=%d\n", bUseTable);
    fprintf (fp, "提示词库中的词组=%d\n", bPhraseTips);

    fprintf (fp, "\n[拼音]\n");
    fprintf (fp, "使用全拼=%d\n", bFullPY);
    fprintf (fp, "拼音自动组词=%d\n", bPYCreateAuto);
    fprintf (fp, "保存自动组词=%d\n", bPYSaveAutoAsPhrase);
    fprintf (fp, "增加拼音常用字=CTRL_8\n");
    fprintf (fp, "删除拼音常用字=CTRL_7\n");
    fprintf (fp, "删除拼音用户词组=CTRL_DELETE\n");
    fprintf (fp, "#拼音以词定字键，等号后面紧接键，不要有空格\n");
    fprintf (fp, "拼音以词定字键=%c%c\n", cPYYCDZ[0], cPYYCDZ[1]);
    fprintf (fp, "#重码调整方式说明：0-->不调整  1-->快速调整  2-->按频率调整\n");
    fprintf (fp, "拼音单字重码调整方式=%d\n", baseOrder);
    fprintf (fp, "拼音词组重码调整方式=%d\n", phraseOrder);
    fprintf (fp, "拼音常用词重码调整方式=%d\n", freqOrder);
    fprintf (fp, "模糊an和ang=%d\n", MHPY_C[0].bMode);
    fprintf (fp, "模糊en和eng=%d\n", MHPY_C[1].bMode);
    fprintf (fp, "模糊ian和iang=%d\n", MHPY_C[2].bMode);
    fprintf (fp, "模糊in和ing=%d\n", MHPY_C[3].bMode);
    fprintf (fp, "模糊ou和u=%d\n", MHPY_C[4].bMode);
    fprintf (fp, "模糊uan和uang=%d\n", MHPY_C[5].bMode);
    fprintf (fp, "模糊c和ch=%d\n", MHPY_S[0].bMode);
    fprintf (fp, "模糊f和h=%d\n", MHPY_S[1].bMode);
    fprintf (fp, "模糊l和n=%d\n", MHPY_S[2].bMode);
    fprintf (fp, "模糊s和sh=%d\n", MHPY_S[3].bMode);
    fprintf (fp, "模糊z和zh=%d\n", MHPY_S[4].bMode);

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

	    if (MyStrcmp (str, "版本=")) {
		pstr += 5;

		if (!strcasecmp (FCITX_VERSION, pstr))
		    bRetVal = True;
	    }
	    else if (MyStrcmp (str, "主窗口位置X=")) {
		pstr += 12;
		iMainWindowX = atoi (pstr);
		if (iMainWindowX < 0)
		    iMainWindowX = 0;
		else if ((iMainWindowX + MAINWND_WIDTH) > DisplayWidth (dpy, iScreen))
		    iMainWindowX = DisplayWidth (dpy, iScreen) - MAINWND_WIDTH;
	    }
	    else if (MyStrcmp (str, "主窗口位置Y=")) {
		pstr += 12;
		iMainWindowY = atoi (pstr);
		if (iMainWindowY < 0)
		    iMainWindowY = 0;
		else if ((iMainWindowY + MAINWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iMainWindowY = DisplayHeight (dpy, iScreen) - MAINWND_HEIGHT;
	    }
	    else if (MyStrcmp (str, "输入窗口位置X=")) {
		pstr += 14;
		iInputWindowX = atoi (pstr);
		if (iInputWindowX < 0)
		    iInputWindowX = 0;
		else if ((iInputWindowX + iInputWindowWidth) > DisplayWidth (dpy, iScreen))
		    iInputWindowX = DisplayWidth (dpy, iScreen) - iInputWindowWidth - 3;
	    }
	    else if (MyStrcmp (str, "输入窗口位置Y=")) {
		pstr += 14;
		iInputWindowY = atoi (pstr);
		if (iInputWindowY < 0)
		    iInputWindowY = 0;
		else if ((iInputWindowY + INPUTWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iInputWindowY = DisplayHeight (dpy, iScreen) - iInputWindowHeight;
	    }
	    else if (MyStrcmp (str, "全角=")) {
		pstr += 5;
		bCorner = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "中文标点=")) {
		pstr += 9;
		bChnPunc = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "GBK=")) {
		pstr += 4;
		bUseGBK = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "光标跟随=")) {
		pstr += 9;
		bTrackCursor = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "联想=")) {
		pstr += 5;
		bUseLegend = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "当前输入法=")) {
		pstr += 11;
		iIMIndex = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "禁止键盘切换=")) {
		pstr += 13;
		bLocked = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "简洁模式=")) {
		pstr += 9;
		bCompactMainWindow = atoi (pstr);
	    }
	    else if (MyStrcmp (str, "GBK繁体=")) {
		pstr += 8;
		bUseGBKT = atoi (pstr);
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
	fprintf (stderr, "\n无法创建文件 profile!\n");
	return;
    }

    fprintf (fp, "版本=%s\n", FCITX_VERSION);
    fprintf (fp, "主窗口位置X=%d\n", iMainWindowX);
    fprintf (fp, "主窗口位置Y=%d\n", iMainWindowY);
    fprintf (fp, "输入窗口位置X=%d\n", iInputWindowX);
    fprintf (fp, "输入窗口位置Y=%d\n", iInputWindowY);
    fprintf (fp, "全角=%d\n", bCorner);
    fprintf (fp, "中文标点=%d\n", bChnPunc);
    fprintf (fp, "GBK=%d\n", bUseGBK);
    fprintf (fp, "光标跟随=%d\n", bTrackCursor);
    fprintf (fp, "联想=%d\n", bUseLegend);
    fprintf (fp, "当前输入法=%d\n", iIMIndex);
    fprintf (fp, "禁止键盘切换=%d\n", bLocked);
    fprintf (fp, "简洁模式=%d\n", bCompactMainWindow);

    fprintf (fp, "GBK繁体=%d\n", bUseGBKT);

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
 * 计算文件中有多少行
 * 注意:文件中的空行也做为一行处理
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
	switchKey = XKeysymToKeycode(dpy, XK_Control_R);
    else if (!strcasecmp (str, "R_SHIFT"))
	switchKey = XKeysymToKeycode(dpy, XK_Shift_R);
    else if (!strcasecmp (str, "L_SHIFT"))
	switchKey = XKeysymToKeycode(dpy, XK_Shift_L);
    else if (!strcasecmp (str, "R_SUPER"))
	switchKey = XKeysymToKeycode(dpy, XK_Super_R);
    else if (!strcasecmp (str, "L_SUPER"))
	switchKey = XKeysymToKeycode(dpy, XK_Super_L);
    else
        switchKey = XKeysymToKeycode(dpy, XK_Control_L);
}

void SetTriggerKeys (char *str)
{
    int             i;
    char            strKey[2][30];
    char           *p;

    //首先来判断用户设置了几个热键，最多为2    
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
	else if (MyStrcmp (strKey[i], "SUPER_")) {
	    Trigger_Keys[i].modifier = Trigger_Keys[i].modifier | Mod4Mask;
	    Trigger_Keys[i].modifier_mask = Trigger_Keys[i].modifier_mask | Mod4Mask;
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
	else if (!strcasecmp (p, "LSUPER"))
	    Trigger_Keys[i].keysym = XK_Super_L;
	else if (!strcasecmp (p, "RSUPER"))
	    Trigger_Keys[i].keysym = XK_Super_R;
	else
	    Trigger_Keys[i].keysym = XK_space;
    }
}

Bool CheckHZCharset (char *strHZ)
{
    if (!bUseGBK) {
	//GB2312的汉字编码规则为：第一个字节的值在0xA1到0xFE之间(实际为0xF7)，第二个字节的值在0xA1到0xFE之间
	//由于查到的资料说法不一，懒得核实，就这样吧
	int             i;

	for (i = 0; i < strlen (strHZ); i++) {
	    if ((unsigned char) strHZ[i] < (unsigned char) 0xA1 || (unsigned char) strHZ[i] > (unsigned char) 0xF7 || (unsigned char) strHZ[i + 1] < (unsigned char) 0xA1 || (unsigned char) strHZ[i + 1] > (unsigned char) 0xFE)
		return False;
	    i++;
	}
    }

    return True;
}

static char    *gGBKS2TTable = NULL;
static int      gGBKS2TTableSize = -1;

/**
 * 该函数装载data/gbks2t.tab的简体转繁体的码表，
 * 然后按码表将GBK字符转换成GBK繁体字符。
 *
 * WARNING： 该函数返回新分配内存字符串，请调用者
 * 注意释放。
 */
char           *ConvertGBKSimple2Tradition (char *strHZ)
{
    FILE           *fp;
    char           *ret;
    char            strPath[PATH_MAX];
    int             i, len;
    unsigned int    idx;

    if (strHZ == NULL)
	return NULL;

    if (!gGBKS2TTable) {
	len = 0;

	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, TABLE_GBKS2T);
	fp = fopen (strPath, "rb");
	if (!fp) {
	    ret = (char *) malloc (sizeof (char) * (strlen (strHZ) + 1));
	    strcpy (ret, strHZ);
	    return ret;
	}

	fseek (fp, 0, SEEK_END);
	fgetpos (fp, (fpos_t *) & len);
	if (len > 0) {
	    gGBKS2TTable = (char *) malloc (sizeof (char) * len);
	    gGBKS2TTableSize = len;
	    fseek (fp, 0, SEEK_SET);
	    fread (gGBKS2TTable, sizeof (char), len, fp);
	}
	fclose (fp);
    }

    i = 0;
    len = strlen (strHZ);
    ret = (char *) malloc (sizeof (char) * (len + 1));
    for (; i < len; ++i) {
	if (i < (len - 1))
	    if ((unsigned char) strHZ[i] >= (unsigned char) 0x81
		&& (unsigned char) strHZ[i] <= (unsigned char) 0xfe &&
		(((unsigned char) strHZ[i + 1] >= (unsigned char) 0x40 && (unsigned char) strHZ[i + 1] <= (unsigned char) 0x7e) || ((unsigned char) strHZ[i + 1] > (unsigned char) 0x7f && (unsigned char) strHZ[i + 1] <= (unsigned char) 0xfe))) {
		idx = (((unsigned char) strHZ[i] - (unsigned char) 0x81)
		       * (unsigned char) 0xbe + ((unsigned char) strHZ[i + 1] - (unsigned char) 0x40)
		       - ((unsigned char) strHZ[i + 1] / (unsigned char) 0x80)) * 2;
		if (idx >= 0 && idx < gGBKS2TTableSize - 1) {
		    //if ((unsigned char)gGBKS2TTable[idx] != (unsigned char)0xa1 && (unsigned char) gGBKS2TTable[idx + 1] != (unsigned char) 0x7f) {
		    if ((unsigned char) gGBKS2TTable[idx + 1] != (unsigned char) 0x7f) {
			ret[i] = gGBKS2TTable[idx];
			ret[i + 1] = gGBKS2TTable[idx + 1];
			i += 1;
			continue;
		    }
		}
	    }
	ret[i] = strHZ[i];
    }
    ret[len] = '\0';

    return ret;
}

int CalHZIndex (char *strHZ)
{
    return (strHZ[0] + 127) * 255 + strHZ[1] + 128;
}
