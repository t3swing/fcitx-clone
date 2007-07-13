#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "table.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif

#include <X11/Xlib.h>

#include "tools.h"
#include "InputWindow.h"
#include "py.h"
#include "pyParser.h"

TABLE          *table;
INT8            iTableIMIndex = 0;
INT8            iTableCount;

Bool            bTableDictLoaded = False;	//��Ҫ�õ�ʱ���ٶ������

RECORD         *currentRecord = NULL, *recordHead = NULL;
RECORD        **tableSingleHZ;	//��¼����еĵ��֣�������Զ�������ٶ�
unsigned int    iSingleHZCount;
TABLECANDWORD   tableCandWord[MAX_CAND_WORD];

RECORD_INDEX   *recordIndex = NULL;

AUTOPHRASE     *autoPhrase = NULL;
AUTOPHRASE     *insertPoint = NULL;

INT16           iAutoPhrase = 0;
INT16           iTotalAutoPhrase;

INT8            iTableOrderChanged = 0;
char            strTableLegendSource[PHRASE_MAX_LENGTH * 2 + 1] = "\0";

FH             *fh;
int             iFH = 0;
unsigned int    iTableIndex = 0;

Bool            bIsTableDelPhrase = False;
HOTKEYS         hkTableDelPhrase[HOT_KEY_COUNT] = { CTRL_7, 0 };
Bool            bIsTableAdjustOrder = False;
HOTKEYS         hkTableAdjustOrder[HOT_KEY_COUNT] = { CTRL_6, 0 };
Bool            bIsTableAddPhrase = False;
HOTKEYS         hkTableAddPhrase[HOT_KEY_COUNT] = { CTRL_8, 0 };

INT8            iTableChanged = 0;
INT8            iTableNewPhraseHZCount;
Bool            bCanntFindCode;	//��¼����ɵĴ��ܷ�������ʱ���--һ������¶��ǿ��Ե�
char           *strNewPhraseCode;

SINGLE_HZ       hzLastInput[MAX_HZ_SAVED + PHRASE_MAX_LENGTH];	//��¼�������ĺ���
INT16           iHZLastInputCount = 0;
Bool            bTablePhraseTips = False;

ADJUSTORDER     PYBaseOrder;
extern char     strPYAuto[];

extern Display *dpy;
extern Window   inputWindow;

extern char     strCodeInput[];
extern Bool     bIsDoInputOnly;
extern int      iCandPageCount;
extern int      iCurrentCandPage;
extern int      iCandWordCount;
extern int      iLegendCandWordCount;
extern int      iLegendCandPageCount;
extern int      iCurrentLegendCandPage;
extern int      iCodeInputCount;
extern int      iMaxCandWord;
extern char     strStringGet[];
extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;

extern INT8     iIMIndex;
extern Bool     bUseLegend;
extern Bool     bIsInLegend;
extern INT8     lastIsSingleHZ;
extern Bool     bDisablePagingInLegend;
extern Bool     bShowCursor;
extern Bool     bAutoHideInputWindow;
extern ADJUSTORDER baseOrder;
extern Bool     bSP;

//----------------------------------------
extern PYFA    *PYFAList;
extern PYCandWord PYCandWords[];
extern Bool     bSingleHZMode;
extern char     strFindString[];
extern ParsePYStruct findMap;

//----------------------------------------

/*
 * ��ȡ������뷨�� ���� �� �ļ�·��
 */
void LoadTableInfo (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];
    int             i;
    char           *pstr;

    FreeTableIM ();
    if (table)
	free (table);
    iTableCount = 0;

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, TABLE_CONFIG_FILENAME);

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, TABLE_CONFIG_FILENAME);
    }

    fp = fopen (strPath, "rt");
    if (!fp)
	return;

    //�����������ж��ٸ����
    while (1) {
	if (!fgets (strPath, PATH_MAX, fp))
	    break;

	i = strlen (strPath) - 1;
	while (strPath[i] == ' ' || strPath[i] == '\n')
	    strPath[i--] = '\0';

	pstr = strPath;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (strstr (pstr, "[���]"))
	    iTableCount++;
    }

    table = (TABLE *) malloc (sizeof (TABLE) * iTableCount);
    for (iTableIMIndex = 0; iTableIMIndex < iTableCount; iTableIMIndex++) {
	table[iTableIMIndex].strInputCode = NULL;
	table[iTableIMIndex].strName[0] = '\0';
	table[iTableIMIndex].strPath[0] = '\0';
	table[iTableIMIndex].strSymbolFile[0] = '\0';
	table[iTableIMIndex].tableOrder = AD_NO;
	table[iTableIMIndex].bUsePY = True;
	table[iTableIMIndex].cPinyin = '\0';
	table[iTableIMIndex].bTableAutoSendToClient = True;
	table[iTableIMIndex].bUseMatchingKey = False;
	table[iTableIMIndex].cMatchingKey = '\0';
	table[iTableIMIndex].bTableExactMatch = False;
	table[iTableIMIndex].bAutoPhrase = True;
	table[iTableIMIndex].bAutoPhrasePhrase = True;
	table[iTableIMIndex].iSaveAutoPhraseAfter = 1;
	table[iTableIMIndex].iAutoPhrase = 4;
	table[iTableIMIndex].bPromptTableCode = True;
	table[iTableIMIndex].strSymbol[0] = '\0';
    }

    iTableIMIndex = -1;
    //��ȡ�������Ե�Ĭ��ֵ
    if (iTableCount) {
	rewind (fp);

	while (1) {
	    if (!fgets (strPath, PATH_MAX, fp))
		break;

	    i = strlen (strPath) - 1;
	    while (strPath[i] == ' ' || strPath[i] == '\n')
		strPath[i--] = '\0';

	    pstr = strPath;
	    if (*pstr == ' ')
		pstr++;
	    if (pstr[0] == '#')
		continue;

	    if (!strcmp (pstr, "[���]")) {
		if (iTableIMIndex != -1) {
		    if (table[iTableIMIndex].strName[0] == '\0' || table[iTableIMIndex].strPath[0] == '\0') {
			iTableCount = 0;
			free (table);
			fprintf (stderr, "��%d����������ļ��д��޷������������\n", iTableIMIndex);
			return;
		    }
		}
		iTableIMIndex++;
	    }
	    else if (!strncmp (pstr, "����=", 5)) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strName, pstr);
	    }
	    else if (!strncmp (pstr, "���=", 5)) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strPath, pstr);
	    }
	    else if (!strncmp (pstr, "��Ƶ=", 5)) {
		pstr += 5;
		table[iTableIMIndex].tableOrder = (ADJUSTORDER) atoi (pstr);
	    }
	    else if (!strncmp (pstr, "ƴ��=", 5)) {
		pstr += 5;
		table[iTableIMIndex].bUsePY = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "ƴ����=", 7)) {
		pstr += 7;
		while (*pstr == ' ')
		    pstr++;
		table[iTableIMIndex].cPinyin = *pstr;
	    }
	    else if (!strncmp (pstr, "�Զ�����=", 9)) {
		pstr += 9;
		table[iTableIMIndex].bTableAutoSendToClient = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "ģ��=", 5)) {
		pstr += 5;
		table[iTableIMIndex].bUseMatchingKey = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "ģ����=", 7)) {
		pstr += 7;
		while (*pstr == ' ')
		    pstr++;
		table[iTableIMIndex].cMatchingKey = *pstr;
	    }
	    else if (!strncmp (pstr, "��ȷƥ��=", 9)) {
		pstr += 9;
		table[iTableIMIndex].bTableExactMatch = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "�Զ�����=", 9)) {
		pstr += 9;
		table[iTableIMIndex].bAutoPhrase = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "�Զ����鳤��=", 13)) {
		pstr += 13;
		table[iTableIMIndex].iAutoPhrase = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "��������Զ����=", 17)) {
		pstr += 17;
		table[iTableIMIndex].bAutoPhrasePhrase = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "�����Զ�����=", 13)) {
		pstr += 13;
		table[iTableIMIndex].iSaveAutoPhraseAfter = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "��ʾ����=", 9)) {
		pstr += 9;
		table[iTableIMIndex].bPromptTableCode = atoi (pstr);
	    }
	    else if (!strncmp (pstr, "����=", 5)) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strSymbol, pstr);
	    }
	    else if (!strncmp (pstr, "�����ļ�=", 9)) {
		pstr += 9;
		strcpy (table[iTableIMIndex].strSymbolFile, pstr);
	    }
	}
    }

    table[iTableIMIndex].rule = NULL;

    fclose (fp);
}

