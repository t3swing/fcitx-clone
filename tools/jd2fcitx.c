#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char strCode[10];
    char strHZ[50];
    int s;
    
    if ( argc!=2 ) {
	fprintf(stderr,"Usage: jd2fcitx <����ļ�>\n");
	return -1;
    }
    
    fp=fopen(argv[1], "rt");
    if (!fp ) {
	    fprintf(stderr,"Cannot open source file.\n");
	    return -2;
    }
    
    s=0;
    while (!feof(fp) ) {
	    fscanf(fp, "%s\n", strHZ );
	    if ( isalpha(strHZ[0]) )
		    strcpy(strCode, strHZ);
	    else {
		    printf("%s %s\n", strCode,strHZ);
		    s++;
	    }
    }
    
    fprintf(stderr,"Total: %d\n",s);
    fcloseall();
    
    return 0;
}
