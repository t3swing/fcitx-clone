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
/**
 * @file   skin.h
 * @author Yuking yuking_net@sohu.com  t3swing  t3swing@sina.com
 *        
 * @date   2009-10-9
 * 
 * @brief  皮肤设置相关定义及初始化加载工作
 * 
 * 
 */

#ifndef _SKIN_H
#define _SKIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/xpm.h>
#include <dirent.h>
#include <sys/stat.h>
#include "ui.h"
#include "MainWindow.h"
#include "InputWindow.h"


#define ISDEFAULT (strcmp(skinType,"default") ==0 || strlen(skinType)==0)

typedef struct skin_xpm
{
	char xpmname[32];
	char enable;
	//图片绘画区域
	int  position_x;
	int  position_y;
	int  width;
	int  height;
	//按键响应区域
	int  response_x;
	int  response_y;
	int  response_w;
	int  response_h;
}skin_xpm;


typedef struct _SkinInfo
{
	char skinname[64];
	char skinversion[32];
	char skinauthor[64];
	char shinremark[128];
}_SkinInfo;

typedef struct _SkinFontOption
{
	int fontsize;
	char font[32];
	char inputcharcolor[16];
	char outputcharcolor[16];
	char charnocolor[16];
	char firstcharcolor[16];
	char othercolor[16];
}_SkinFontOption;

typedef struct _SkinMainBar
{
	skin_xpm mbbg_xpm;
	skin_xpm mbmask_xbm;
	skin_xpm logo_xpm;
	skin_xpm zhpunc_xpm;
	skin_xpm enpunc_xpm;
	skin_xpm chs_xpm;
	skin_xpm cht_xpm;
	skin_xpm halfcorner_xpm;
	skin_xpm fullcorner_xpm;
	skin_xpm unlock_xpm;
	skin_xpm lock_xpm;
	skin_xpm gbkoff_xpm;
	skin_xpm gbkon_xpm;
	skin_xpm lxoff_xpm;
	skin_xpm lxon_xpm;
	skin_xpm vkhide_xpm;
	skin_xpm vkshow_xpm;
	skin_xpm english_xpm;
	skin_xpm pinyin_xpm;
	skin_xpm shuangpin_xpm;
	skin_xpm quwei_xpm;
	skin_xpm wubi_xpm;
	skin_xpm mixpywb_xpm;
	skin_xpm erbi_xpm;
	skin_xpm cj_xpm;
	skin_xpm wanfeng_xpm;
	skin_xpm bingcan_xpm;
	skin_xpm ziran_xpm;
	skin_xpm otherim_xpm;
}_SkinMainBar;

typedef struct _SkinInputBar
{
	
	skin_xpm ibbg_xpm;
	skin_xpm ibmask_xbm;
	unsigned char resize;
	int	resizeposition_x;
	int resizeposition_w;
	int inputposition;
	int outputposition;
	int layoutleft;
	int layoutright;
	skin_xpm backarrow_xpm;
	skin_xpm forwardarrow_xpm;
}_SkinInputBar;


typedef struct _SkinTrayIcon
{
	skin_xpm active_xpm;
	skin_xpm inactive_xpm;
}_SkinTrayIcon;

/** 
* 配置文件结构,方便处理,结构固定
*/
typedef struct _Skin_Config
{	
		_SkinInfo SkinInfo;
		_SkinFontOption SkinFontOption;
		_SkinMainBar SkinMainBar;
		_SkinInputBar SkinInputBar;
		_SkinTrayIcon SkinTrayIcon;	
}_Skin_Config;

/**
*
*/
typedef struct _Skin_Dir
{
	char  dirbuf[128];
	char  dirbase[64];
}_Skin_Dir;
/**
*解析skin_xpm结构的值
*/

extern Display  *dpy;

extern MESSAGE_COLOR messageColor[];
extern MESSAGE_COLOR   inputWindowLineColor;