Bool LoadTableDict (void)
{
    char            strCode[MAX_CODE_LENGTH + 1];
    char            strHZ[PHRASE_MAX_LENGTH * 2 + 1];
    FILE           *fpDict;
    RECORD         *recTemp;
    char            strPath[PATH_MAX];
    unsigned int    i = 0;
    unsigned int    iTemp;
    char            cChar = 0;

    //���ȣ����������Ǹõ����ĸ����
    for (i = 0; i < iTableCount; i++) {
	if (table[i].iIMIndex == iIMIndex)
	    iTableIMIndex = i;
    }

    //�������
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, table[iTableIMIndex].strPath);

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, table[iTableIMIndex].strPath);
    }

    fpDict = fopen (strPath, "rb");
    if (!fpDict) {
	fprintf (stderr, "�޷�������ļ�: %s\n", strPath);
	return False;
    }

    //�ȶ�ȡ������Ϣ
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    table[iTableIMIndex].strInputCode = (char *) malloc (sizeof (char) * (iTemp + 1));
    fread (table[iTableIMIndex].strInputCode, sizeof (char), iTemp + 1, fpDict);

    /*
     * ��������
     */
    recordIndex = (RECORD_INDEX *) malloc (strlen (table[iTableIMIndex].strInputCode) * sizeof (RECORD_INDEX));
    for (iTemp = 0; iTemp < strlen (table[iTableIMIndex].strInputCode); iTemp++)
	recordIndex[iTemp].cCode = table[iTableIMIndex].strInputCode[iTemp];
    /* ********************************************************************** */

    fread (&(table[iTableIMIndex].iCodeLength), sizeof (unsigned char), 1, fpDict);
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    table[iTableIMIndex].strIgnoreChars = (char *) malloc (sizeof (char) * (iTemp + 1));
    fread (table[iTableIMIndex].strIgnoreChars, sizeof (char), iTemp + 1, fpDict);

    fread (&(table[iTableIMIndex].bRule), sizeof (unsigned char), 1, fpDict);

    if (table[iTableIMIndex].bRule) {	//��ʾ����ʹ���
	table[iTableIMIndex].rule = (RULE *) malloc (sizeof (RULE) * (table[iTableIMIndex].iCodeLength - 1));
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	    fread (&(table[iTableIMIndex].rule[i].iFlag), sizeof (unsigned char), 1, fpDict);
	    fread (&(table[iTableIMIndex].rule[i].iWords), sizeof (unsigned char), 1, fpDict);
	    table[iTableIMIndex].rule[i].rule = (RULE_RULE *) malloc (sizeof (RULE_RULE) * table[iTableIMIndex].iCodeLength);
	    for (iTemp = 0; iTemp < table[iTableIMIndex].iCodeLength; iTemp++) {
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iFlag), sizeof (unsigned char), 1, fpDict);
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iWhich), sizeof (unsigned char), 1, fpDict);
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iIndex), sizeof (unsigned char), 1, fpDict);
	    }
	}
    }

    recordHead = (RECORD *) malloc (sizeof (RECORD));
    currentRecord = recordHead;

    fread (&(table[iTableIMIndex].iRecordCount), sizeof (unsigned int), 1, fpDict);
    iSingleHZCount = 0;

    for (i = 0; i < table[iTableIMIndex].iRecordCount; i++) {
	fread (strCode, sizeof (char), table[iTableIMIndex].iCodeLength + 1, fpDict);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
	fread (strHZ, sizeof (char), iTemp, fpDict);

	//���iTempΪ3��˵�����Ǹ�����
	if (iTemp == 3)
	    iSingleHZCount++;

	recTemp = (RECORD *) malloc (sizeof (RECORD));
	recTemp->strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
	strcpy (recTemp->strCode, strCode);
	recTemp->strHZ = (char *) malloc (sizeof (char) * iTemp);
	strcpy (recTemp->strHZ, strHZ);
	recTemp->flag = 0;

	fread (&(recTemp->iHit), sizeof (unsigned int), 1, fpDict);
	fread (&(recTemp->iIndex), sizeof (unsigned int), 1, fpDict);
	if (recTemp->iIndex > iTableIndex)
	    iTableIndex = recTemp->iIndex;

	/* �������� */
	if (cChar != recTemp->strCode[0]) {
	    cChar = recTemp->strCode[0];
	    iTemp = 0;
	    while (cChar != recordIndex[iTemp].cCode)
		iTemp++;
	    recordIndex[iTemp].record = recTemp;
	}
	/* **************************************************************** */

	currentRecord->next = recTemp;
	recTemp->prev = currentRecord;
	currentRecord = recTemp;
    }

    currentRecord->next = recordHead;
    recordHead->prev = currentRecord;

    fclose (fpDict);

    //��ȡ��Ӧ��������ű�
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, table[iTableIMIndex].strSymbolFile);

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, table[iTableIMIndex].strSymbolFile);
	fpDict = fopen (strPath, "rt");
    }

    fpDict = fopen (strPath, "rt");
    if (fpDict) {
	iFH = CalculateRecordNumber (fpDict);
	fh = (FH *) malloc (sizeof (FH) * iFH);

	for (i = 0; i < iFH; i++) {
	    if (EOF == fscanf (fpDict, "%s\n", fh[i].strFH))
		break;
	}
	iFH = i;

	fclose (fpDict);
    }

    strNewPhraseCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
    strNewPhraseCode[table[iTableIMIndex].iCodeLength] = '\0';
    bTableDictLoaded = True;

    /*
     * ������Ҫ���ٸ���¼�������Զ����ɵĴ���
     */
    iTotalAutoPhrase = 0;
    for (i = 2; i < table[iTableIMIndex].iAutoPhrase; i++)
	iTotalAutoPhrase += MAX_HZ_SAVED - i + 1;

    autoPhrase = (AUTOPHRASE *) malloc (sizeof (AUTOPHRASE) * iTotalAutoPhrase);

    for (i = 0; i < iTotalAutoPhrase; i++) {
	autoPhrase[i].strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
	autoPhrase[i].strHZ = (char *) malloc (sizeof (char) * (PHRASE_MAX_LENGTH * 2 + 1));
	autoPhrase[i].iSelected = 0;
	if (i == iTotalAutoPhrase - 1)
	    autoPhrase[i].next = &autoPhrase[0];
	else
	    autoPhrase[i].next = &autoPhrase[i + 1];
    }
    insertPoint = &autoPhrase[0];

    /*
     * Ϊ��������һ����
     */
    tableSingleHZ = (RECORD **) malloc (sizeof (RECORD *) * iSingleHZCount);
    recTemp = recordHead->next;
    i = 0;
    while (recTemp != recordHead) {
	if (strlen (recTemp->strHZ) == 2)
	    tableSingleHZ[i++] = recTemp;
	recTemp = recTemp->next;
    }

    /*
     * Ȼ���ʼ��������뷨
     TableInit();
     */
    return True;
}

void TableInit (void)
{
    bSP = False;
    PYBaseOrder = baseOrder;
    baseOrder = AD_FAST;
    strPYAuto[0] = '\0';
}

/*
 * �ͷŵ�ǰ�����ռ�õ��ڴ�
 * Ŀ�����л����ʱʹռ�õ��ڴ����
 */
