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
#include "pyParser.h"
#include "ime.h"

#include <stdio.h>
#include <string.h>

#include "pyMapTable.h"
#include "PYFA.h"
#include "sp.h"

extern PYTABLE  PYTable[];
extern ConsonantMap consonantMapTable[];
extern SyllabaryMap syllabaryMapTable[];
extern int      iIMEIndex;
extern Bool     bSP;
extern MHPY     MHPY_C[];
extern Bool     bFullPY;

int IsSyllabary (char *strPY, Bool bMode)
{
    register int    i;

    for (i = 0; syllabaryMapTable[i].cMap; i++) {
	if (bMode) {
	    if (!strncmp (strPY, syllabaryMapTable[i].strPY, strlen (syllabaryMapTable[i].strPY)))
		return i;
	}
	else {
	    if (!strcmp (strPY, syllabaryMapTable[i].strPY))
		return i;
	}
    }

    return -1;
}

int IsConsonant (char *strPY, Bool bMode)
{
    register int    i;

    for (i = 0; consonantMapTable[i].cMap; i++) {
	if (bMode) {
	    if (!strncmp (strPY, consonantMapTable[i].strPY, strlen (consonantMapTable[i].strPY)))
		return i;
	}
	else {
	    if (!strcmp (strPY, consonantMapTable[i].strPY))
		return i;
	}
    }

    return -1;
}

int FindPYFAIndex (char *strPY, Bool bMode)
{
    int             i;
    int             iTemp;

    iTemp = -1;
    for (i = 0; PYTable[i].strPY[0] != '\0'; i++) {
	if (bMode) {
	    if (!strncmp (strPY, PYTable[i].strPY, strlen (PYTable[i].strPY))) {
		if (!PYTable[i].pMH)
		    return i;
		else if (*(PYTable[i].pMH))
		    return i;
	    }
	}
	else {
	    if (!strcmp (strPY, PYTable[i].strPY)) {
		if (!PYTable[i].pMH)
		    return i;
		else if (*(PYTable[i].pMH))
		    return i;
	    }
	}
    }

    return -1;
}

