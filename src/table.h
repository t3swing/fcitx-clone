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
#ifndef _TABLE_H
#define _TABLE_H

#include <X11/Xlib.h>
#include <limits.h>

#include "ime.h"

#define TABLE_CONFIG_FILENAME "tables.conf"

#define MAX_CODE_LENGTH  30
#define PHRASE_MAX_LENGTH 10
#define FH_MAX_LENGTH  10
#define TABLE_AUTO_SAVE_AFTER 1024
#define AUTO_PHRASE_COUNT 10000
#define SINGLE_HZ_COUNT 33000

typedef struct _RULE_RULE {
    unsigned char   iFlag;	// 1 --> ����   0 --> ����
    unsigned char   iWhich;	//�ڼ�����
    unsigned char   iIndex;	//�ڼ�������
} RULE_RULE;

typedef struct _RULE {
    unsigned char   iWords;	//���ٸ���
    unsigned char   iFlag;	//1 --> ���ڵ���iWords  0 --> ����iWords
    RULE_RULE      *rule;
} RULE;

typedef struct _TABLE {
    char            strPath[PATH_MAX];
    char            strSymbolFile[PATH_MAX];
    char            strName[MAX_IM_NAME + 1];
    char           *strInputCode;
    unsigned char   iCodeLength;
    unsigned char   iPYCodeLength;
    char           *strEndCode;	//��ֹ�������¸ü��൱������ü����ٰ�һ���ո�
    char           *strIgnoreChars;
    char            cMatchingKey;
    char            strSymbol[MAX_CODE_LENGTH + 1];
    char            cPinyin;	//����ü��󣬱�ʾ������ʱƴ��״̬
    unsigned char   bRule;

    RULE           *rule;	//��ʹ���
    INT8            iIMIndex;	//��¼������Ӧ�����뷨��˳��
    unsigned int    iRecordCount;
    ADJUSTORDER     tableOrder;

    Bool            bUsePY;	//ʹ��ƴ��
    INT8            iTableAutoSendToClient;	//�Զ�����
    INT8            iTableAutoSendToClientWhenNone;	//�����Զ�����
    Bool            bUseMatchingKey;	//�Ƿ�ģ��ƥ��
    Bool            bAutoPhrase;	//�Ƿ��Զ����
    INT8            iSaveAutoPhraseAfter;	//ѡ��N�κ󱣴��Զ����飬0-�����棬1-��������
    Bool            bAutoPhrasePhrase;	//�����Ƿ�������
    INT8            iAutoPhrase;	//�Զ���ʳ���
    Bool            bTableExactMatch;	//�Ƿ�ֻ��ʾ��ȷƥ��ĺ�ѡ��/��
    Bool            bPromptTableCode;	//������Ϻ��Ƿ���ʾ����
    int             iMaxPhraseAllowed;	//����������������0-����

    Bool            bHasPinyin;		//��Ǹ�������Ƿ���ƴ��
    char            choose[11];		//����ѡ���
} TABLE;

typedef struct _RECORD {
    char           *strCode;
    char           *strHZ;
    struct _RECORD *next;
    struct _RECORD *prev;
    unsigned int    iHit;
    unsigned int    iIndex;
    unsigned int    flag:1;
    unsigned int    bPinyin:1;
} RECORD;

/* ���ݼ�������һ���򵥵�������ָ��ü�����ʼ�ĵ�һ����¼ */
typedef struct _RECORD_INDEX {
    RECORD         *record;
    char            cCode;
} RECORD_INDEX;

typedef struct _FH {
    char            strFH[FH_MAX_LENGTH * 2 + 1];
} FH;

typedef struct _AUTOPHRASE {
    char           *strHZ;
    char           *strCode;
    INT8            iSelected;
    unsigned int    flag:1;
    struct _AUTOPHRASE *next;	//����һ������
} AUTOPHRASE;

typedef union {
    AUTOPHRASE     *autoPhrase;
    RECORD         *record;
    char            strPYPhrase[PHRASE_MAX_LENGTH * 2 + 1];
} CANDWORD;

typedef struct _TABLECANDWORD {
    unsigned int    flag:2;	//ָʾ�ú�ѡ��/�����Զ���Ĵʻ�����������/��
    CANDWORD        candWord;
} TABLECANDWORD;

typedef enum {
    CT_NORMAL = 0,
    CT_AUTOPHRASE,
    CT_PYPHRASE			//��ʱƴ��ת�������ĺ�ѡ��/��
} CANDTYPE;

void            LoadTableInfo (void);
Bool            LoadTableDict (void);
void            TableInit (void);
void            FreeTableIM (void);
void            SaveTableDict (void);
Bool            IsInputKey (int iKey);
Bool            IsIgnoreChar (char cChar);
Bool            IsEndKey (char cChar);
INPUT_RETURN_VALUE DoTableInput (int iKey);
INPUT_RETURN_VALUE TableGetCandWords (SEARCH_MODE mode);
void            TableAddCandWord (RECORD * wbRecord, SEARCH_MODE mode);
void            TableAddAutoCandWord (INT16 which, SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetLegendCandWords (SEARCH_MODE mode);
void            TableAddLegendCandWord (RECORD * record, SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetFHCandWords (SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetPinyinCandWords (SEARCH_MODE mode);
void            TableResetStatus (void);
char           *TableGetLegendCandWord (int iIndex);
char           *TableGetFHCandWord (int iIndex);
Bool            HasMatchingKey (void);
int             TableCompareCode (char *strUser, char *strDict);
int             TableFindFirstMatchCode (void);
void            TableAdjustOrderByIndex (int iIndex);
void            TableDelPhraseByIndex (int iIndex);
void            TableDelPhraseByHZ (char *strHZ);
void            TableDelPhrase (RECORD * record);
RECORD         *TableHasPhrase (char *strCode, char *strHZ);
RECORD         *TableFindPhrase (char *strHZ);
void            TableInsertPhrase (char *strCode, char *strHZ);
char	       *_TableGetCandWord (int iIndex, Bool _bLegend);		//Internal
char           *TableGetCandWord (int iIndex);
void		TableUpdateHitFrequency (RECORD * record);
void            TableCreateNewPhrase (void);
void            TableCreatePhraseCode (char *strHZ);
Bool            TablePhraseTips (void);
void            TableSetCandWordsFlag (int iCount, Bool flag);
void            TableResetFlags (void);

void            TableCreateAutoPhrase (INT8 iCount);

void            UpdateHZLastInput (char *);

#endif