void FreeTableIM (void)
{
    RECORD         *recTemp, *recNext;
    INT16           i;

    if (!recordHead)
	return;

    if (iTableChanged || iTableOrderChanged)
	SaveTableDict ();

    //�ͷ����
    recTemp = recordHead->next;
    while (recTemp != recordHead) {
	recNext = recTemp->next;

	free (recTemp->strCode);
	free (recTemp->strHZ);
	free (recTemp);

	recTemp = recNext;
    }

    free (recordHead);
    recordHead = NULL;

    if (iFH) {
	free (fh);
	iFH = 0;
    }
    free (table[iTableIMIndex].strInputCode);
    free (table[iTableIMIndex].strIgnoreChars);
    table[iTableIMIndex].iRecordCount = 0;
    bTableDictLoaded = False;

    free (strNewPhraseCode);

    //�ͷ���ʹ���Ŀռ�
    if (table[iTableIMIndex].rule) {
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++)
	    free (table[iTableIMIndex].rule[i].rule);
	free (table[iTableIMIndex].rule);

	table[iTableIMIndex].rule = NULL;
    }

    //�ͷ������Ŀռ�
    if (recordIndex) {
	free (recordIndex);
	recordIndex = NULL;
    }

    //�ͷ��Զ�����Ŀռ�
    for (i = 0; i < iTotalAutoPhrase; i++) {
	free (autoPhrase[i].strCode);
	free (autoPhrase[i].strHZ);
    }
    free (autoPhrase);

    baseOrder = PYBaseOrder;

    //�ͷŵ��ֱ�
    free (tableSingleHZ);
}

void TableResetStatus (void)
{
    bIsTableAddPhrase = False;
    bIsTableDelPhrase = False;
    bIsTableAdjustOrder = False;
    bIsDoInputOnly = False;
}

void SaveTableDict (void)
{
    RECORD         *recTemp;
    char            strPath[PATH_MAX];
    char            strPathTemp[PATH_MAX];
    FILE           *fpDict;
    unsigned int    iTemp;
    unsigned int    i;

    //�Ƚ��������һ����ʱ�ļ��У���꽱���ɹ����ٽ�����ʱ�ļ�����������������������Է�ֹ����ļ����ƻ�
    strcpy (strPathTemp, (char *) getenv ("HOME"));
    strcat (strPathTemp, "/.fcitx/");
    if (access (strPathTemp, 0))
	mkdir (strPathTemp, S_IRWXU);
    strcat (strPathTemp, TEMP_FILE);
    fpDict = fopen (strPathTemp, "wb");
    if (!fpDict) {
	fprintf (stderr, "�޷���������ļ�: %s\n", strPathTemp);
	return;
    }

    iTemp = strlen (table[iTableIMIndex].strInputCode);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (table[iTableIMIndex].strInputCode, sizeof (char), iTemp + 1, fpDict);
    fwrite (&(table[iTableIMIndex].iCodeLength), sizeof (INT8), 1, fpDict);
    iTemp = strlen (table[iTableIMIndex].strIgnoreChars);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (table[iTableIMIndex].strIgnoreChars, sizeof (char), iTemp + 1, fpDict);

    fwrite (&(table[iTableIMIndex].bRule), sizeof (unsigned char), 1, fpDict);
    if (table[iTableIMIndex].bRule) {	//��ʾ����ʹ���
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	    fwrite (&(table[iTableIMIndex].rule[i].iFlag), sizeof (unsigned char), 1, fpDict);
	    fwrite (&(table[iTableIMIndex].rule[i].iWords), sizeof (unsigned char), 1, fpDict);
	    for (iTemp = 0; iTemp < table[iTableIMIndex].iCodeLength; iTemp++) {
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iFlag), sizeof (unsigned char), 1, fpDict);
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iWhich), sizeof (unsigned char), 1, fpDict);
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iIndex), sizeof (unsigned char), 1, fpDict);
	    }
	}
    }

    fwrite (&(table[iTableIMIndex].iRecordCount), sizeof (unsigned int), 1, fpDict);
    recTemp = recordHead->next;
    while (recTemp != recordHead) {
	fwrite (recTemp->strCode, sizeof (char), table[iTableIMIndex].iCodeLength + 1, fpDict);
	iTemp = strlen (recTemp->strHZ) + 1;
	fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
	fwrite (recTemp->strHZ, sizeof (char), iTemp, fpDict);
	fwrite (&(recTemp->iHit), sizeof (unsigned int), 1, fpDict);
	fwrite (&(recTemp->iIndex), sizeof (unsigned int), 1, fpDict);
	recTemp = recTemp->next;
    }

    fclose (fpDict);

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, table[iTableIMIndex].strPath);
    if (access (strPath, 0))
	unlink (strPath);
    rename (strPathTemp, strPath);

    iTableOrderChanged = 0;
    iTableChanged = 0;
}

Bool IsInputKey (int iKey)
{
    char           *p;

    p = table[iTableIMIndex].strInputCode;
    if (!p)
	return False;

    while (*p) {
	if (iKey == *p)
	    return True;
	p++;
    }

    return False;
}

Bool IsIgnoreChar (char cChar)
{
    char           *p;

    p = table[iTableIMIndex].strIgnoreChars;
    while (*p) {
	if (cChar == *p)
	    return True;
	p++;
    }

    return False;
}