void ParsePY (char *strPY, ParsePYStruct * parsePY, PYPARSEINPUTMODE mode)
{
    char           *strP;
    int             iIndex;
    int             iTemp;
    char            str_Map[3];
    char            strTemp[7];

    parsePY->iMode = PARSE_SINGLEHZ;
    strP = strPY;
    parsePY->iHZCount = 0;

    if (bSP) {
	char            strQP[7];
	char            strJP[3];

	strJP[2] = '\0';

	while (*strP) {
	    strJP[0] = *strP++;
	    strJP[1] = *strP;
	    SP2QP (strJP, strQP);
	    MapPY (strQP, str_Map, mode);

	    if (!*strP) {
		strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
		strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
		break;
	    }

	    iIndex = FindPYFAIndex (strQP, 0);
	    if (iIndex != -1) {
		strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
		strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
		strP++;
	    }
	    else {
		strJP[1] = '\0';
		SP2QP (strJP, strQP);
		if (!MapPY (strQP, str_Map, mode))
		    strcpy (parsePY->strMap[parsePY->iHZCount], strJP);
		else
		    strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
		strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
	    }

	    if (*strP == PY_SEPARATOR) {
		strcat (parsePY->strPYParsed[parsePY->iHZCount - 1], PY_SEPARATOR_S);
		while (*strP == PY_SEPARATOR )
		    strP++;
	    }
	}
    }
    else {
	Bool            bSeperator = False;

	do {
	    iIndex = FindPYFAIndex (strP, 1);

	    if (iIndex != -1) {
		strTemp[0] = PYTable[iIndex].strPY[strlen (PYTable[iIndex].strPY) - 1];
		iTemp = -1;
		if (strTemp[0] == 'g' || strTemp[0] == 'n') {
		    strncpy (strTemp, strP, strlen (PYTable[iIndex].strPY) - 1);
		    strTemp[strlen (PYTable[iIndex].strPY) - 1] = '\0';

		    iTemp = FindPYFAIndex (strTemp, 0);
		    if (iTemp != -1) {
			iTemp = FindPYFAIndex (strP + strlen (PYTable[iTemp].strPY), 1);
			if (iTemp != -1) {
			    if (strlen (PYTable[iTemp].strPY) == 1 || !strcmp ("ng", PYTable[iTemp].strPY))
				iTemp = -1;
			}
			if (iTemp != -1) {
			    strncpy (strTemp, strP, strlen (PYTable[iIndex].strPY) - 1);
			    strTemp[strlen (PYTable[iIndex].strPY) - 1] = '\0';
			}
		    }
		}
		if (iTemp == -1)
		    strcpy (strTemp, PYTable[iIndex].strPY);
		MapPY (strTemp, str_Map, mode);
		strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
		strP += strlen (strTemp);

		if (bSeperator) {
		    bSeperator = False;
		    parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
		    parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
		}
		else
		    parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
		strcat (parsePY->strPYParsed[parsePY->iHZCount++], strTemp);
	    }
	    else {
		if (bFullPY && *strP != PY_SEPARATOR)
		    parsePY->iMode = PARSE_ERROR;

		iIndex = IsConsonant (strP, 1);
		if (-1 != iIndex) {
		    parsePY->iMode = PARSE_ERROR;

		    if (bSeperator) {
			bSeperator = False;
			parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
			parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
		    }
		    else
			parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
		    strcat (parsePY->strPYParsed[parsePY->iHZCount], consonantMapTable[iIndex].strPY);
		    MapPY (consonantMapTable[iIndex].strPY, str_Map, mode);
		    strcpy (parsePY->strMap[parsePY->iHZCount++], str_Map);
		    strP += strlen (consonantMapTable[iIndex].strPY);
		}
		else {
		    iIndex = IsSyllabary (strP, 1);
		    if (-1 != iIndex) {
			if (bSeperator) {
			    bSeperator = False;
			    parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
			    parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
			}
			else
			    parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
			strcat (parsePY->strPYParsed[parsePY->iHZCount], syllabaryMapTable[iIndex].strPY);
			MapPY (syllabaryMapTable[iIndex].strPY, str_Map, mode);
			strcpy (parsePY->strMap[parsePY->iHZCount++], str_Map);

			strP += strlen (syllabaryMapTable[iIndex].strPY);
			if (parsePY->iMode != PARSE_ERROR)
			    parsePY->iMode = PARSE_ABBR;
		    }
		    else {	//�ض��Ƿָ���
			strP++;
			bSeperator = True;
			parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
			parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
			parsePY->strMap[parsePY->iHZCount][0] = '0';
			parsePY->strMap[parsePY->iHZCount][1] = '0';
			parsePY->strMap[parsePY->iHZCount][2] = '\0';
		    }
		}
	    }
	} while (*strP);
    }

    if (strPY[strlen (strPY) - 1] == PY_SEPARATOR && !bSP)
	parsePY->iHZCount++;

    if (parsePY->iMode != PARSE_ERROR) {
	parsePY->iMode = parsePY->iMode & PARSE_ABBR;
	if (parsePY->iHZCount > 1)
	    parsePY->iMode = parsePY->iMode | PARSE_PHRASE;
	else
	    parsePY->iMode = parsePY->iMode | PARSE_SINGLEHZ;
    }
}

/*
 * ��һ��ƴ��(������Ϊ��ĸ����ĸ)ת��Ϊƴ��ӳ��
 * ����TrueΪת���ɹ�������ΪFalse(һ������ΪstrPY����һ����׼��ƴ��)
 */
