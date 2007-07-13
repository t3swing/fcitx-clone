#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#define MAX_CODE_LENGTH 12

typedef struct _RECORD
{
  char            *strCode;
  char            *strHZ;
  struct _RECORD  *next;
  struct _RECORD  *prev;
  unsigned int iHit;
  unsigned int iIndex;
}
RECORD;

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
  char		*p;
  FILE           *fpDict, *fpNew;
  RECORD         *temp,*head,*newRec,*current;
  unsigned int    s = 0;
  int		i;
  unsigned int	iTemp;
  char *pstr=0;
  char strTemp[10];
  unsigned char	bRule;
  RULE	*rule=0;

  unsigned char	iCodeLength=0;
  char		strInputCode[100]="\0";
  char		strIgnoreChars[100]="\0";

  if ( argc!=3 ) {
    printf("\nUsage: mb2txt <Source File> <IM File>\n\n");
    exit (1);
  }

  fpDict=fopen(argv[1],"rt");
  if ( !fpDict ) {
    printf("\nCan not read source file!\n\n");
    exit (2);
  }

  head = (RECORD *) malloc (sizeof (RECORD));
  head->next=head;
  head->prev=head;
  current = head;

  bRule = 0;
  for (;;) {
    if (!fgets (strCode, 100, fpDict))
      break;

    i = strlen (strCode) - 1;
    while (strCode[i] == ' ' || strCode[i] == '\n')
      strCode[i--] = '\0';

    pstr = strCode;
    if (*pstr == ' ')
      pstr++;
    if (pstr[0] == '#')
      continue;

    if (strstr (pstr, "����=") ) {
      pstr += 5;
      strcpy (strInputCode, pstr);
    } else if (strstr (pstr, "�볤=") ) {
      pstr += 5;
      iCodeLength =atoi(pstr);
    } else if (strstr (pstr, "����ַ�=") ) {
      pstr += 9;
      strcpy(strIgnoreChars,pstr);
    } else if ( strstr (pstr,"[����]") )
      break;
    else if ( strstr (pstr,"[��ʹ���]") ) {
      bRule = 1;
      break;
    }
  }

  if ( iCodeLength<=0 || !strInputCode[0] ) {
    printf("Source File Format Error!\n");
    exit (1);
  }

  if ( bRule ) {
    /*
     * ��ʹ�����Ӧ�ñȼ��볤����1
     */
    rule = (RULE *)malloc(sizeof(RULE)*(iCodeLength-1));

    for (iTemp=0;iTemp<(iCodeLength-1);iTemp++) {
      if (!fgets (strCode, 100, fpDict))
        break;

      rule[iTemp].rule=(RULE_RULE *)malloc(sizeof(RULE_RULE)*iCodeLength);

      i = strlen (strCode) - 1;
      while (strCode[i] == ' ' || strCode[i] == '\n')
        strCode[i--] = '\0';

      pstr = strCode;
      if (*pstr == ' ')
        pstr++;
      if (pstr[0] == '#')
        continue;

      if ( strstr (pstr,"[����]") )
        break;

      switch ( *pstr ) {
          case 'e':
          case 'E':
          rule[iTemp].iFlag=0;
          break;
          case 'a':
          case 'A':
          rule[iTemp].iFlag=1;
          break;
          default:
          printf("2   Phrase rules are not suitable!\n");
          printf("\t\t%s\n",strCode);
          exit(1);
      }
      pstr++;
      p=pstr;
      while (*p && *p!='=' )
        p++;
      if ( !(*p) ) {
        printf("3   Phrase rules are not suitable!\n");
        printf("\t\t%s\n",strCode);
        exit(1);
      }
      strncpy(strTemp, pstr, p-pstr);
      strTemp[p-pstr]='\0';
      rule[iTemp].iWords = atoi(strTemp);

      p++;

      for ( i=0; i<iCodeLength;i++ ) {
        while (*p==' ')
          p++;

        switch (*p) {
            case 'p':
            case 'P':
            rule[iTemp].rule[i].iFlag = 1;
            break;
            case 'n':
            case 'N':
            rule[iTemp].rule[i].iFlag = 0;
            break;
            default:
            printf("4   Phrase rules are not suitable!\n");
            printf("\t\t%s\n",strCode);
            exit(1);
        }

        p++;

        rule[iTemp].rule[i].iWhich = *p++ - '0';
        rule[iTemp].rule[i].iIndex = *p++ - '0';

        while (*p==' ')
          p++;
        if ( i!=(iCodeLength-1) ) {
          if ( *p!='+' ) {
            printf("5   Phrase rules are not suitable!\n");
            printf("\t\t%s\n",strCode);
            exit(1);
          }

          p++;
        }
      }
    }

    if ( iTemp!= iCodeLength-1 ) {
      printf("6  Phrase rules are not suitable!\n");
      exit(1);
    }


    for (iTemp=0;iTemp<(iCodeLength-1);iTemp++) {
      if (!fgets (strCode, 100, fpDict))
        break;

      i = strlen (strCode) - 1;
      while (strCode[i] == ' ' || strCode[i] == '\n')
        strCode[i--] = '\0';

      pstr = strCode;
      if (*pstr == ' ')
        pstr++;
      if (pstr[0] == '#')
        continue;

      if ( strstr (pstr,"[����]") )
        break;
    }
  }


  if ( !strstr (pstr,"[����]") )  {
    printf("Source File Format Error!\n");
    exit (1);
  }

  for(;;) {
    if (EOF == fscanf (fpDict, "%s %s\n", strCode, strHZ))
      break;
    
    if ( strlen(strCode)>iCodeLength )
    	continue;
    if ( strlen(strHZ)>20 )		//����鳤��Ϊ10������
        continue;
      
    //�����Ƿ��ظ�
    temp=current;
    if ( temp!=head ) {
	if ( strcmp(temp->strCode,strCode)> 0 ) {
		while ( temp!=head && strcmp(temp->strCode,strCode)>=0 ) {
			if ( !strcmp(temp->strHZ,strHZ) && !strcmp(temp->strCode,strCode) ) {
				printf("Delete:  %s %s\n",strCode,strHZ);
				goto _next;
			}
			temp=temp->prev;
		}
		while ( temp!=head && strcmp(temp->strCode,strCode)<=0 )
			temp=temp->next;
	}
	else {
		while ( temp!=head && strcmp(temp->strCode,strCode)<=0 ) {
			if ( !strcmp(temp->strHZ,strHZ) && !strcmp(temp->strCode,strCode) ) {
				printf("Delete:  %s %s\n",strCode,strHZ);
				goto _next;
			}
			temp=temp->next;
		}
	}
    }
    //����temp��ǰ��
    newRec = (RECORD *) malloc (sizeof (RECORD));
    newRec->strCode=(char *) malloc(sizeof(char)*(iCodeLength+1));
    newRec->strHZ = (char *) malloc (sizeof (char) * strlen (strHZ) + 1);
    strcpy (newRec->strCode, strCode);
    strcpy (newRec->strHZ, strHZ);
    newRec->iHit=0;
    newRec->iIndex=0;

    temp->prev->next=newRec;
    newRec->next=temp;
    newRec->prev=temp->prev;
    temp->prev=newRec;
    
    current = newRec;
    
    s++;
    _next:
    	continue;
  }

  fclose (fpDict);

  printf("\nReading %d records.\n\n", s);

  fpNew=fopen(argv[2],"wb");
  if ( !fpNew ) {
    printf("\nCan not create target file!\n\n");
    exit (3);
  }

  iTemp=(unsigned int)strlen(strInputCode);
  fwrite(&iTemp,sizeof(unsigned int),1,fpNew);
  fwrite(strInputCode,sizeof(char), iTemp+1,fpNew);
  fwrite(&iCodeLength,sizeof(unsigned char),1,fpNew);
  iTemp=(unsigned int)strlen(strIgnoreChars);
  fwrite(&iTemp,sizeof(unsigned int),1,fpNew);
  fwrite(strIgnoreChars,sizeof(char),iTemp+1,fpNew);

  fwrite(&bRule, sizeof(unsigned char),1,fpNew);
  if ( bRule ) {
    for ( i=0; i<iCodeLength-1; i++ ) {
      fwrite(&(rule[i].iFlag),sizeof(unsigned char),1,fpNew);
      fwrite(&(rule[i].iWords),sizeof(unsigned char),1,fpNew);
      for ( iTemp=0; iTemp<iCodeLength;iTemp++) {
        fwrite(&(rule[i].rule[iTemp].iFlag),sizeof(unsigned char),1,fpNew);
        fwrite(&(rule[i].rule[iTemp].iWhich),sizeof(unsigned char),1,fpNew);
        fwrite(&(rule[i].rule[iTemp].iIndex),sizeof(unsigned char),1,fpNew);
      }
    }
  }

  fwrite(&s,sizeof(unsigned int),1,fpNew);
  current=head->next;
  while (current!=head) {
    fwrite(current->strCode,sizeof(char),iCodeLength + 1,fpNew);
    s=strlen(current->strHZ)+1;
    fwrite(&s,sizeof(unsigned int),1,fpNew);
    fwrite(current->strHZ,sizeof(char),s,fpNew);
    fwrite( &(current->iHit),sizeof(unsigned int),1,fpNew);
    fwrite( &(current->iIndex),sizeof(unsigned int),1,fpNew);
    current=current->next;
  }
  fclose(fpNew);

  return 0;
}