INPUT_RETURN_VALUE DoTableInput (int iKey)
{
    INPUT_RETURN_VALUE retVal;

    if (!bTableDictLoaded)
	LoadTableDict ();

    if (bTablePhraseTips) {
	if (iKey == CTRL_DELETE) {
	    bTablePhraseTips = False;
	    TableDelPhraseByHZ (messageUp[1].strMsg);
	    return IRV_DONOT_PROCESS_CLEAN;
	}
	else if (iKey != LCTRL && iKey != RCTRL && iKey != LSHIFT && iKey != RSHIFT) {
	    uMessageUp = uMessageDown = 0;
	    bTablePhraseTips = False;
	    if (bAutoHideInputWindow)
		XUnmapWindow (dpy, inputWindow);
	    else
		DisplayInputWindow ();
	}
    }

    retVal = IRV_DO_NOTHING;
    if (IsInputKey (iKey) || iKey == table[iTableIMIndex].cMatchingKey || iKey == table[iTableIMIndex].cPinyin) {
	bIsInLegend = False;

	if (!bIsTableAddPhrase && !bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY) {
		if (iCodeInputCount != (MAX_PY_LENGTH + 1)) {
		    strCodeInput[iCodeInputCount++] = (char) iKey;
		    strCodeInput[iCodeInputCount] = '\0';
		    retVal = TableGetCandWords (SM_FIRST);
		}
		else
		    retVal = IRV_DO_NOTHING;
	    }
	    else {
		if (iCodeInputCount < table[iTableIMIndex].iCodeLength) {
		    strCodeInput[iCodeInputCount++] = (char) iKey;
		    strCodeInput[iCodeInputCount] = '\0';

		    if (iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY) {
			iCandWordCount = 0;
			retVal = IRV_DISPLAY_LAST;
		    }
		    else {
			retVal = TableGetCandWords (SM_FIRST);

			if (table[iTableIMIndex].bTableAutoSendToClient && (iCodeInputCount == table[iTableIMIndex].iCodeLength)) {
			    if (iCandWordCount == 1 && tableCandWord[0].flag) {	//���ֻ��һ����ѡ�ʣ����͵��ͻ�������
				strcpy (strStringGet, TableGetCandWord (0));
				iCandWordCount = 0;
				if (bIsInLegend)
				    retVal = IRV_GET_LEGEND;
				else
				    retVal = IRV_GET_CANDWORDS;
			    }
			}
		    }
		}
		else {
		    if (table[iTableIMIndex].bTableAutoSendToClient) {
			if (iCandWordCount) {
			    if (tableCandWord[0].flag) {
				strcpy (strStringGet, TableGetCandWord (0));
				retVal = IRV_GET_CANDWORDS_NEXT;
			    }
			    else
				retVal = IRV_DISPLAY_CANDWORDS;
			}
			else
			    retVal = IRV_DISPLAY_CANDWORDS;

			iCodeInputCount = 1;
			strCodeInput[0] = iKey;
			strCodeInput[1] = '\0';
			bIsInLegend = False;

			TableGetCandWords (SM_FIRST);
		    }
		    else
			retVal = IRV_DO_NOTHING;
		}
	    }
	}
    }
    else {
	if (bIsTableAddPhrase) {
	    switch (iKey) {
	    case LEFT:
		if (iTableNewPhraseHZCount < iHZLastInputCount && iTableNewPhraseHZCount < PHRASE_MAX_LENGTH) {
		    iTableNewPhraseHZCount++;
		    TableCreateNewPhrase ();
		}
		break;
	    case RIGHT:
		if (iTableNewPhraseHZCount > 2) {
		    iTableNewPhraseHZCount--;
		    TableCreateNewPhrase ();
		}
		break;
	    case ENTER:
		if (!bCanntFindCode)
		    TableInsertPhrase (messageDown[1].strMsg, messageDown[0].strMsg);
	    case ESC:
		bIsTableAddPhrase = False;
		bIsDoInputOnly = False;
		return IRV_CLEAN;
	    default:
		return IRV_DO_NOTHING;
	    }

	    return IRV_DISPLAY_MESSAGE;
	}
	if (IsHotKey (iKey, hkTableAddPhrase)) {
	    if (!bIsTableAddPhrase) {
		if (iHZLastInputCount < 2 || !table[iTableIMIndex].bRule)	//��������Ϊ��������
		    return IRV_DO_NOTHING;

		iTableNewPhraseHZCount = 2;
		bIsTableAddPhrase = True;
		bIsDoInputOnly = True;
		bShowCursor = False;

		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "��/�Ҽ�����/���٣�ENTERȷ����ESCȡ��");
		messageUp[0].type = MSG_TIPS;

		uMessageDown = 2;
		messageDown[0].type = MSG_FIRSTCAND;
		messageDown[1].type = MSG_CODE;

		TableCreateNewPhrase ();
		DisplayInputWindow ();

		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else
		retVal = IRV_TO_PROCESS;

	    return retVal;
	}

	if (!iCodeInputCount && !bIsInLegend)
	    return IRV_TO_PROCESS;

	if (iKey == ESC) {
	    if (bIsTableDelPhrase || bIsTableAdjustOrder) {
		TableResetStatus ();
		retVal = IRV_DISPLAY_CANDWORDS;
	    }
	    else
		return IRV_CLEAN;
	}
	else if (iKey >= '0' && iKey <= '9') {
	    if (!iCandWordCount)
		return IRV_TO_PROCESS;

	    iKey -= '0';
	    if (iKey == 0)
		iKey = 10;

	    if (!bIsInLegend) {
		if (iKey > iCandWordCount)
		    return IRV_DO_NOTHING;
		else {
		    if (bIsTableDelPhrase) {
			TableDelPhraseByIndex (iKey);
			retVal = TableGetCandWords (SM_FIRST);
		    }
		    else if (bIsTableAdjustOrder) {
			TableAdjustOrderByIndex (iKey);
			retVal = TableGetCandWords (SM_FIRST);
		    }
		    else {
			//�����ƴ����������ٵ�����Ƶ��ʽ
		        if (strcmp (strCodeInput, table[iTableIMIndex].strSymbol) && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
			    PYGetCandWord (iKey - 1);
			
			strcpy (strStringGet, TableGetCandWord (iKey - 1));
			if (bIsInLegend)
			    retVal = IRV_GET_LEGEND;
			else
			    retVal = IRV_GET_CANDWORDS;
		    }
		}
	    }
	    else {
		strcpy (strStringGet, TableGetLegendCandWord (iKey - 1));
		retVal = IRV_GET_LEGEND;
	    }
	}
	else if (!bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (IsHotKey (iKey, hkTableAdjustOrder)) {
		if ((iCurrentCandPage == 0 && iCandWordCount < 2) || bIsInLegend)
		    return IRV_DO_NOTHING;

		bIsTableAdjustOrder = True;
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "ѡ����Ҫ��ǰ�Ĵ�����ţ�ESCȡ��");
		messageUp[0].type = MSG_TIPS;
		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else if (IsHotKey (iKey, hkTableDelPhrase)) {
		if (!iCandWordCount || bIsInLegend)
		    return IRV_DO_NOTHING;

		bIsTableDelPhrase = True;
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "ѡ����Ҫɾ���Ĵ�����ţ�ESCȡ��");
		messageUp[0].type = MSG_TIPS;
		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else if (iKey == (XK_BackSpace & 0x00FF)) {
		if (!iCodeInputCount) {
		    bIsInLegend = False;
		    return IRV_DONOT_PROCESS_CLEAN;
		}

		iCodeInputCount--;
		strCodeInput[iCodeInputCount] = '\0';

		if (iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin) {
		    iCandWordCount = 0;
		    retVal = IRV_DISPLAY_LAST;
		}
		else if (iCodeInputCount)
		    retVal = TableGetCandWords (SM_FIRST);
		else
		    retVal = IRV_CLEAN;
	    }
	    else if (iKey == ' ') {
		if (!bIsInLegend) {
		    if (!(table[iTableIMIndex].bUsePY && iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin)) {
			if (strcmp (strCodeInput, table[iTableIMIndex].strSymbol) && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
			    PYGetCandWord (0);

			if (!iCandWordCount) {
			    iCodeInputCount = 0;
			    return IRV_CLEAN;
			}
			strcpy (strStringGet, TableGetCandWord (0));
		    }
		    else
			uMessageDown = 0;

		    if (bIsInLegend)
			retVal = IRV_GET_LEGEND;
		    else
			retVal = IRV_GET_CANDWORDS;
		}
		else {
		    strcpy (strStringGet, TableGetLegendCandWord (0));
		    retVal = IRV_GET_LEGEND;
		}
	    }
	    else
		return IRV_TO_PROCESS;
	}
    }

    if (!bIsInLegend) {
	if (!bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (iCodeInputCount) {
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, strCodeInput);
		messageUp[0].type = MSG_INPUT;
	    }
	    else
		uMessageUp = 0;
	}
	else
	    bIsDoInputOnly = True;
    }

    if (bIsTableDelPhrase || bIsTableAdjustOrder || bIsInLegend)
	bShowCursor = False;
    else
	bShowCursor = True;
    
    return retVal;
}

char           *TableGetCandWord (int iIndex)
{
    char           *pCandWord;

    if (!strcmp (strCodeInput, table[iTableIMIndex].strSymbol))
	return TableGetFHCandWord (iIndex);

    bIsInLegend = False;

    if (!iCandWordCount)
	return NULL;

    if (iIndex > (iCandWordCount - 1))
	iIndex = iCandWordCount - 1;

    if (tableCandWord[iIndex].flag) {
	tableCandWord[iIndex].candWord.record->iHit++;
	tableCandWord[iIndex].candWord.record->iIndex = ++iTableIndex;
    }
    if (table[iTableIMIndex].tableOrder != AD_NO) {
	iTableOrderChanged++;
	if (iTableOrderChanged == TABLE_AUTO_SAVE_AFTER)
	    SaveTableDict ();
    }
    if (tableCandWord[iIndex].flag)
	pCandWord = tableCandWord[iIndex].candWord.record->strHZ;
    else {
	if (table[iTableIMIndex].iSaveAutoPhraseAfter) {
	    if (table[iTableIMIndex].iSaveAutoPhraseAfter >= tableCandWord[iIndex].candWord.autoPhrase->iSelected)
		tableCandWord[iIndex].candWord.autoPhrase->iSelected++;
	    if (table[iTableIMIndex].iSaveAutoPhraseAfter == tableCandWord[iIndex].candWord.autoPhrase->iSelected) {
		//�����Զ�����
		TableInsertPhrase (tableCandWord[iIndex].candWord.autoPhrase->strCode, tableCandWord[iIndex].candWord.autoPhrase->strHZ);
		tableCandWord[iIndex].candWord.autoPhrase->iSelected = 0;
	    }
	}
	pCandWord = tableCandWord[iIndex].candWord.autoPhrase->strHZ;
    }

    if (bUseLegend) {
	strcpy (strTableLegendSource, pCandWord);
	TableGetLegendCandWords (SM_FIRST);
    }
    else {
	if (table[iTableIMIndex].bPromptTableCode) {
	    RECORD         *temp;

	    uMessageUp = 1;
	    strcpy (messageUp[0].strMsg, strCodeInput);
	    messageUp[0].type = MSG_INPUT;

	    strcpy (messageDown[0].strMsg, pCandWord);
	    messageDown[0].type = MSG_TIPS;
	    temp = TableFindCode (pCandWord, False);
	    if (temp) {
		strcpy (messageDown[1].strMsg, temp->strCode);
		messageDown[1].type = MSG_CODE;
		uMessageDown = 2;
	    }
	    else
		uMessageDown = 1;
	}
	else {
	    uMessageDown = 0;
	    uMessageUp = 0;
	    iCodeInputCount = 0;
	}
    }

    if (strlen (pCandWord) == 2)
	lastIsSingleHZ = 1;
    else
	lastIsSingleHZ = 0;

    if (strlen (pCandWord) == 2 || (strlen (pCandWord) > 2 && table[iTableIMIndex].bAutoPhrasePhrase))
	UpdateHZLastInput (pCandWord);

    return pCandWord;
}

