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

#include "MenuWindow.h"

#include <ctype.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#endif
#include <iconv.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#include "skin.h"

extern Display *dpy;
extern _Skin_Config skin_config;
extern _Skin_Dir skinBuf[10];
extern int skinCount;

extern int   iMainWindowX;
extern int   iMainWindowY;

extern int      iScreen;

extern int      iVKWindowFontSize;

#ifdef _USE_XFT
extern XftFont *xftMainWindowFont;
extern XftFont *xftMainWindowFontEn;
extern XftFont        *xftMenuFont;
#else
extern XFontSet fontSetMainWindow;
#endif

#ifdef _USE_XFT
extern XftFont *xftFont;
extern XftFont *xftFontEn;
#else
extern XFontSet fontSet;
#endif

#ifdef _USE_XFT
extern XftFont *xftVKWindowFont;
#else
extern XFontSet fontSetVKWindow;
#endif

xlibMenu mainMenu,imMenu,vkMenu,skinMenu; 
					
void InitMenuDefault(xlibMenu * Menu)
{
	Menu->pos_x=100;
	Menu->pos_y=100;
	Menu->width=110;
	Menu->bgcolor=0xDCDCDC;
	Menu->bgselectcolor=0x0A2465;
	Menu->charcolor=0x111111;
	Menu->charselectcolor=(0xFFFFFF-0x111111);
	Menu->mark=-1;
}

void SetMeueShell(menuShell * shell,char * tips,int isselect,shellType type)
{
	strcpy(shell->tipstr,tips);
	shell->isselect=isselect;
	shell->type=type;
}

int CreateXlibMenu(Display * dpy,xlibMenu * Menu)
{
	XSetWindowAttributes attrib;
	unsigned long   attribmask;
	int             iBackPixel;
	WINDOW_COLOR     menuWindowColor = { NULL, NULL, {0, 220 << 8, 220 << 8, 220 << 8}};
					
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(menuWindowColor.backColor)))
		iBackPixel = menuWindowColor.backColor.pixel;
    else
		iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));
	//开始只创建一个简单的窗口不做任何动作
    Menu->menuWindow =XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 
									0, 0,
									MENU_WINDOW_WIDTH,MENU_WINDOW_HEIGHT,
									0,WhitePixel (dpy, DefaultScreen (dpy)),iBackPixel);	
    if (Menu->menuWindow == (Window) NULL)
	{
		//printf("CreateMenuWindow failed\n");
		return False;
	}
	
  	attrib.override_redirect = True;
	attribmask = CWOverrideRedirect;
	XChangeWindowAttributes (dpy, Menu->menuWindow, attribmask, &attrib);

    XSelectInput (dpy, Menu->menuWindow, ExposureMask | ButtonPressMask | ButtonReleaseMask  | PointerMotionMask );
    //XFlush( dpy);

    return True;
}

void GetMenuHeight(Display * dpy,xlibMenu * Menu)
{
	int i=0;
	int winheight=0;
	int fontheight=0;
	winheight = 8+8;//菜单头和尾都空8个pixel
	fontheight=FontHeight(Menu->menuFont);
	for(i=0;i<Menu->useItemCount;i++)
	{
		//printf("%d %d %x %x %x %s\n",winheight,Menu->width,Menu->bgcolor,Menu->charcolor,Menu->charselectcolor,Menu->shell[i].tipstr);
		if( Menu->shell[i].type == menushell)
			winheight=winheight+6+fontheight;
		else if( Menu->shell[i].type == divline)
			winheight+=3;
	}
	Menu->height=winheight;
}
//根据Menu内容来绘制菜单内容
void DrawXlibMenu(Display * dpy,xlibMenu * Menu)
{
	int i=0;
	int fontheight;
	int pos_y=0;

	fontheight=FontHeight(Menu->menuFont);

	GetMenuHeight(dpy,Menu);
	//printf("%d %d\n",Menu->height,Menu->useItemCount);
	//从第8像素点处开始绘图
	pos_y=8;
	for (i=0;i<Menu->useItemCount;i++)
	{
		if( Menu->shell[i].type == menushell)
		{
			DisplayText( dpy,Menu,i,pos_y);
			if(Menu->mark == i)//void menuMark(Display * dpy,xlibMenu * Menu,int y,int i)
				menuMark(dpy,Menu,pos_y,i);
			pos_y=pos_y+6+fontheight;
		}
		else if( Menu->shell[i].type == divline)
		{
			DrawDivLine(dpy,Menu,pos_y);
			pos_y+=5;
		}
	}
	XMoveResizeWindow(dpy, Menu->menuWindow, Menu->pos_x,Menu->pos_y,Menu->width, Menu->height);
}

