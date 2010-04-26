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
#include <stdio.h>
#include <limits.h>

#include "im/special/QuickPhrase.h"
#include "ui/InputWindow.h"
#include "tools/tools.h"
#include "tools/array.h"

uint uQuickPhraseCount;
ARRAY *quickPhrases = NULL;
int iFirstQuickPhrase = -1;
int iLastQuickPhrase;
QUICK_PHRASE *quickPhraseCandWords[MAX_CAND_WORD];

#define MIN(a,b) ((a) < (b)?(a) : (b))

extern int iCandPageCount;
extern int iCandWordCount;
extern int iCurrentCandPage;
extern int iMaxCandWord;
extern char strCodeInput[];
extern uint uMessageDown;
extern MESSAGE         messageDown[];
extern char strStringGet[];

int PhraseCmp(const void* a, const void* b)
{
    return strcmp(((QUICK_PHRASE**)a)[0]->strCode, ((QUICK_PHRASE**)b)[0]->strCode);
}

int PhraseCmpA(const void* a, const void* b)
{
    int res,len;
    len = strlen(((QUICK_PHRASE**)a)[0]->strCode);
    res = strncmp(((QUICK_PHRASE**)a)[0]->strCode, ((QUICK_PHRASE**)b)[0]->strCode, len);
    if (res)
        return res;
    else
    {
        return 1;
    }

}

/**
 * @brief 加载快速输入词典
 * @param void
 * @return void
 * @note 快速输入是在;的行为定义为2,并且输入;以后才可以使用的。
 * 加载快速输入词典.如：输入“zg”就直接出现“中华人民共和国”等等。
 * 文件中每一行数据的定义为：<字符组合> <短语>
 * 如：“zg 中华人民共和国”
 */
void LoadQuickPhrase(void)
{
    FILE *fp;
    char            strPath[PATH_MAX];
    char strCode[QUICKPHRASE_CODE_LEN+1];
    char strPhrase[QUICKPHRASE_PHRASE_LEN*2+1];
    QUICK_PHRASE *tempQuickPhrase;

    uQuickPhraseCount=0;

    fp = UserConfigFile("QuickPhrase.mb", "rt", NULL);
    if (!fp) {
    strcpy (strPath, PKGDATADIR "/data/");
    strcat (strPath, "QuickPhrase.mb");

    /* add by zxd begin */
        if( access( strPath, 0) && getenv( "FCITXDIR") ) {
            strcpy( strPath, getenv( "FCITXDIR" ) );
            strcat (strPath, "/share/fcitx/data/AutoEng.dat");
        }
        /* add by zxd end */
    
    fp = fopen (strPath, "rt");
    if (!fp)
        return;
    }

    // 这儿的处理比较简单。因为是单索引对单值。
    // 应该注意的是，它在内存中是以单链表保存的。
    for (;;) {
    if (EOF==fscanf (fp, "%s", strCode))
        break;
    if (EOF==fscanf (fp, "%s", strPhrase))
        break;

    tempQuickPhrase=(QUICK_PHRASE *)malloc(sizeof(QUICK_PHRASE));
    strcpy(tempQuickPhrase->strCode,strCode);
    strcpy(tempQuickPhrase->strPhrase,strPhrase);

    ArrayAddPointer(quickPhrases, &tempQuickPhrase);

    }

    if (quickPhrases)
    {
        ArraySort(quickPhrases, PhraseCmp);
    }

    fclose(fp);
}

void FreeQuickPhrase(void)
{
    QUICK_PHRASE *quickPhrase;

    if ( !quickPhrases )
    return;

    int i;
    ArrayForeach(i, quickPhrases)
    {
        quickPhrase = ArrayCell(quickPhrases, i, QUICK_PHRASE*);
        free(quickPhrase);
    }
    ArrayFree(quickPhrases);
}