INPUT_RETURN_VALUE TableGetPinyinCandWords (SEARCH_MODE mode)
{
    int             i;
    RECORD         *pRec;

    if (mode == SM_FIRST) {
	bSingleHZMode = True;
	strcpy (strFindString, strCodeInput + 1);

	DoPYInput (-1);

	strCodeInput[0] = table[iTableIMIndex].cPinyin;
	strCodeInput[1] = '\0';

	strcat (strCodeInput, strFindString);
	iCodeInputCount = strlen (strCodeInput);
    }
    else
	PYGetCandWords (mode);

    //���潫ƴ���ĺ�ѡ�ֱ�ת��Ϊ������뷨����ʽ
    for (i = 0; i < iCandWordCount; i++) {
	pRec = TableFindCode (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].strHZ, False);
	tableCandWord[i].flag = True;
	if (pRec)
	    tableCandWord[i].candWord.record = pRec;
	else
	    tableCandWord[i].candWord.record = recordHead->next;
    }

    return IRV_DISPLAY_CANDWORDS;
}

INPUT_RETURN_VALUE TableGetCandWords (SEARCH_MODE mode)
{
    int             i;
    char            strTemp[3], *pstr;
    unsigned int    iTableTotalCandCount = 0;

    if (bIsInLegend)
	return TableGetLegendCandWords (mode);
    if (!strcmp (strCodeInput, table[iTableIMIndex].strSymbol))
	return TableGetFHCandWords (mode);

    if (strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
	TableGetPinyinCandWords (mode);
    else {
	if (mode == SM_FIRST) {
	    iCandWordCount = 0;
	    iCandPageCount = 0;
	    iCurrentCandPage = 0;
	    iTableTotalCandCount = 0;

	    TableResetFlags ();

	    if (TableFindFirstMatchCode () == -1 && !iAutoPhrase) {
		uMessageDown = 0;
		return IRV_DISPLAY_CANDWORDS;	//Not Found
	    }
	}
	else {
	    if (!iCandPageCount)
		return IRV_TO_PROCESS;

	    if (mode == SM_NEXT) {
		if (iCurrentCandPage == iCandPageCount)
		    return IRV_DO_NOTHING;

		iCurrentCandPage++;
	    }
	    else {
		if (!iCurrentCandPage)
		    return IRV_DO_NOTHING;

		TableSetCandWordsFlag (iCandWordCount, False);
		iCurrentCandPage--;
	    }

	    TableFindFirstMatchCode ();
	}

	iCandWordCount = 0;
	if (mode == SM_PREV && table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase && iCodeInputCount == table[iTableIMIndex].iCodeLength) {
	    for (i = 0; i < iAutoPhrase; i++) {
		if (!TableCompareCode (strCodeInput, autoPhrase[i].strCode) && autoPhrase[i].flag) {
		    if (!TableCandHasPhrase (autoPhrase[i].strHZ))
			TableAddAutoCandWord (i, mode);
		}
	    }
	}

	if (iCandWordCount < iMaxCandWord) {
	    while (currentRecord != recordHead) {
		if ((mode == SM_PREV) ^ (!currentRecord->flag)) {
		    if (!TableCompareCode (strCodeInput, currentRecord->strCode) && CheckHZCharset (currentRecord->strHZ)) {
			if (mode == SM_FIRST)
			    iTableTotalCandCount++;
			TableAddCandWord (currentRecord, mode);
		    }
		}

		currentRecord = currentRecord->next;
	    }
	}

	if (table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase && mode != SM_PREV && iCodeInputCount == table[iTableIMIndex].iCodeLength) {
	    for (i = 0; i < iAutoPhrase; i++) {
		if (!TableCompareCode (strCodeInput, autoPhrase[i].strCode) && !autoPhrase[i].flag) {
		    if (!TableCandHasPhrase (autoPhrase[i].strHZ)) {
			if (mode == SM_FIRST)
			    iTableTotalCandCount++;
			TableAddAutoCandWord (i, mode);
		    }
		}
	    }
	}

	TableSetCandWordsFlag (iCandWordCount, True);

	if (mode == SM_FIRST)
	    iCandPageCount = iTableTotalCandCount / iMaxCandWord - ((iTableTotalCandCount % iMaxCandWord) ? 0 : 1);
    }

    strTemp[1] = '.';
    strTemp[2] = '\0';
    uMessageDown = 0;

    for (i = 0; i < iCandWordCount; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;

	strcpy (messageDown[uMessageDown].strMsg, (tableCandWord[i].flag) ? tableCandWord[i].candWord.record->strHZ : tableCandWord[i].candWord.autoPhrase->strHZ);
	if (tableCandWord[i].flag)
	    messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
	else
	    messageDown[uMessageDown++].type = MSG_TIPS;
	if (HasMatchingKey () || strCodeInput[0] == table[iTableIMIndex].cPinyin)
	    pstr = (tableCandWord[i].flag) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode;
	else
	    pstr = ((tableCandWord[i].flag) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode) + iCodeInputCount;
	strcpy (messageDown[uMessageDown].strMsg, pstr);
	if (i != (iCandWordCount - 1)) {
#ifdef _USE_XFT
	    strcat (messageDown[uMessageDown].strMsg, "  ");
#else
	    strcat (messageDown[uMessageDown].strMsg, " ");
#endif
	}
	messageDown[uMessageDown++].type = MSG_CODE;
    }

    return IRV_DISPLAY_CANDWORDS;
}

/*
 * �жϺ�ѡ���б����Ƿ���ָ���Ĵ���
 * ���ڽ��Զ�����е��ظ�����ȥ��
 */
Bool TableCandHasPhrase (char *strHZ)
{
    int             i;

    for (i = 0; i < iCandWordCount; i++) {
	if (!tableCandWord[i].flag)
	    return False;
	if (!strcmp (strHZ, tableCandWord[i].candWord.record->strHZ))
	    return True;
    }

    return False;
}

void TableAddAutoCandWord (INT16 which, SEARCH_MODE mode)
{
    int             i, j;

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    i = iCandWordCount - 1;
	    for (j = 0; j < iCandWordCount - 1; j++)
		tableCandWord[j].candWord.autoPhrase = tableCandWord[j + 1].candWord.autoPhrase;
	}
	else
	    i = iCandWordCount++;
	tableCandWord[i].flag = False;
	tableCandWord[i].candWord.autoPhrase = &autoPhrase[which];
    }
    else {
	if (iCandWordCount == iMaxCandWord)
	    return;

	tableCandWord[iCandWordCount].flag = False;
	tableCandWord[iCandWordCount++].candWord.autoPhrase = &autoPhrase[which];
    }
}