void DisplayXlibMenu(Display * dpy,xlibMenu * Menu)
{
	//clearSelectFlag(Menu);
	XMapRaised (dpy, Menu->menuWindow);	     
}

void DrawDivLine(Display * dpy,xlibMenu * Menu,int line_y)
{
	//printf("DrawDivLine============\n");
	//分割线的颜色写死了，浅灰色
	GC gc = XCreateGC( dpy, Menu->menuWindow, 0, NULL );
	XSetForeground(dpy,gc,0xDDDDDD );
	XDrawLine(dpy, Menu->menuWindow, gc,10, line_y+1, Menu->width-20, line_y+1);
	XSetForeground(dpy,gc,0x949494);
	XDrawLine(dpy ,Menu->menuWindow, gc, 5, line_y+2, Menu->width-10, line_y+2);
	XSetForeground(dpy,gc,0xDDDDDD);
	XDrawLine(dpy, Menu->menuWindow, gc,10, line_y+3, Menu->width-20, line_y+3);
	XFreeGC (dpy, gc);
}

void menuMark(Display * dpy,xlibMenu * Menu,int y,int i)
{
	GC gc ;
	gc=XCreateGC( dpy, Menu->menuWindow, 0, NULL );
	if(Menu->shell[i].isselect == 0)
		XSetForeground(dpy,gc,0x0A2465 );
	else
		XSetForeground(dpy,gc,0xDCDCDC );
	XFillArc(dpy, Menu->menuWindow, gc,  3, y+5, 11, 11, 0, 360*64);
	XFreeGC (dpy, gc);
}

/*
* 显示菜单上面的文字信息,只需要指定窗口,窗口宽度,需要显示文字的上边界,字体,显示的字符串和是否选择(选择后反色)
* 其他都固定,如背景和文字反色不反色的颜色,反色框和字的位置等
*/
void DisplayText(Display * dpy,xlibMenu * Menu,int shellindex,int line_y)
{
	XColor color;
	unsigned long backcolor;
	int fontheight;
	//char tmpstr[128];
	MESSAGE_COLOR   FontColor = { NULL, {0, 0, 0, 0}};
	MESSAGE_COLOR   fontSelectColor = { NULL, {0,  220 << 8,  220 << 8,  220 << 8}};
	GC gc ;

	gc=XCreateGC( dpy, Menu->menuWindow, 0, NULL );
	
	if(Menu->shell[shellindex].isselect ==1)
	{	
		backcolor=Menu->bgselectcolor;
		color=fontSelectColor.color;
	}
	else //( Menu->shell[shellindex].isselect == 0)//未反色
	{
		backcolor=Menu->bgcolor;
		color=FontColor.color;
	}
	
	fontheight=FontHeight(Menu->menuFont);
	
	//printf("%s\n",Menu->shell[shellindex].tipstr);
	
	XSetForeground(dpy,gc,backcolor);
	//左右留空5p,字体上留2p,下留4p
	XFillRectangle(dpy, Menu->menuWindow,gc,2,line_y,Menu->width-4,fontheight+6);
	//code_convert("utf-8","gb2312",Menu->shell[shellindex].tipstr,strlen(Menu->shell[shellindex].tipstr),tmpstr,64);
	//显示字串,离左边20p
	//OutputString ( Menu->menuWindow, Menu->menuFont,tmpstr ,20,line_y+2+fontheight, color);
	OutputString ( Menu->menuWindow, Menu->menuFont,Menu->shell[shellindex].tipstr ,20,line_y+2+fontheight, color);
	XFreeGC (dpy, gc);
}

/**
*返回鼠标指向的菜单在menu中是第多少项
*/
int selectShellIndex(xlibMenu * Menu,int y)
{
	int i;
	int winheight=8;
	int fontheight;
	
	fontheight=FontHeight(Menu->menuFont);
	for(i=0;i<12;i++)
	{
		if( Menu->shell[i].type == menushell)
		{
			if(y>winheight+1 && y<winheight+6+fontheight-1)
				return i;
			winheight=winheight+6+fontheight;
		}
		else if( Menu->shell[i].type == divline)
			winheight+=5;
	}
	return -1;
}

void colorRevese(xlibMenu * Menu,int shellIndex)
{
	int i;
	for(i=0;i<12;i++)
	{
		Menu->shell[i].isselect=0;
	}
	if(shellIndex>=0)
		Menu->shell[shellIndex].isselect=1;
}

