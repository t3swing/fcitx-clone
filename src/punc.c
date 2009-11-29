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
#include "punc.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "ime.h"
#include "tools.h"

ChnPunc        *chnPunc = (ChnPunc *) NULL;

extern int      iCodeInputCount;

/**
 * @brief ���ر��ʵ�
 * @param void
 * @return void
 * @note �ļ������ݵĸ�ʽΪ�� ��Ӧ��Ӣ�ķ��� ���ı�� <���ı��>
 * ���ر��ʵ䡣���ʵ䶨����һ����ת���������롮.����ֱ��ת���ɡ�����
 */
int LoadPuncDict (void)
{
    FILE           *fpDict;				// �ʵ��ļ�ָ��
    int             iRecordNo;
    char            strText[11];
    char            strPath[PATH_MAX];	// �ʵ��ļ���ȫ������������·����
    char           *pstr;				// ��ʱָ��
    int             i;

    fpDict = UserConfigFile(PUNC_DICT_FILENAME, "rt", NULL);

    // ������ļ������ڣ���ʹ�ð�װĿ¼�µ��ļ�
    if (!fpDict) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, PUNC_DICT_FILENAME);
	fpDict = fopen (strPath, "rt");
	if (!fpDict) {
	    printf ("Can't open Chinese punc file: %s\n", strPath);
	    return False;
	}
    }

    /* ����ʵ������ж��ٵ�����
     * ��������ǳ��򵥣����Ǽ�����ļ��ж����У��������У���
     * ��Ϊ���У����������ȥ�����ԣ���������ڴ���˷�����
     * û��һ�����о����˷�sizeof (ChnPunc)�ֽ��ڴ�*/
    iRecordNo = CalculateRecordNumber (fpDict);
    // ����ռ䣬���������Щ���ݡ����û�м���Ƿ����뵽�ڴ棬�ϸ�˵��С����
    // chnPunc��һ��ȫ�ֱ���
    chnPunc = (ChnPunc *) malloc (sizeof (ChnPunc) * (iRecordNo + 1));

    iRecordNo = 0;

    // �������ѭ��������һ��һ�еĶ���ʵ��ļ������ݡ���������뵽chnPunc����ȥ��
    for (;;) {
	if (!fgets (strText, 10, fpDict))
	    break;
	i = strlen (strText) - 1;

	// ���ҵ����һ���ַ�
	while ((strText[i] == '\n') || (strText[i] == ' ')) {
	    if (!i)
		break;
	    i--;
	}

	// ����ҵ������г��롣���ǿ���ʱ���϶��Ҳ��������ԣ�Ҳ���Թ��˿��еĴ���
	if (i) {
	    strText[i + 1] = '\0';				// ���ַ��������Ӹ����
	    pstr = strText;						// ��pstrָ���һ���ǿ��ַ�
	    while (*pstr == ' ')
		pstr++;
	    chnPunc[iRecordNo].ASCII = *pstr++;	// ����������ķ�������Ӧ��ASCII��ֵ
	    while (*pstr == ' ')				// Ȼ�󣬽�pstrָ����һ���ǿ��ַ�
		pstr++;

	    chnPunc[iRecordNo].iCount = 0;		// �÷����м���ת��������Ӣ��"�Ϳ���ת���ɡ��͡�
	    chnPunc[iRecordNo].iWhich = 0;		// ��ʾ�÷��ŵ�����״̬�������ڵڼ���ת������"��iWhich��ʾ��ת���ɡ����ǡ�
	    // ���ν���ASCII������Ӧ�ķ��ŷ��뵽�ṹ��
	    while (*pstr) {
		i = 0;
		// ��Ϊ���ķ��Ŷ���˫�ֽڵģ����ԣ�Ҫһֱ�������֪���ո�����ַ�����ĩβ
		while (*pstr != ' ' && *pstr) {
		    chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i] = *pstr;
		    i++;
		    pstr++;
		}

		// ÿ�����ķ�����'\0'����
		chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i] = '\0';
		while (*pstr == ' ')
		    pstr++;
		chnPunc[iRecordNo].iCount++;
	    }

	    iRecordNo++;
	}
    }

    chnPunc[iRecordNo].ASCII = '\0';
    fclose (fpDict);

    return True;
}

void FreePunc (void)
{
    if (!chnPunc)
	return;

    free (chnPunc);
    chnPunc = (ChnPunc *) NULL;
}

/*
 * �����ַ��õ���Ӧ�ı�����
 * ������ַ����ڱ����ż��У��򷵻�NULL
 */
char           *GetPunc (int iKey)
{
    int             iIndex = 0;
    char           *pPunc;

    if (!chnPunc)
	return (char *) NULL;

    while (chnPunc[iIndex].ASCII) {
	if (chnPunc[iIndex].ASCII == iKey) {
	    pPunc = chnPunc[iIndex].strChnPunc[chnPunc[iIndex].iWhich];
	    chnPunc[iIndex].iWhich++;
	    if (chnPunc[iIndex].iWhich >= chnPunc[iIndex].iCount)
		chnPunc[iIndex].iWhich = 0;
	    return pPunc;
	}
	iIndex++;
    }

    return (char *) NULL;
}