void TableAddCandWord (RECORD * record, SEARCH_MODE mode)
{
    int             i = 0, j;

    switch (table[iTableIMIndex].tableOrder) {
    case AD_NO:
	if (mode == SM_PREV) {
	    if (iCandWordCount == iMaxCandWord)
		i = iCandWordCount - 1;
	    else {
		for (i = 0; i < iCandWordCount; i++) {
		    if (!tableCandWord[i].flag)
			break;
		}
	    }
	}
	else {
	    if (iCandWordCount == iMaxCandWord)
		return;
	    tableCandWord[iCandWordCount].flag = True;
	    tableCandWord[iCandWordCount++].candWord.record = record;
	    return;
	}
	break;
    case AD_FREQ:
	if (mode == SM_PREV) {
	    for (i = iCandWordCount - 1; i >= 0; i--) {
		if (tableCandWord[i].flag) {
		    if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) < 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iHit >= record->iHit))
			break;
		}
	    }

	    if (iCandWordCount == iMaxCandWord) {
		if (i < 0)
		    return;
	    }
	    else
		i++;
	}
	else {
	    for (; i < iCandWordCount; i++) {
		if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) > 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iHit < record->iHit))
		    break;
	    }
	    if (i == iMaxCandWord)
		return;
	}
	break;

    case AD_FAST:
	if (mode == SM_PREV) {
	    for (i = iCandWordCount - 1; i >= 0; i--) {
		if (tableCandWord[i].flag) {
		    if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) < 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iIndex >= record->iIndex))
			break;
		}
	    }

	    if (iCandWordCount == iMaxCandWord) {
		if (i < 0)
		    return;
	    }
	    else
		i++;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) > 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iIndex < record->iIndex))
		    break;
	    }

	    if (i == iMaxCandWord)
		return;
	}
	break;
    }

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    for (j = 0; j < i; j++) {
		tableCandWord[j].flag = tableCandWord[j + 1].flag;
		if (tableCandWord[j].flag)
		    tableCandWord[j].candWord.record = tableCandWord[j + 1].candWord.record;
		else
		    tableCandWord[j].candWord.autoPhrase = tableCandWord[j + 1].candWord.autoPhrase;
	    }
	}
	else {
	    for (j = iCandWordCount; j > i; j--) {
		tableCandWord[j].flag = tableCandWord[j - 1].flag;
		if (tableCandWord[j].flag)
		    tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
		else
		    tableCandWord[j].candWord.autoPhrase = tableCandWord[j - 1].candWord.autoPhrase;
	    }
	}
    }
    else {
	j = iCandWordCount;
	if (iCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    tableCandWord[j].flag = tableCandWord[j - 1].flag;
	    if (tableCandWord[j].flag)
		tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
	    else
		tableCandWord[j].candWord.autoPhrase = tableCandWord[j - 1].candWord.autoPhrase;
	}
    }

    tableCandWord[i].flag = True;
    tableCandWord[i].candWord.record = record;

    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
}

void TableResetFlags (void)
{
    RECORD         *record = recordHead->next;
    INT16           i;

    while (record != recordHead) {
	record->flag = False;
	record = record->next;
    }

    for (i = 0; i < iAutoPhrase; i++)
	autoPhrase[i].flag = False;
}

void TableSetCandWordsFlag (int iCount, Bool flag)
{
    int             i;

    for (i = 0; i < iCount; i++) {
	if (tableCandWord[i].flag)
	    tableCandWord[i].candWord.record->flag = flag;
	else
	    tableCandWord[i].candWord.autoPhrase->flag = flag;
    }
}

Bool HasMatchingKey (void)
{
    char           *str;

    str = strCodeInput;
    while (*str) {
	if (*str++ == table[iTableIMIndex].cMatchingKey)
	    return True;
    }

    return False;
}

int TableCompareCode (char *strUser, char *strDict)
{
    int             i;

    for (i = 0; i < strlen (strUser); i++) {
	if (!strDict[i])
	    return strUser[i];
	if (strUser[i] != table[iTableIMIndex].cMatchingKey || !table[iTableIMIndex].bUseMatchingKey) {
	    if (strUser[i] != strDict[i])
		return (strUser[i] - strDict[i]);
	}
    }

    if (table[iTableIMIndex].bTableExactMatch) {
	if (strlen (strUser) != strlen (strDict))
	    return -999;	//�����һ��ֵ
    }

    return 0;
}

int TableFindFirstMatchCode (void)
{
    int             i = 0;

    if (!recordHead)
	return -1;

    if (table[iTableIMIndex].bUseMatchingKey && (strCodeInput[0] == table[iTableIMIndex].cMatchingKey))
	i = 0;
    else {
	while (strCodeInput[0] != recordIndex[i].cCode)
	    i++;
    }

    currentRecord = recordIndex[i].record;
    while (currentRecord != recordHead) {
	if (!TableCompareCode (strCodeInput, currentRecord->strCode)) {
	    if (CheckHZCharset (currentRecord->strHZ))
		return i;
	}
	currentRecord = currentRecord->next;
	i++;
    }

    return -1;			//Not found
}

/*
 * �������
 * bMode=True��ʾ������ʣ���ʱ����һ���������롣�����ֻ�ж�������ʱ���ض������룬����һ������
 */
RECORD         *TableFindCode (char *strHZ, Bool bMode)
{
    RECORD         *recShort = NULL;	//��¼���������λ��
    int             i;

    for (i = 0; i < iSingleHZCount; i++) {
	if (!strcmp (tableSingleHZ[i]->strHZ, strHZ) && !IsIgnoreChar (tableSingleHZ[i]->strCode[0])) {
	    if (!bMode)
		return tableSingleHZ[i];

	    if (strlen (tableSingleHZ[i]->strCode) == 2)
		recShort = tableSingleHZ[i];
	    else if (strlen (tableSingleHZ[i]->strCode) > 2)
		return tableSingleHZ[i];
	}
    }

    return recShort;
}

/*
 * ������ŵ�������˳����Ŵ�1��ʼ
 * ��ָ������/�ʵ�����ͬ���������ǰ��
 */
void TableAdjustOrderByIndex (int iIndex)
{
    RECORD         *recTemp;

    if (!(tableCandWord[iIndex - 1].flag))
	return;

    recTemp = tableCandWord[iIndex - 1].candWord.record;
    while (!strcmp (recTemp->strCode, recTemp->prev->strCode))
	recTemp = recTemp->prev;
    if (recTemp == tableCandWord[iIndex - 1].candWord.record)	//˵���Ѿ��ǵ�һ��
	return;

    //��ָ������/�ʷŵ�recTempǰ
    tableCandWord[iIndex - 1].candWord.record->prev->next = tableCandWord[iIndex - 1].candWord.record->next;
    tableCandWord[iIndex - 1].candWord.record->next->prev = tableCandWord[iIndex - 1].candWord.record->prev;
    recTemp->prev->next = tableCandWord[iIndex - 1].candWord.record;
    tableCandWord[iIndex - 1].candWord.record->prev = recTemp->prev;
    recTemp->prev = tableCandWord[iIndex - 1].candWord.record;
    tableCandWord[iIndex - 1].candWord.record->next = recTemp;

    iTableChanged++;
    if (iTableChanged == 5)
	SaveTableDict ();
}

/*
 * �������ɾ�����飬��Ŵ�1��ʼ
 */
void TableDelPhraseByIndex (int iIndex)
{
    if (!(tableCandWord[iIndex - 1].flag))
	return;

    if (strlen (tableCandWord[iIndex - 1].candWord.record->strHZ) <= 2)
	return;

    TableDelPhrase (tableCandWord[iIndex - 1].candWord.record);
}

/*
 * �����ִ�ɾ������
 */
void TableDelPhraseByHZ (char *strHZ)
{
    RECORD         *recTemp;

    recTemp = TableFindPhrase (strHZ);
    if (recTemp)
	TableDelPhrase (recTemp);
}

void TableDelPhrase (RECORD * record)
{
    record->prev->next = record->next;
    record->next->prev = record->prev;

    free (record->strCode);
    free (record->strHZ);
    free (record);

    table[iTableIMIndex].iRecordCount--;

    SaveTableDict ();
}

/*
 *�ж�ĳ�������ǲ����Ѿ��ڴʿ���,�з���NULL���޷��ز����
 */