void clearSelectFlag(xlibMenu * Menu)
{
	int i;
	for(i=0;i<12;i++)
	{
		Menu->shell[i].isselect=0;
	}
}


//以上为菜单的简单封装，下面为对菜单的操作部分
//=========================================================================================
//创建菜单窗口
Bool CreateMenuWindow( )
{
	Bool ret;
	InitMenuDefault(&mainMenu);
	mainMenu.width=MENU_WINDOW_WIDTH;
	SetMeueShell(&mainMenu.shell[0],"关于fcitx",0,menushell);
	SetMeueShell(&mainMenu.shell[1],"",0,divline);
	SetMeueShell(&mainMenu.shell[2],"皮肤切换      >",0,menushell);
	SetMeueShell(&mainMenu.shell[3],"输入法切换    >",0,menushell);
	SetMeueShell(&mainMenu.shell[4],"软键盘切换    >",0,menushell);
	SetMeueShell(&mainMenu.shell[5],"",0,divline);
	SetMeueShell(&mainMenu.shell[6],"fcitx配置   ...",0,menushell);
	SetMeueShell(&mainMenu.shell[7],"主页",0,menushell);
	mainMenu.useItemCount=8;
	ret=CreateXlibMenu(dpy,&mainMenu);
	return 0;
}

//创建输入法选择菜单窗口
Bool CreateImMenuWindow() 
{	
	Bool ret;
	InitMenuDefault(&imMenu);
	SetMeueShell(&imMenu.shell[0],"智能拼音",0,menushell);
	SetMeueShell(&imMenu.shell[1],"智能双拼",0,menushell);
	SetMeueShell(&imMenu.shell[2],"区位输入",0,menushell);
	SetMeueShell(&imMenu.shell[3],"五笔字型",0,menushell);
	SetMeueShell(&imMenu.shell[4],"五笔拼音",0,menushell);
	SetMeueShell(&imMenu.shell[5],"二笔",0,menushell);
	SetMeueShell(&imMenu.shell[6],"仓颉",0,menushell);
	SetMeueShell(&imMenu.shell[7],"晚风",0,menushell);
	SetMeueShell(&imMenu.shell[8],"冰蟾全息",0,menushell);
	SetMeueShell(&imMenu.shell[9],"自然码",0,menushell);
	SetMeueShell(&imMenu.shell[10],"电报码",0,menushell);
	
	imMenu.useItemCount=11;
	ret=CreateXlibMenu(dpy,&imMenu);
	return ret;
}  
 
//创建皮肤选择菜单窗口,皮肤菜单由于在窗口创建之初,信息不全,菜单结构在菜单显示之前再填充
Bool CreateSkinMenuWindow()
{
	Bool ret;
	InitMenuDefault(&skinMenu);
	ret=CreateXlibMenu(dpy,&skinMenu);
	return ret;
} 
	
//创建软键盘布局选择菜单窗口
Bool CreateVKMenuWindow()
{
	Bool ret;
	InitMenuDefault(&vkMenu);
	SetMeueShell(&vkMenu.shell[0],"西文半角",0,menushell);
	SetMeueShell(&vkMenu.shell[1],"全角符号",0,menushell);
	SetMeueShell(&vkMenu.shell[2],"希腊字母",0,menushell);
	SetMeueShell(&vkMenu.shell[3],"俄文字母",0,menushell);
	SetMeueShell(&vkMenu.shell[4],"数字序号",0,menushell);
	SetMeueShell(&vkMenu.shell[5],"数学符号",0,menushell);
	SetMeueShell(&vkMenu.shell[6],"数字符号",0,menushell);
	SetMeueShell(&vkMenu.shell[7],"特殊符号",0,menushell);
	SetMeueShell(&vkMenu.shell[8],"日文平假名",0,menushell);
	SetMeueShell(&vkMenu.shell[9],"日文片假名",0,menushell);
	SetMeueShell(&vkMenu.shell[10],"制表符",0,menushell);
		
	vkMenu.useItemCount=11;
	ret=CreateXlibMenu(dpy,&vkMenu);
	return ret;
	
}

