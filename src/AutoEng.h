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
 * @file   AutoEng.h
 * @author Yuking yuking_net@sohu.com 
 * @date   2008-1-16
 * 
 * @brief �Զ��л���Ӣ��״̬
*
*
*/

#ifndef _AUTOENG_H
#define _AUTOENG_H

#include "xim.h"

#define MAX_AUTO_TO_ENG	10

typedef struct {
    char            str[MAX_AUTO_TO_ENG + 1];
} AUTO_ENG;

/** 
 * �� ~/.fcitx/!AutoEng.dat 
 * ����������ڣ���� /usr/local/share/fcitx/data/!AutoEng.dat��
 * ��ȡ��Ҫ�Զ�ת����Ӣ������״̬����������ݡ�
 */
void            LoadAutoEng (void);

/** 
 * �ͷ������Դ  
 */
void            FreeAutoEng (void);
Bool            SwitchToEng (char *);

#endif