RECORD         *TableHasPhrase (char *strCode, char *strHZ)
{
    RECORD         *recTemp;
    int             i;

    i = 0;
    while (strCode[0] != recordIndex[i].cCode)
	i++;

    recTemp = recordIndex[i].record->prev;
    do {
	if (strcmp (recTemp->strCode, strCode) > 0)
	    break;
	else if (!strcmp (recTemp->strCode, strCode)) {
	    if (!strcmp (recTemp->strHZ, strHZ))	//�ô����Ѿ��ڴʿ���
		return NULL;
	}

	recTemp = recTemp->next;
    } while (recTemp != recordHead);
    
    return recTemp;
}

/*
 *�����ִ��жϴʿ����Ƿ���ĳ����/��
 */
RECORD         *TableFindPhrase (char *strHZ)
{
    RECORD         *recTemp;
    char            strTemp[3];
    int             i;

    //���ȣ��Ȳ��ҵ�һ�����ֵı���
    strTemp[0] = strHZ[0];
    strTemp[1] = strHZ[1];
    strTemp[2] = '\0';

    recTemp = TableFindCode (strTemp, True);
    if (!recTemp)
	return (RECORD *) NULL;

    //Ȼ����ݸñ����ҵ���������ʼ��
    i = 0;
    while (recTemp->strCode[0] != recordIndex[i].cCode)
	i++;

    recTemp = recordIndex[i].record;
    while (recTemp != recordHead) {
	if (recTemp->strCode[0] != recordIndex[i].cCode)
	    break;
	if (!strcmp (recTemp->strHZ, strHZ))
	    return recTemp;

	recTemp = recTemp->next;
    }

    return (RECORD *) NULL;
}

void TableInsertPhrase (char *strCode, char *strHZ)
{
    RECORD         *insertPoint, *dictNew;

    insertPoint = TableHasPhrase (strCode, strHZ);

    if (!insertPoint)
	return;

    dictNew = (RECORD *) malloc (sizeof (RECORD));
    dictNew->strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
    strcpy (dictNew->strCode, strCode);
    dictNew->strHZ = (char *) malloc (sizeof (char) * (strlen (strHZ) + 1));
    strcpy (dictNew->strHZ, strHZ);
    dictNew->iHit = 0;
    dictNew->iIndex = iTableIndex;

    dictNew->prev = insertPoint->prev;
    insertPoint->prev->next = dictNew;
    insertPoint->prev = dictNew;
    dictNew->next = insertPoint;

    table[iTableIMIndex].iRecordCount++;

    SaveTableDict ();
}

void TableCreateNewPhrase (void)
{
    int             i;

    messageDown[0].strMsg[0] = '\0';
    for (i = iTableNewPhraseHZCount; i > 0; i--)
	strcat (messageDown[0].strMsg, hzLastInput[iHZLastInputCount - i].strHZ);

    TableCreatePhraseCode (messageDown[0].strMsg);

    if (!bCanntFindCode)
	strcpy (messageDown[1].strMsg, strNewPhraseCode);
    else
	strcpy (messageDown[1].strMsg, "????");
}

void TableCreatePhraseCode (char *strHZ)
{
    char           *str[table[iTableIMIndex].iCodeLength];
    unsigned char   i;
    unsigned char   i1, i2;
    int             iLen;
    char            strTemp[3];
    RECORD         *recTemp;

    strTemp[2] = '\0';
    bCanntFindCode = False;
    iLen = strlen (strHZ) / 2;
    if (iLen >= table[iTableIMIndex].iCodeLength) {
	i2 = table[iTableIMIndex].iCodeLength;
	i1 = 1;
    }
    else {
	i2 = iLen;
	i1 = 0;
    }

    for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	if (table[iTableIMIndex].rule[i].iWords == i2 && table[iTableIMIndex].rule[i].iFlag == i1)
	    break;
    }

    for (i1 = 0; i1 < table[iTableIMIndex].iCodeLength; i1++) {
	if (table[iTableIMIndex].rule[i].rule[i1].iFlag) {
	    strTemp[0] = strHZ[(table[iTableIMIndex].rule[i].rule[i1].iWhich - 1) * 2];
	    strTemp[1] = strHZ[(table[iTableIMIndex].rule[i].rule[i1].iWhich - 1) * 2 + 1];
	}
	else {
	    strTemp[0] = strHZ[(iLen - table[iTableIMIndex].rule[i].rule[i1].iWhich) * 2];
	    strTemp[1] = strHZ[(iLen - table[iTableIMIndex].rule[i].rule[i1].iWhich) * 2 + 1];
	}

	recTemp = TableFindCode (strTemp, True);

	if (!recTemp) {
	    bCanntFindCode = True;
	    break;
	}

	str[i1] = recTemp->strCode;
	strNewPhraseCode[i1] = str[i1][table[iTableIMIndex].rule[i].rule[i1].iIndex - 1];
    }
}

/*
 * ��ȡ�����ѡ���б�
 */
INPUT_RETURN_VALUE TableGetLegendCandWords (SEARCH_MODE mode)
{
    char            strTemp[3];
    int             iLength;
    int             i;
    RECORD         *tableLegend = NULL;
    unsigned int    iTableTotalLengendCandCount = 0;

    if (!strTableLegendSource[0])
	return IRV_TO_PROCESS;

    iLength = strlen (strTableLegendSource);

    if (mode == SM_FIRST) {
	iCurrentLegendCandPage = 0;
	iLegendCandPageCount = 0;
	TableResetFlags ();
    }
    else {
	if (!iLegendCandPageCount)
	    return IRV_TO_PROCESS;

	if (mode == SM_NEXT) {
	    if (iCurrentLegendCandPage == iLegendCandPageCount)
		return IRV_DO_NOTHING;

	    iCurrentLegendCandPage++;
	}
	else {
	    if (!iCurrentLegendCandPage)
		return IRV_DO_NOTHING;

	    TableSetCandWordsFlag (iLegendCandWordCount, False);
	    iCurrentLegendCandPage--;
	}
    }

    iLegendCandWordCount = 0;
    tableLegend = recordHead->next;

    while (tableLegend != recordHead) {
	if (((mode == SM_PREV) ^ (!tableLegend->flag)) && ((iLength + 2) == strlen (tableLegend->strHZ))) {
	    if (!strncmp (tableLegend->strHZ, strTableLegendSource, iLength) && tableLegend->strHZ[iLength] && CheckHZCharset (tableLegend->strHZ)) {
		if (mode == SM_FIRST)
		    iTableTotalLengendCandCount++;
		TableAddLegendCandWord (tableLegend, mode);
	    }
	}

	tableLegend = tableLegend->next;
    }

    TableSetCandWordsFlag (iLegendCandWordCount, True);

    if (mode == SM_FIRST && bDisablePagingInLegend)
	iLegendCandPageCount = iTableTotalLengendCandCount / iMaxCandWord - ((iTableTotalLengendCandCount % iMaxCandWord) ? 0 : 1);

    uMessageUp = 2;
    strcpy (messageUp[0].strMsg, "���룺");
    messageUp[0].type = MSG_TIPS;
    strcpy (messageUp[1].strMsg, strTableLegendSource);
    messageUp[1].type = MSG_INPUT;

    strTemp[1] = '.';
    strTemp[2] = '\0';
    uMessageDown = 0;
    for (i = 0; i < iLegendCandWordCount; i++) {
	if (i == 9)
	    strTemp[0] = '0';
	else
	    strTemp[0] = i + 1 + '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;

	strcpy (messageDown[uMessageDown].strMsg, tableCandWord[i].candWord.record->strHZ + strlen (strTableLegendSource));
	if (i != (iLegendCandWordCount - 1)) {
#ifdef _USE_XFT
	    strcat (messageDown[uMessageDown].strMsg, "  ");
#else
	    strcat (messageDown[uMessageDown].strMsg, " ");
#endif
	}
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
    }

    bIsInLegend = (iLegendCandWordCount != 0);

    return IRV_DISPLAY_CANDWORDS;
}

