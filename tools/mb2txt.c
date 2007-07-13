#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#define MAX_CODE_LENGTH 12

typedef struct _RULE_RULE
{
  unsigned char iFlag;	// 1 --> ����	0 --> ����
  unsigned char iWhich;		//�ڼ�����
  unsigned char iIndex;		//�ڼ�������
}
RULE_RULE;

typedef struct _RULE
{
  unsigned char iWords;		//���ٸ���
  unsigned char iFlag;	//1 --> ���ڵ���iWords	0 --> ����iWords
  RULE_RULE *rule;
}
RULE;


int main (int argc, char *argv[])
{
    char            strCode[100];
    char            strHZ[100];
    FILE           *fpDict;
    unsigned int    i = 0;
    unsigned int    iTemp;
    unsigned int    j;
    unsigned char   iLen;
    unsigned char   iRule;

    if ( argc!=2 ) {
    	printf("\nUsage: mb2txt <Source File>\n\n");
    	exit (1);
    }

    fpDict=fopen(argv[1],"rb");
    if ( !fpDict ) {
    	printf("\nCan not read source file!\n\n");
    	exit (2);
    }
    //�ȶ�ȡ������Ϣ
    fread(&iTemp,sizeof(unsigned int),1,fpDict);
    fread(strCode,sizeof(char), iTemp+1,fpDict);
    printf("����=%s\n",strCode);
    fread(&iLen,sizeof(unsigned char),1,fpDict);
    printf("�볤=%d\n",iLen);
    fread(&iTemp,sizeof(unsigned int),1,fpDict);
    fread(strCode,sizeof(char), iTemp+1,fpDict);
    if ( iTemp )
    	printf("����ַ�=%s\n",strCode);

    fread(&iRule, sizeof(unsigned char),1, fpDict);
    if ( iRule ) {		//��ʾ����ʹ���
    	printf("[��ʹ���]\n");
	for ( i=0; i<iLen-1;i++ ) {		
		fread(&iRule,sizeof(unsigned char),1,fpDict);
		printf("%c", (iRule)? 'a':'e');
		fread(&iRule,sizeof(unsigned char),1,fpDict);
		printf("%d=",iRule);
		for ( iTemp=0; iTemp<iLen;iTemp++) {
			fread(&iRule,sizeof(unsigned char),1,fpDict);
			printf("%c", (iRule)?'p':'n');
			fread(&iRule,sizeof(unsigned char),1,fpDict);
			printf("%d", iRule);
			fread(&iRule,sizeof(unsigned char),1,fpDict);
			printf("%d", iRule);
			if ( iTemp!=(iLen-1) )
				printf("+");
		}
		printf("\n");
	}
    }
    
    printf("[����]\n");
    fread(&j,sizeof(unsigned int),1,fpDict);
    for ( i=0; i<j; i++) {
    	fread(strCode, sizeof(char), iLen+1, fpDict);
	fread(&iTemp, sizeof(unsigned int), 1, fpDict);
	fread(strHZ, sizeof(char), iTemp, fpDict);
	printf("%s %s\n",strCode,strHZ);

	fread(&iTemp,sizeof(unsigned int),1,fpDict);
	fread(&iTemp,sizeof(unsigned int),1,fpDict);
    }

    fclose (fpDict);
    return 0;
}