Bool MapPY (char *strPY, char strMap[3], PYPARSEINPUTMODE mode)
{
    char            str[5];
    int             iIndex;

    //���⴦��eng
    if (!strcmp (strPY, "eng") && MHPY_C[1].bMode) {
	strcpy (strMap, "X0");
	return True;
    }

    strMap[2] = '\0';
    iIndex = IsSyllabary (strPY, 0);
    if (-1 != iIndex) {
	strMap[0] = syllabaryMapTable[iIndex].cMap;
	strMap[1] = mode;
	return True;
    }
    iIndex = IsConsonant (strPY, 0);

    if (-1 != iIndex) {
	strMap[0] = mode;
	strMap[1] = consonantMapTable[iIndex].cMap;
	return True;
    }

    str[0] = strPY[0];
    str[1] = '\0';

    if (strPY[1] == 'h' || strPY[1] == 'g') {
	str[0] = strPY[0];
	str[1] = strPY[1];
	str[2] = '\0';
	iIndex = IsSyllabary (str, 0);
	strMap[0] = consonantMapTable[iIndex].cMap;
	iIndex = IsConsonant (strPY + 2, 0);
	strMap[1] = consonantMapTable[iIndex].cMap;
    }
    else {
	str[0] = strPY[0];
	str[1] = '\0';
	iIndex = IsSyllabary (str, 0);
	if (iIndex == -1)
	    return False;
	strMap[0] = consonantMapTable[iIndex].cMap;
	iIndex = IsConsonant (strPY + 1, 0);
	if (iIndex == -1)
	    return False;
	strMap[1] = consonantMapTable[iIndex].cMap;
    }

    return True;
}

/*
 * ��һ��������ƴ��ӳ��ת��Ϊƴ��������False��ʾʧ�ܣ�
 * һ��ԭ����ƴ��ӳ�䲻��ȷ
 */
Bool MapToPY (char strMap[3], char *strPY)
{
    int             i;

    strPY[0] = '\0';
    if (strMap[0] != ' ') {
	i = 0;
	while (syllabaryMapTable[i].cMap) {
	    if (syllabaryMapTable[i].cMap == strMap[0]) {
		strcpy (strPY, syllabaryMapTable[i].strPY);
		break;
	    }
	    i++;
	}
	if (!strlen (strPY))
	    return False;
    }

    if (strMap[1] != ' ') {
	i = 0;
	while (consonantMapTable[i].cMap) {
	    if (consonantMapTable[i].cMap == strMap[1]) {
		strcat (strPY, consonantMapTable[i].strPY);
		return True;
	    }
	    i++;
	}
    }
    else
	    return True;

    return False;
}

/*
 * �Ƚ�һλƴ��ӳ��
 * 0��ʾ���
 * bָʾ����ĸ������ĸ��True��ʾ��ĸ
 */
int Cmp1Map (char map1, char map2, Bool b)
{
    int             iVal1, iVal2;

    if (map2 == '0' || map1 == '0') {
	if (map1 == ' ' || map2 == ' ' || !bFullPY || bSP)
	    return 0;
    }
    else {
	if (b) {
	    iVal1 = GetMHIndex_S (map1);
	    iVal2 = GetMHIndex_S (map2);
	}
	else {
	    iVal1 = GetMHIndex_C (map1);
	    iVal2 = GetMHIndex_C (map2);
	}
	if (iVal1 == iVal2)
	    if (iVal1 >= 0)
		return 0;
    }

    return (map1 - map2);
}

/*
 * �Ƚ���λƴ��ӳ��
 * 0��ʾ���
 * >0��ʾǰ�ߴ�
 * <0��ʾ���ߴ�
 */
int Cmp2Map (char map1[3], char map2[3])
{
    int             i;

    i = Cmp1Map (map1[0], map2[0], True);
    if (i)
	return i;

    return Cmp1Map (map1[1], map2[1], False);
}

/*
 * �ж�strMap2�Ƿ���strMap1��ƥ��
 * �� ����ֵΪ0
 * �� ����ֵ��Ϊ0
 * *iMatchedLength ��¼�˶����ܹ�ƥ��ĳ���
 */
int CmpMap (char *strMap1, char *strMap2, int *iMatchedLength)
{
    int             val;

    *iMatchedLength = 0;
    for (;;) {
	if (!strMap2[*iMatchedLength])
	    return (strMap1[*iMatchedLength] - strMap2[*iMatchedLength]);
	val = Cmp1Map (strMap1[*iMatchedLength], strMap2[*iMatchedLength], (*iMatchedLength + 1) % 2);
	if (val)
	    return Cmp1Map (strMap1[*iMatchedLength], strMap2[*iMatchedLength], (*iMatchedLength + 1) % 2);
	(*iMatchedLength)++;
    }

    return 0;
}