//主菜单事件处理
void MainMenuEvent(int x,int y)
{
	int i,j;
	i=selectShellIndex(&mainMenu, y);
	//printf("i=%d\n",i);
	colorRevese(&mainMenu,i);
	DrawXlibMenu( dpy,&mainMenu);
	DisplayXlibMenu(dpy,&mainMenu);	
	
	switch(i)
	{
	//显示皮肤菜单
		case 2:
			skinMenu.menuFont=xftMenuFont ;
			skinMenu.useItemCount=skinCount;
			for(j=0;j<skinCount;j++)
			{
				strcpy(skinMenu.shell[j].tipstr,skinBuf[j].dirbase);
			}
			skinMenu.pos_x=mainMenu.pos_x;
			skinMenu.pos_y=mainMenu.pos_y;
		
			if( skinMenu.pos_x+ mainMenu.width+skinMenu.width > DisplayWidth(dpy,iScreen))
				skinMenu.pos_x=skinMenu.pos_x-skinMenu.width+4;
			else
				skinMenu.pos_x=skinMenu.pos_x+ mainMenu.width-4;
			
			if( skinMenu.pos_y+y+skinMenu.height >DisplayHeight(dpy, iScreen))
				skinMenu.pos_y=DisplayHeight(dpy, iScreen)-skinMenu.height-10;
			else 
				skinMenu.pos_y=y+skinMenu.pos_y-10;
			
			clearSelectFlag(&skinMenu);
			DrawXlibMenu( dpy,&skinMenu);
			DisplayXlibMenu(dpy,&skinMenu);	
			break;
		case 3:
			imMenu.menuFont=xftMenuFont ;
			imMenu.pos_x=mainMenu.pos_x;
			imMenu.pos_y=mainMenu.pos_y;
		
			if( imMenu.pos_x+ mainMenu.width+imMenu.width > DisplayWidth(dpy,iScreen))
				imMenu.pos_x=imMenu.pos_x-imMenu.width+4;
			else
				imMenu.pos_x=imMenu.pos_x+ mainMenu.width-4;
			
			if( imMenu.pos_y+y+imMenu.height >DisplayHeight(dpy, iScreen))
				imMenu.pos_y=DisplayHeight(dpy, iScreen)-imMenu.height-10;
			else 
				imMenu.pos_y=y+imMenu.pos_y-10;
			
			clearSelectFlag(&imMenu);
			DrawXlibMenu( dpy,&imMenu);
			DisplayXlibMenu(dpy,&imMenu);	
			break;
		case 4:
			vkMenu.menuFont=xftMenuFont ;
			vkMenu.pos_x=mainMenu.pos_x;
			vkMenu.pos_y=mainMenu.pos_y;
		
			if( vkMenu.pos_x+ mainMenu.width+vkMenu.width > DisplayWidth(dpy,iScreen))
				vkMenu.pos_x=vkMenu.pos_x-vkMenu.width+4;
			else
				vkMenu.pos_x=vkMenu.pos_x+ mainMenu.width-4;
			
			if( vkMenu.pos_y+y+vkMenu.height >DisplayHeight(dpy, iScreen))
				vkMenu.pos_y=DisplayHeight(dpy, iScreen)-vkMenu.height-10;
			else 
				vkMenu.pos_y=y+vkMenu.pos_y-10;
			
			clearSelectFlag(&vkMenu)	;
			DrawXlibMenu( dpy,&vkMenu);
			DisplayXlibMenu(dpy,&vkMenu);	
			break;
		default :
			break;
	}
	
	if(mainMenu.shell[2].isselect == 0)
		XUnmapWindow (dpy, skinMenu.menuWindow);
	if(mainMenu.shell[3].isselect == 0)
		XUnmapWindow (dpy, imMenu.menuWindow);
	if(mainMenu.shell[4].isselect == 0)
		XUnmapWindow (dpy, vkMenu.menuWindow);
}

//输入法菜单事件处理
void IMMenuEvent(int x,int y)
{
	int i;
	i=selectShellIndex(&imMenu, y);

	//printf("i=%d\n",i);
	colorRevese(&imMenu,i);
	DrawXlibMenu(dpy,&imMenu);
	DisplayXlibMenu(dpy,&imMenu);	
}

//虚拟键盘菜单事件处理
void VKMenuEvent(int x,int y)
{
	int i;
	i=selectShellIndex(&vkMenu, y);
	//printf("i=%d\n",i);
	colorRevese(&vkMenu,i);
	DrawXlibMenu(dpy,&vkMenu);
	DisplayXlibMenu(dpy,&vkMenu);	
}

//皮肤菜单事件处理
void SkinMenuEvent(int x,int y)
{
	int i;
	i=selectShellIndex(&skinMenu, y);
	//printf("i=%d\n",i);
	colorRevese(&skinMenu,i);
	DrawXlibMenu(dpy,&skinMenu);
	DisplayXlibMenu(dpy,&skinMenu);	
}

