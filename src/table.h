#ifndef _TABLE_H
#define _TABLE_H

#include <X11/Xlib.h>
#include <limits.h>

#include "ime.h"

#define TABLE_CONFIG_FILENAME	"tables.conf"

#define	MAX_CODE_LENGTH		12
#define PHRASE_MAX_LENGTH	10
#define FH_MAX_LENGTH		10
#define TABLE_AUTO_SAVE_AFTER	48

typedef struct _RULE_RULE {
  unsigned char   iFlag;	// 1 --> ����   0 --> ����
  unsigned char   iWhich;	//�ڼ�����
  unsigned char   iIndex;	//�ڼ�������
} RULE_RULE;

typedef struct _RULE
{
  unsigned char  iWords;	//���ٸ���
  unsigned char   iFlag;	//1 --> ���ڵ���iWords  0 --> ����iWords
  RULE_RULE      *rule;
} RULE;

typedef struct _TABLE {
  char           strPath[PATH_MAX];
  char		 strSymbolFile[PATH_MAX];
  char	 	 strName[MAX_IM_NAME + 1];
  char 		*strInputCode;
  unsigned char  iCodeLength;
  char 		*strIgnoreChars;
  char 		 cMatchingKey;
  char 		 strSymbol[MAX_CODE_LENGTH + 1];
  char 		 cPinyin;
  unsigned char  bRule;

  RULE 		*rule;	//��ʹ���
  INT8 		 iIMIndex;	//��¼������Ӧ�����뷨��˳��
  unsigned int   iRecordCount;
  ADJUSTORDER    tableOrder;

  Bool 		  bUsePY;	//ʹ��ƴ��
  Bool            bTableAutoSendToClient;	//�Զ�����
  Bool            bUseMatchingKey;	//�Ƿ�ģ��ƥ��
  Bool		  bAutoPhrase;		//�Ƿ��Զ����
  INT8		  iAutoPhrase;		//�Զ���ʳ���
  Bool            bTableExactMatch;	//�Ƿ�ֻ��ʾ��ȷƥ��ĺ�ѡ��/��
  Bool            bPromptTableCode;	//������Ϻ��Ƿ���ʾ����--ʹ��ƴ��ʱ������ʾ
} TABLE;

typedef struct _RECORD {
  char          *strCode;
  char *strHZ;
  struct _RECORD *next;
  struct _RECORD *prev;
  unsigned int iHit;
  unsigned int iIndex;
  unsigned int flag:1;
} RECORD;

typedef struct _FH {
  char           strFH[FH_MAX_LENGTH * 2 + 1];
} FH;

typedef struct _AUTOPHRASE {
  char	       *strHZ;
  char	       *strCode;
  unsigned int	flag:1;
  struct _AUTOPHRASE  *next;			//����һ������
} AUTOPHRASE;

typedef union {
	AUTOPHRASE	*autoPhrase;
	RECORD		*record;
} CANDWORD;

typedef struct _TABLECANDWORD {
  unsigned int  flag:1;		//ָʾ�ú�ѡ��/�����Զ���Ĵʻ�����������/��
  CANDWORD candWord;
} TABLECANDWORD;

void          LoadTableInfo (void);
Bool	      LoadTableDict (void);
void	      TableInit(void);
void          FreeTableIM (void);
void          SaveTableDict (void);
Bool IsInputKey (int iKey);
Bool IsIgnoreChar(char cChar);
INPUT_RETURN_VALUE DoTableInput (int iKey);
INPUT_RETURN_VALUE TableGetCandWords (SEARCH_MODE mode);
void TableAddCandWord (RECORD * wbRecord, SEARCH_MODE mode);
void TableAddAutoCandWord(INT16 which, SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetLegendCandWords (SEARCH_MODE mode);
void TableAddLegendCandWord (RECORD * record, SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetFHCandWords (SEARCH_MODE mode);
INPUT_RETURN_VALUE TableGetPinyinCandWords (SEARCH_MODE mode);
void           TableResetStatus (void);
char          *TableGetLegendCandWord (int iIndex);
char          *TableGetFHCandWord (int iIndex);
Bool	       HasMatchingKey (void);
int            TableCompareCode (char *strUser, char *strDict);
int            TableFindFirstMatchCode (void);
RECORD 	      *TableFindCode (char *strHZ, Bool bMode);
void           TableAdjustOrderByIndex (int iIndex);
void           TableDelPhraseByIndex (int iIndex);
void           TableDelPhraseByHZ (char *strHZ);
void           TableDelPhrase(RECORD *record);
RECORD        *TableHasPhrase(char *strCode,char *strHZ);
RECORD        *TableFindPhrase(char *strHZ);
void           TableInsertPhrase (char *strCode, char *strHZ);
char          *TableGetCandWord (int iIndex);
void           TableCreateNewPhrase (void);
void           TableCreatePhraseCode (char *strHZ);
Bool	       TablePhraseTips (void);
void           TableSetCandWordsFlag (int iCount, Bool flag);
void           TableResetFlags (void);

void	       TableCreateAutoPhrase(INT8 iCount);

void           UpdateHZLastInput (char *);

#endif
