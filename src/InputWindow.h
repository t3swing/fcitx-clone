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
#ifndef _INPUT_WINDOW_H
#define _INPUT_WINDOW_H

#include <X11/Xlib.h>

#define INPUTWND_STARTX	300
#define INPUTWND_WIDTH	50
#define INPUTWND_STARTY	420
#define INPUTWND_HEIGHT	40

#define INPUTWND_START_POS_UP	8
#define INPUTWND_START_POS_DOWN	8

#define MESSAGE_MAX_LENGTH	300	//����������ʾ������ȣ����ַ���

/* ������������ʾ�����ݷ�Ϊ���¼��� */
#define MESSAGE_TYPE_COUNT	7

typedef enum {
    MSG_TIPS,			//��ʾ�ı�
    MSG_INPUT,			//�û�������
    MSG_INDEX,			//��ѡ��ǰ������
    MSG_FIRSTCAND,		//��һ����ѡ��
    MSG_USERPHR,		//�û�����
    MSG_CODE,			//��ʾ�ı���
    MSG_OTHER			//�����ı�
} MSG_TYPE;

typedef struct {
    char            strMsg[MESSAGE_MAX_LENGTH + 1];
    MSG_TYPE        type;
} MESSAGE;

Bool            CreateInputWindow (void);
void            DisplayInputWindow (void);
void            InitInputWindowColor (void);
void            CalculateInputWindowHeight (void);
void            DrawCursor (int iPos);
void            DisplayMessageUp (void);
void            DisplayMessageDown (void);
void            DisplayMessage (void);
void            DrawInputWindow (void);
void            ResetInputWindow (void);

#endif
