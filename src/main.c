/***************************************************************************
 *   С����������뷨(Free Chinese Input Toys for X, FCITX)                   *
 *   ��Yuking(yuking_net@sohu.com)��д                                           *
 *   Э��: GPL                                                              *
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
 * @brief  ���������
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

extern Display *dpy;
extern Window   inputWindow;

extern Bool     bIsUtf8;

extern HIDE_MAINWINDOW hideMainWindow;

int main (int argc, char *argv[])
{
    XEvent          event;
    int             c; 	//���ڱ����û�����Ĳ���
    Bool            bBackground = True;

    while((c = getopt(argc, argv, "dDvh")) != -1) {
        switch(c){
            case 'd':
                /* nothing to do */
                break;
            case 'D':
                bBackground = False;
                break;
            case 'v':	//����汾��
                Version();
                return 0;
            case 'h':	//h���������κβ��Ϸ��Ĳ��������������������Ϣ
            case '?':
                Usage();
                return 0;
        }
    }

    /*�������д������ڼ�鵱ǰϵͳʹ�õ��Ƿ���UTF�ַ������൱�����ַ��ն�ִ��
     * ��locale charmap��
     */
    setlocale (LC_CTYPE, "");
    bIsUtf8 = (strcmp (nl_langinfo (CODESET), "UTF-8") == 0);

    /* �ȳ�ʼ�� X �ټ��������ļ�����Ϊ���ÿ�ݼ��� keysym ת����
     * keycode ��ʱ����Ҫ Display
     */
    if (!InitX ())
	exit (1);

    /*�����û������ļ���ͨ���ǡ�~/.fcitx/config����������ļ������ھʹӰ�װĿ¼�п���
     * ��/data/config������~/.fcitx/config��
     */
    LoadConfig (True);

    /*�������塣ʵ���ϣ����Ǹ����û������ã�ʹ��xft��ȡ����������Ϣ��
     * xft��x11�ṩ�Ĵ����������غ�����
     */
    CreateFont ();
    //��������������봰�ڵĸ߶�
    CalculateInputWindowHeight ();
    /*���������ļ�����������ļ������û����õģ��������ڼ�¼fctix������״̬�ģ�
     * ������ȫ�ǻ��ǰ�ǵȵȡ�ͨ���ǡ�~/.fcitx/profile����������ļ������ھʹӰ�װ
     * Ŀ¼�п�����/data/profile������~/.fcitx/profile��
     */
    LoadProfile ();

    //�����ֵ��ļ�
    LoadPuncDict ();
    //���س���
    LoadQuickPhrase ();
    /*�� ~/.fcitx/AutoEng.dat ����������ڣ�
     * ��� /usr/local/share/fcitx/data/AutoEng.dat��
     * ��ȡ��Ҫ�Զ�ת����Ӣ������״̬�����������
     */
    LoadAutoEng ();

    //�����ǽ���Ĵ���

    CreateMainWindow ();	//���������ڣ������뷨״̬����
    CreateVKWindow ();		//������ѡ�ʴ���
    CreateInputWindow ();	//�������봰��
    CreateAboutWindow ();	//�������ڴ���

    //������ɫ������ѡ�ʴ��ڵ���ɫ��Ҳ���������ڡ�~/.fcitx/config���������Щ��ɫ��Ϣ
    InitGC (inputWindow);

    //����������뵽���뷨�飬����ϵͳ��ʹ���������ַ�
    SetIM ();

    //���������ڵ���ʾ
    if (hideMainWindow != HM_HIDE) {
	DisplayMainWindow ();
	DrawMainWindow ();
    }

    //��ʼ�����뷨
    if (!InitXIM (inputWindow))
	exit (4);

    //�Ժ�̨��ʽ����
    if (bBackground) {
	pid_t           id;

	id = fork ();
	if (id == -1) {
	    printf ("Can't run as a daemon��\n");
	    exit (1);
	}
	else if (id > 0)
	    exit (0);
    }

    //��ѭ������XWindow����Ϣѭ��
    for (;;) {
	XNextEvent (dpy, &event);					//�ȴ�һ���¼�����

	if (XFilterEvent (&event, None) == True)	//����ǳ�ʱ���ȴ���һ���¼�
	    continue;

	MyXEventHandler (&event);					//�����¼�
    }

    return 0;
}

void Usage ()
{
    printf("Usage: fcitx [OPTION]\n"
           "\t-d\trun as daemon(default)\n"
           "\t-D\tdon't run as daemon\n"
           "\t-v\tdisplay the version information and exit\n"
           "\t-h\tdisplay this help and exit\n");
}

void Version ()
{
    printf ("fcitx version: %s-%s\n", FCITX_VERSION, USE_XFT);
}