extern Pixmap  Bar;
extern Pixmap  Logo;
extern Pixmap  Punc[2];
extern Pixmap  GBK[2];
extern Pixmap  Corner[2];	
extern Pixmap  LX[2];	
extern Pixmap  GBKT[2];	
extern Pixmap  Lock[2];	
extern Pixmap  VK;
extern Pixmap  Input;
extern Pixmap  Prev;
extern Pixmap  Next;
extern Pixmap  English;
extern Pixmap  Pinyin;
extern Pixmap  Shuangpin;
extern Pixmap  Wubi;
extern Pixmap  Mix;
extern Pixmap  Erbi;
extern Pixmap  CangJi;
extern Pixmap  Wanfeng;
extern Pixmap  Bingcan;
extern Pixmap  Ziran;
extern Pixmap  Quwei;
extern Pixmap  Otherim;

extern Pixmap Barmask;
extern Pixmap  Logomask;
extern Pixmap  Puncmask[2];
extern Pixmap  GBKmask[2];
extern Pixmap  Cornermask[2];	
extern Pixmap  LXmask[2];	
extern Pixmap  GBKTmask[2];	
extern Pixmap  Lockmask[2];	
extern Pixmap  VKmask;
extern Pixmap  Inputmask;
extern Pixmap  Prevmask;
extern Pixmap  Nextmask;
extern Pixmap  Englishmask;
extern Pixmap  Pinyinmask;
extern Pixmap  Shuangpinmask;
extern Pixmap  Wubimask;
extern Pixmap  Mixmask;
extern Pixmap  Erbimask;
extern Pixmap  CangJimask;
extern Pixmap  Wanfengmask;
extern Pixmap  Bingcanmask;
extern Pixmap  Ziranmask;
extern Pixmap  Quweimask;
extern Pixmap  Otherimmask;


//輸入窗口圖標
extern Pixmap Input;
extern Pixmap Next;
extern Pixmap Prev;
extern Pixmap Inputmask;
extern Pixmap NextInputmask;
extern Pixmap PrevInputmask;

extern Window  mainWindow;
extern Window   inputWindow;

extern int skinCount;
extern _Skin_Config skin_config;
extern char  skinType[64];
extern _Skin_Dir skinBuf[10];
extern HIDE_MAINWINDOW hideMainWindow;
/*
**
*/
int WritePrivateProfileString(char *SelectName, char *KeyName, char *String, char *FileName);
int GetPrivateProfileString(FILE * fptmp,char *SelectName, char *KeyName,char *DefaultString, char *ReturnString,int Size);
int str2skin_xpm(char * str,skin_xpm * xpm);
int fill_skin_config_int(FILE * fptmp,char * SelectName,char * KeyName);
int fill_skin_config_str(FILE * fptmp,char * SelectName,char * KeyName,char * str);
int fill_skin_config_color(FILE * fptmp,char * SelectName,char * KeyName,XColor * color);

/**
* 加载皮肤配置文件
*/
int load_skin_config();

/**
*为窗口加载皮肤文件
*/
int load_xpm2main_window(XImage  *mask,XpmAttributes  xpmattrib);
int load_xpm2input_window(XImage  *mask,XpmAttributes  xpmattrib);
int load_xpm2tray_icon(XImage  *mask,XpmAttributes  xpmattrib);

int convertstr2color(XColor * color,char * str);
void putXpmImage(Window w,GC gc,Pixmap image,Pixmap  image_mask,int x, int y, int x1, int y1, int width, int height);
void putMainBarImage(GC gc,Pixmap image,Pixmap  image_mask,skin_xpm xpm);
void loadPixmap2MainWindow();
void loadPixmap2InputWindow();
void DisplayInputBar(int barlen);
void DisplaySkin(char * skinname);
int loadSkinDir();

extern Bool IsInBox (int x0, int y0, int x1, int y1, int x2, int y2);
extern Bool IsInRspArea(int x0,int y0,skin_xpm xpm);
#endif


