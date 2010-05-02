/***************************************************************************
 *   Copyright (C) 2005 by Yunfan                                          *
 *   yunfan_zg@163.com                                                     *
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
 *   t3swing  t3swing@sina.com    20091117                                 *
 ***************************************************************************/

/* 鼠标点击logo图标的时候产生的窗口 */

#ifndef _MENUWINDOW_H_
#define _MENUWINDOW_H_

#include "ui.h"
#include "core/xim.h"
#include "core/ime.h"
#include "im/pinyin/PYFA.h"
#include "MainWindow.h"
#include "TrayWindow.h"
#include "tools/tools.h"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#define MENU_WINDOW_WIDTH	120
#define MENU_WINDOW_HEIGHT	140

#define IM_MENU_WINDOW_WIDTH	110
#define IM_MENU_WINDOW_HEIGHT	158

#define SKIN_MENU_WINDOW_WIDTH	110
#define SKIN_MENU_WINDOW_HEIGHT	200

#define VK_MENU_WINDOW_WIDTH	110
#define VK_MENU_WINDOW_HEIGHT	158

#define CHAR_COLOR 0x111111
#define CHAR_SELECT_COLOR (0xFFFFFF-0x111111)

//背景色 默认银灰色
#define BG_COLOR 0xDCDCDC
//选中后的背景色 深蓝色
#define BG_SELECT_COLOR 0x0A2465

typedef enum
{
	menushell, //暂时只有菜单项和分割线两种类型
	divline
}shellType;

//菜单项属性
typedef	struct 
{
	char tipstr[24];
	int  isselect;
	shellType type;
}menuShell;

typedef struct
{
	int pos_x;
	int pos_y;
	int width;
	int height;
	Window menuWindow;
	unsigned int bgcolor;
	unsigned int bgselectcolor;
	unsigned int charcolor;
	unsigned int charselectcolor;
#if 1//#ifdef _USE_XFT
	XftFont *menuFont;
#else
	XFontSet menuFontset;
#endif	
	menuShell shell[12];
	int useItemCount;
	int	mark;
}xlibMenu;

extern xlibMenu mainMenu,imMenu,vkMenu,skinMenu;
extern INT8     iIMIndex;
Bool            CreateMenuWindow (void);
void            InitMenu(void );
void            InitMenuWindowColor (void);
void            DisplayMenuWindow (void);
void            DrawMenuWindow (void);
void GetMenuHeight(Display * dpy,xlibMenu * Menu);

Bool CreateMenuWindow (void);
Bool CreateImMenuWindow (void);
Bool CreateSkinMenuWindow (void);

void DrawXlibMenu(Display * dpy,xlibMenu * Menu);
void DrawDivLine(Display * dpy,xlibMenu * Menu,int line_y);
void DisplayText(Display * dpy,xlibMenu * Menu,int shellindex,int line_y);
int selectShellIndex(xlibMenu * Menu,int y);
void menuMark(Display * dpy,xlibMenu * Menu,int y,int i);
void clearSelectFlag(xlibMenu * Menu);
void MainMenuEvent(int x,int y);
void IMMenuEvent(int x,int y);
void VKMenuEvent(int x,int y);
void SkinMenuEvent(int x,int y);

#endif
