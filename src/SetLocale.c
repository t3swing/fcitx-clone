#include "SetLocale.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlocale.h>

char            strUserLocale[20];

extern Bool     bUseGBK;

Bool CheckLocale (char *strHZ)
{
    if (!bUseGBK) {
	//GB2312�ĺ��ֱ������Ϊ����һ���ֽڵ�ֵ��0xA1��0xFE֮��(ʵ��Ϊ0xF7)���ڶ����ֽڵ�ֵ��0xA1��0xFE֮��
	//���ڲ鵽������˵����һ�����ú�ʵ����������
	int             i;

	for (i = 0; i < strlen (strHZ); i++) {
	    if ((unsigned char) strHZ[i] < (unsigned char) 0xA1 || (unsigned char) strHZ[i] > (unsigned char) 0xF7 || (unsigned char) strHZ[i + 1] < (unsigned char) 0xA1 || (unsigned char) strHZ[i + 1] > (unsigned char) 0xFE)
		return False;
	    i++;
	}
    }

    return True;
}