INPUT_RETURN_VALUE QuickPhraseDoInput (int iKey)
{
    int retVal;

    if (iKey >= '0' && iKey <= '9') {
    if (!iCandWordCount)
        return IRV_TO_PROCESS;

    iKey -= '0';
    if (iKey == 0)
        iKey = 10;

    if (iKey > iCandWordCount)
        retVal = IRV_DO_NOTHING;
    else {
        strcpy (strStringGet, quickPhraseCandWords[iKey-1]->strPhrase);
        retVal = IRV_GET_CANDWORDS;
        uMessageDown = 0;
    }
    }
    else if (iKey==' ') {
    if (!iCandWordCount)
        retVal = IRV_TO_PROCESS;
    else {
        strcpy (strStringGet, quickPhraseCandWords[0]->strPhrase);
        retVal = IRV_GET_CANDWORDS;
        uMessageDown = 0;
    }
    }
    else
    retVal = IRV_TO_PROCESS;

    return retVal;
}

INPUT_RETURN_VALUE QuickPhraseGetCandWords (SEARCH_MODE mode)
{
    int i, iInputLen;
    QUICK_PHRASE searchKey, *pKey, **currentQuickPhrase;
    char strTemp[2];

    pKey = &searchKey;

    if ( !quickPhrases )
        return IRV_DISPLAY_MESSAGE;

    iInputLen = strlen(strCodeInput);
    if (iInputLen > QUICKPHRASE_CODE_LEN)
        return IRV_DISPLAY_MESSAGE;

    strcpy(searchKey.strCode, strCodeInput);

    if (mode==SM_FIRST) {
    currentQuickPhrase=ArrayPos(quickPhrases, 0);
    iCandPageCount=0;
    iCurrentCandPage=0;
    iCandWordCount=0;
    currentQuickPhrase = ArrayBsearch(&pKey, quickPhrases, False, PhraseCmp);
    iFirstQuickPhrase = ArrayIndex(quickPhrases, currentQuickPhrase);
    iLastQuickPhrase = ArrayIndex(quickPhrases, ArrayBsearch(&pKey, quickPhrases, False, PhraseCmpA));

    iCandPageCount = (iLastQuickPhrase - iFirstQuickPhrase) / iMaxCandWord;
    if ( !currentQuickPhrase || strncmp(strCodeInput,currentQuickPhrase[0]->strCode,iInputLen) ) {
        uMessageDown = 0;
        currentQuickPhrase = NULL;
        return IRV_DISPLAY_MESSAGE;
    }

    }
    else if (mode==SM_NEXT) {
    if (iCurrentCandPage>=iCandPageCount)
        return IRV_DO_NOTHING;
    iCandWordCount=0;
    iCurrentCandPage++;
    }
    else {
    if (iCurrentCandPage <= 0)
        return IRV_DO_NOTHING;
    iCandWordCount=0;
    iCurrentCandPage--;
    }

    ArrayForeachFrom( iFirstQuickPhrase + iCurrentCandPage * iMaxCandWord, i, quickPhrases)
    {
        currentQuickPhrase = ArrayPos(quickPhrases, i);
        if (!strncmp(strCodeInput,currentQuickPhrase[0]->strCode,iInputLen)) {
        quickPhraseCandWords[iCandWordCount++]=currentQuickPhrase[0];
        if (iCandWordCount==iMaxCandWord) {
            break;
        }
        }
    }

    if (!iCandWordCount)
        return IRV_DISPLAY_MESSAGE;

    uMessageDown = 0;
    strTemp[1]='\0';

    for (i = 0; i < iCandWordCount; i++) {
    strTemp[0] = i + 1 + '0';
    if (i == 9)
        strTemp[0] = '0';
    strcpy (messageDown[uMessageDown].strMsg, strTemp);
    messageDown[uMessageDown++].type = MSG_INDEX;

    strcpy (messageDown[uMessageDown].strMsg, quickPhraseCandWords[i]->strPhrase);
    messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);

    //编码提示
    strcpy (messageDown[uMessageDown].strMsg, quickPhraseCandWords[i]->strCode + iInputLen);
    if (i != (iCandWordCount - 1))
        strcat (messageDown[uMessageDown].strMsg, " ");
    messageDown[uMessageDown++].type = MSG_CODE;
    }

    return IRV_DISPLAY_CANDWORDS;
}