void TableAddLegendCandWord (RECORD * record, SEARCH_MODE mode)
{
    int             i, j;

    if (mode == SM_PREV) {
	for (i = iLegendCandWordCount - 1; i >= 0; i--) {
	    if (tableCandWord[i].candWord.record->iHit >= record->iHit)
		break;
	}

	if (iLegendCandWordCount == iMaxCandWord) {
	    if (i < 0)
		return;
	}
	else
	    i++;
    }
    else {
	for (i = 0; i < iLegendCandWordCount; i++) {
	    if (tableCandWord[i].candWord.record->iHit < record->iHit)
		break;
	}
	if (i == iMaxCandWord)
	    return;
    }

    if (mode == SM_PREV) {
	if (iLegendCandWordCount == iMaxCandWord) {
	    for (j = 0; j < i; j++)
		tableCandWord[j].candWord.record = tableCandWord[j + 1].candWord.record;
	}
	else {
	    for (j = iLegendCandWordCount; j > i; j--)
		tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
	}
    }
    else {
	j = iLegendCandWordCount;
	if (iLegendCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--)
	    tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
    }

    tableCandWord[i].flag = True;
    tableCandWord[i].candWord.record = record;

    if (iLegendCandWordCount != iMaxCandWord)
	iLegendCandWordCount++;
}

char           *TableGetLegendCandWord (int iIndex)
{
    if (iLegendCandWordCount) {
	if (iIndex > (iLegendCandWordCount - 1))
	    iIndex = iLegendCandWordCount - 1;

	tableCandWord[iIndex].candWord.record->iHit++;
	strcpy (strTableLegendSource, tableCandWord[iIndex].candWord.record->strHZ + strlen (strTableLegendSource));
	TableGetLegendCandWords (SM_FIRST);

	return strTableLegendSource;
    }

    return NULL;
}

INPUT_RETURN_VALUE TableGetFHCandWords (SEARCH_MODE mode)
{
    char            strTemp[3];
    int             i;

    if (!iFH)
	return IRV_DISPLAY_MESSAGE;

    strTemp[1] = '.';
    strTemp[2] = '\0';
    uMessageDown = 0;

    if (mode == SM_FIRST) {
	iCandPageCount = iFH / iMaxCandWord - ((iFH % iMaxCandWord) ? 0 : 1);
	iCurrentCandPage = 0;
    }
    else {
	if (!iCandPageCount)
	    return IRV_TO_PROCESS;

	if (mode == SM_NEXT) {
	    if (iCurrentCandPage == iCandPageCount)
		return IRV_DO_NOTHING;

	    iCurrentCandPage++;
	}
	else {
	    if (!iCurrentCandPage)
		return IRV_DO_NOTHING;

	    iCurrentCandPage--;
	}
    }

    for (i = 0; i < iMaxCandWord; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;
	strcpy (messageDown[uMessageDown].strMsg, fh[iCurrentCandPage * iMaxCandWord + i].strFH);
	if (i != (iMaxCandWord - 1)) {
#ifdef _USE_XFT
	    strcat (messageDown[uMessageDown].strMsg, "  ");
#else
	    strcat (messageDown[uMessageDown].strMsg, " ");
#endif
	}
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
	if ((iCurrentCandPage * iMaxCandWord + i) >= (iFH - 1)) {
	    i++;
	    break;
	}
    }

    iCandWordCount = i;
    return IRV_DISPLAY_CANDWORDS;
}

char           *TableGetFHCandWord (int iIndex)
{
    uMessageDown = 0;

    if (iCandWordCount) {
	if (iIndex > (iCandWordCount - 1))
	    iIndex = iCandWordCount - 1;

	return fh[iCurrentCandPage * iMaxCandWord + iIndex].strFH;
    }

    return NULL;
}

Bool TablePhraseTips (void)
{
    RECORD         *recTemp = NULL;
    char            strTemp[PHRASE_MAX_LENGTH * 2 + 1] = "\0";
    INT16           i, j;

    if (!recordHead)
	return False;

    //������������һ�����飬��������Ͳ���Ҫ��
    if (lastIsSingleHZ != 1)
	return False;

    j = (iHZLastInputCount > PHRASE_MAX_LENGTH) ? iHZLastInputCount - PHRASE_MAX_LENGTH : 0;
    for (i = j; i < iHZLastInputCount; i++)
	strcat (strTemp, hzLastInput[i].strHZ);
    //���ֻ��һ�����֣��������Ҳ����Ҫ��
    if (strlen (strTemp) < 4)
	return False;

    //����Ҫ�ж��ǲ����Ѿ��ڴʿ���
    for (i = 0; i < (iHZLastInputCount - j - 1); i++) {
	recTemp = TableFindPhrase (strTemp + i * 2);
	if (recTemp) {
	    strcpy (messageUp[0].strMsg, "�ʿ����д��� ");
	    messageUp[0].type = MSG_TIPS;
	    strcpy (messageUp[1].strMsg, strTemp + i * 2);
	    messageUp[1].type = MSG_INPUT;
	    uMessageUp = 2;

	    strcpy (messageDown[0].strMsg, "����Ϊ ");
	    messageDown[0].type = MSG_FIRSTCAND;
	    strcpy (messageDown[1].strMsg, recTemp->strCode);
	    messageDown[1].type = MSG_CODE;
	    strcpy (messageDown[2].strMsg, " ^DELɾ��");
	    messageDown[2].type = MSG_TIPS;
	    uMessageDown = 3;
	    bTablePhraseTips = True;
	    bShowCursor = False;

	    return True;
	}
    }

    return False;
}

void TableCreateAutoPhrase (INT8 iCount)
{
    char            strHZ[table[iTableIMIndex].iAutoPhrase * 2 + 1];
    INT16           i, j, k;

    /*
     * Ϊ�����Ч�ʣ��˴�ֻ����������¼���ֹ��ɵĴ���
     */
    j = iHZLastInputCount - table[iTableIMIndex].iAutoPhrase - iCount;
    if (j < 0)
	j = 0;
    for (; j < iHZLastInputCount - 1; j++) {
	for (i = table[iTableIMIndex].iAutoPhrase; i >= 2; i--) {
	    if ((j + i - 1) > iHZLastInputCount)
		continue;
	    strcpy (strHZ, hzLastInput[j].strHZ);
	    for (k = 1; k < i; k++)
		strcat (strHZ, hzLastInput[j + k].strHZ);

	    //��ȥ���ظ��Ĵ���
	    for (k = 0; k < iAutoPhrase; k++) {
		if (!strcmp (autoPhrase[k].strHZ, strHZ))
		    goto _next;
	    }

	    TableCreatePhraseCode (strHZ);
	    if (iAutoPhrase != iTotalAutoPhrase) {
		autoPhrase[iAutoPhrase].flag = False;
		strcpy (autoPhrase[iAutoPhrase].strCode, strNewPhraseCode);
		strcpy (autoPhrase[iAutoPhrase].strHZ, strHZ);
		autoPhrase[iAutoPhrase].iSelected = 0;
		iAutoPhrase++;
	    }
	    else {
		insertPoint->flag = False;
		strcpy (insertPoint->strCode, strNewPhraseCode);
		strcpy (insertPoint->strHZ, strHZ);
		insertPoint->iSelected = 0;
		insertPoint = insertPoint->next;
	    }
	  _next:
	    continue;
	}
    }
}

void UpdateHZLastInput (char *str)
{
    int             i, j;

    for (i = 0; i < strlen (str) / 2; i++) {
	if (iHZLastInputCount < MAX_HZ_SAVED)
	    iHZLastInputCount++;
	else {
	    for (j = 0; j < (iHZLastInputCount - 1); j++) {
		hzLastInput[j].strHZ[0] = hzLastInput[j + 1].strHZ[0];
		hzLastInput[j].strHZ[1] = hzLastInput[j + 1].strHZ[1];
	    }
	}
	hzLastInput[iHZLastInputCount - 1].strHZ[0] = str[2 * i];
	hzLastInput[iHZLastInputCount - 1].strHZ[1] = str[2 * i + 1];
	hzLastInput[iHZLastInputCount - 1].strHZ[2] = '\0';
    }

    if (table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase)
	TableCreateAutoPhrase ((INT8) (strlen (str) / 2));
}
