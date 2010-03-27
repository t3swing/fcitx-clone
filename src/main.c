/***************************************************************************
 *   小企鹅中文输入法(Free Chinese Input Toys for X, FCITX)                   *
 *   由Yuking(yuking_net@sohu.com)编写                                           *
 *   协议: GPL                                                              *
 *   FCITX( A Chinese XIM Input Method) by Yuking (yuking_net@sohu.com)    *
 *                                                                         *
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
 * @file   main.c
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 *
 * @brief  程序主入口
 *
 *
 */

#include <langinfo.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "version.h"

#include "main.h"
#include "MyErrorsHandlers.h"
#include "tools.h"
#include "ui.h"
#include "MainWindow.h"
#include "InputWindow.h"

#ifdef _ENABLE_TRAY
#include "TrayWindow.h"
#endif

#include "DBus.h"

#include "vk.h"
#include "ime.h"
#include "table.h"
#include "punc.h"
#include "py.h"
#include "sp.h"
#include "about.h"
#include "QuickPhrase.h"
#include "AutoEng.h"

#ifndef CODESET
#define CODESET 14
#endif
#include <pthread.h>

extern Display *dpy;
extern Window   inputWindow;
extern Window   ximWindow;

extern Bool	bUseDBus;

extern HIDE_MAINWINDOW hideMainWindow;

extern void* remoteThread(void*);

#ifdef _ENABLE_TRAY
extern tray_win_t tray;
#endif

extern pthread_rwlock_t plock;

int main (int argc, char *argv[])
{
    XEvent          event;
    int             c; 	//用于保存用户输入的参数
    Bool            bBackground = True;
    char	    *imname=(char *)NULL;
    pthread_t	    pid;

    SetMyExceptionHandler();		//处理事件

    /* 先初始化 X 再加载配置文件，因为设置快捷键从 keysym 转换到
     * keycode 的时候需要 Display
     */
    if (!InitX ())
	exit (1);

    /*加载用户配置文件，如果该文件不存在就从安装目录中拷贝
     * “/data/config”到“~/.fcitx/config”
     */
    LoadConfig (True);

    while((c = getopt(argc, argv, "cdDn:vh")) != -1) {
        switch(c){
	    case 'd':
                /* nothing to do */
                break;
            case 'D':
                bBackground = False;
                break;
	    case 'c':
		SaveConfig();
		return 0;
	    case 'n':
	    	imname=optarg;
		break;
            case 'v':	//输出版本号
                Version();
                return 0;
            case 'h':	//h或者其他任何不合法的参数均，输出参数帮助信息
            case '?':
                Usage();
                return 0;
        }
    }

    /*下面两行代码用于检查当前系统使用的是否是UTF字符集。相当于在字符终端执行
     * “locale charmap”
     */
    setlocale (LC_CTYPE, "");

#ifdef _ENABLE_DBUS
    if (bUseDBus && !InitDBus ())
	exit (5);
#endif

    /*创建字体。实际上，就是根据用户的设置，使用xft读取字体的相关信息。
     * xft是x11提供的处理字体的相关函数集
     */
    CreateFont ();
    //根据字体计算输入窗口的高度
    CalculateInputWindowHeight ();
    /*加载配置文件，这个配置文件不是用户配置的，而是用于记录fctix的运行状态的，
     * 比如是全角还是半角等等。
     */
    LoadProfile ();

    //加载字典文件
    LoadPuncDict ();
    //加载成语
    LoadQuickPhrase ();
    /*从用户配置目录中读取AutoEng.dat （如果不存在，
     * 则从 /usr/local/share/fcitx/data/AutoEng.dat）
     * 读取需要自动转换到英文输入状态的情况的数据
     */
    LoadAutoEng ();

    //以下是界面的处理

    if (!bUseDBus)
	CreateMainWindow ();	//创建主窗口，即输入法状态窗口
#ifdef _ENABLE_DBUS
    else
	registerProperties();
#endif

    CreateInputWindow ();	//创建输入窗口
    CreateVKWindow ();		//创建虚拟键盘窗口

    if (!bUseDBus)
	CreateAboutWindow ();	//创建关于窗口

    //处理颜色，即候选词窗口的颜色，也就是我们在配置文件中定义的那些颜色信息
    InitGC (inputWindow);

    //将本程序加入到输入法组，告诉系统，使用我输入字符
    SetIM ();

    if (!bUseDBus) {
	//处理主窗口的显示
	if (hideMainWindow != HM_HIDE) {
	    DisplayMainWindow ();
	    DrawMainWindow ();
	}
    }

    //初始化输入法
    if (!InitXIM (imname))
	exit (4);

    //以后台方式运行
    if (bBackground) {
	pid_t           id;

	id = fork ();
	if (id == -1) {
	    printf ("Can't run as a daemon！\n");
	    exit (1);
	}
	else if (id > 0)
	    exit (0);
    }

#ifdef _ENABLE_RECORDING
    OpenRecording(True);
#endif

#ifdef _ENABLE_DBUS
    dbus_threads_init_default();
#endif
	
	if (pthread_rwlock_init(&plock,NULL) != 0) {
      fprintf(stderr,"lock init failed\n");
      exit(-1);
    }

    pthread_create(&pid, NULL, remoteThread, NULL);

#ifdef _ENABLE_DBUS
    if (bUseDBus)
        pthread_create(&pid, NULL, (void *)DBusLoop, NULL);
#endif

#ifdef _ENABLE_TRAY
    tray.window = (Window) NULL;
    if (!bUseDBus) {
	CreateTrayWindow ();		//创建系统托盘窗口
    	DrawTrayWindow (INACTIVE_ICON, 0, 0, TRAY_ICON_WIDTH, TRAY_ICON_HEIGHT);	//显示托盘图标
    }
#endif

    //主循环，即XWindow的消息循环
    for (;;) {
	XNextEvent (dpy, &event);			//等待一个事件发生
	    
	if (XFilterEvent (&event, None) == False)
	    MyXEventHandler (&event);		//处理X事件
    }

    return 0;
}

void Usage ()
{
    printf("Usage: fcitx [OPTION]\n"
           "\t-d\t\trun as daemon(default)\n"
           "\t-D\t\tdon't run as daemon\n"
   	   "\t-c\t\t(re)create config file in home directory and then exit\n"
	   "\t-n[im name]\trun as specified name\n"
           "\t-v\t\tdisplay the version information and exit\n"
           "\t-h\t\tdisplay this help and exit\n");
}

void Version ()
{
    printf ("fcitx version: %s-%s\n", FCITX_VERSION, USE_XFT);
}
