#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

#include "ui.h"
#include "version.h"
#include "MainWindow.h"
#include "InputWindow.h"
#include "PYFA.h"
#include "py.h"
#include "sp.h"
#include "ime.h"

extern Display *dpy;
extern int      iScreen;
extern int      MAINWND_WIDTH;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern int      iInputWindowX;
extern int      iInputWindowY;
extern int      iInputWindowWidth;
extern int      iInputWindowHeight;

extern int      iMaxCandWord;
extern int      i3DEffect;
extern WINDOW_COLOR inputWindowColor;
extern WINDOW_COLOR mainWindowColor;
extern char     strUserLocale[];
extern MESSAGE_COLOR messageColor[];
extern MESSAGE_COLOR inputWindowLineColor;
extern MESSAGE_COLOR mainWindowLineColor;
extern MESSAGE_COLOR cursorColor;
extern ENTER_TO_DO enterToDo;

extern Bool     bSP;
extern HOTKEYS  hkGBK[];
extern HOTKEYS  hkCorner[];
extern HOTKEYS  hkPunc[];
extern HOTKEYS  hkPrevPage[];
extern HOTKEYS  hkNextPage[];
extern HOTKEYS  hkERBIPrevPage[];
extern HOTKEYS  hkERBINextPage[];
extern HOTKEYS  hkWBAddPhrase[];
extern HOTKEYS  hkWBDelPhrase[];
extern HOTKEYS  hkWBAdjustOrder[];
extern HOTKEYS  hkPYAddFreq[];
extern HOTKEYS  hkPYDelFreq[];
extern HOTKEYS  hkPYDelUserPhr[];
extern HOTKEYS  hkLegend[];
extern KEYCODE  switchKey;
extern Bool     bUseGBK;
extern Bool     bEngPuncAfterNumber;
extern Bool     bAutoHideInputWindow;
extern Bool     bTrackCursor;
extern Bool     bUseCtrlShift;
extern Bool     bWBAutoSendToClient;
extern Bool     bWBExactMatch;
extern Bool     bUseZPY;
extern Bool     bWBUseZ;
extern XColor   colorArrow;
extern HIDE_MAINWINDOW hideMainWindow;
extern Bool     bWBAutoAdjustOrder;
extern Bool     bPromptWBCode;
extern HIDE_MAINWINDOW hideMainWindow;
extern int      iFontSize;

extern Bool     bChnPunc;
extern Bool     bCorner;
extern Bool     bUseLegend;

extern Bool     bPYCreateAuto;
extern Bool     bPYSaveAutoAsPhrase;
extern Bool     bPhraseTips;
extern Bool     bEngAfterSemicolon;
extern Bool     bEngAfterCap;

extern Bool     bFullPY;
extern Bool     bDisablePagingInLegend;

extern int      i2ndSelectKey;
extern int      i3rdSelectKey;

extern char     strFontName[];

extern ADJUSTORDER baseOrder;
extern ADJUSTORDER phraseOrder;
extern ADJUSTORDER freqOrder;

extern IME      imeIndex;

extern MHPY     MHPY_C[];
extern MHPY     MHPY_S[];

#ifdef _USE_XFT
extern Bool     bUseAA;
#endif

/*
 * ��ȡ�û��������ļ�
 */
void LoadConfig (Bool bMode)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    int             r, g, b;	//���������

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/config");

    fp = fopen (strPath, "rt");

    if (!fp) {
	SaveConfig ();
	LoadConfig (True);		//����Ĭ��ֵ
	return;
    }

    strUserLocale[0] = '\0';

    for (;;) {
	if (!fgets (str, PATH_MAX, fp))
	    break;

	i = strlen (str) - 1;
	while (str[i] == ' ' || str[i] == '\n')
	    str[i--] = '\0';

	pstr = str;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (strstr (pstr, "��������=") && bMode) {
	    pstr += 9;
	    strcpy (strUserLocale, pstr);
	}
	else if (strstr (pstr, "��ʾ����=") && bMode) {
	    pstr += 9;
	    strcpy (strFontName, pstr);
	}
	else if (strstr (pstr, "��ʾ�����С=") && bMode) {
	    pstr += 13;
	    iFontSize = atoi (pstr);
	}
#ifdef _USE_XFT
	else if (strstr (pstr, "�Ƿ�ʹ��AA����=") && bMode) {
	    pstr += 15;
	    bUseAA = atoi (pstr);
	}
#endif
	else if (strstr (pstr, "�Ƿ�ʹ��Ctrl_Shift�����뷨=") && bMode) {
	    pstr += 29;
	    bUseCtrlShift = atoi (pstr);
	}
	else if (strstr (pstr, "��ѡ�ʸ���=")) {
	    pstr += 11;
	    iMaxCandWord = atoi (pstr);
	    if (iMaxCandWord > 10)
		iMaxCandWord = MAX_CAND_WORD;
	}

	else if (strstr (pstr, "���ֺ����Ƿ���=")) {
	    pstr += 17;
	    bEngPuncAfterNumber = atoi (pstr);
	}

	else if (strstr (pstr, "�Ƿ�ʹ��3D����=")) {
	    pstr += 15;
	    i3DEffect = atoi (pstr);
	}
	else if (strstr (pstr, "�Ƿ��Զ�����������=")) {
	    pstr += 19;
	    bAutoHideInputWindow = atoi (pstr);
	}
	else if (strstr (pstr, "����������ģʽ=")) {
	    pstr += 15;
	    hideMainWindow = (HIDE_MAINWINDOW) atoi (pstr);
	}
	else if (strstr (pstr, "�Ƿ������=")) {
	    pstr += 13;
	    bTrackCursor = atoi (pstr);
	}
	else if (strstr (pstr, "���ɫ=") && bMode) {
	    pstr += 7;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    cursorColor.color.red = (r << 8);
	    cursorColor.color.green = (g << 8);
	    cursorColor.color.blue = (b << 8);
	}
	else if (strstr (pstr, "�����ڱ���ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowColor.backColor.red = (r << 8);
	    mainWindowColor.backColor.green = (g << 8);
	    mainWindowColor.backColor.blue = (b << 8);
	}
	else if (strstr (pstr, "����������ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    mainWindowLineColor.color.red = (r << 8);
	    mainWindowLineColor.color.green = (g << 8);
	    mainWindowLineColor.color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰����ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowColor.backColor.red = (r << 8);
	    inputWindowColor.backColor.green = (g << 8);
	    inputWindowColor.backColor.blue = (b << 8);
	}

	else if (strstr (pstr, "���봰��ʾɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[0].color.red = (r << 8);
	    messageColor[0].color.green = (g << 8);
	    messageColor[0].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[1].color.red = (r << 8);
	    messageColor[1].color.green = (g << 8);
	    messageColor[1].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰���ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[2].color.red = (r << 8);
	    messageColor[2].color.green = (g << 8);
	    messageColor[2].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰��һ����ѡ��ɫ=") && bMode) {
	    pstr += 21;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[3].color.red = (r << 8);
	    messageColor[3].color.green = (g << 8);
	    messageColor[3].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[4].color.red = (r << 8);
	    messageColor[4].color.green = (g << 8);
	    messageColor[4].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰��ʾ����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[5].color.red = (r << 8);
	    messageColor[5].color.green = (g << 8);
	    messageColor[5].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰�����ı�ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[6].color.red = (r << 8);
	    messageColor[6].color.green = (g << 8);
	    messageColor[6].color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰����ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    inputWindowLineColor.color.red = (r << 8);
	    inputWindowLineColor.color.green = (g << 8);
	    inputWindowLineColor.color.blue = (b << 8);
	}
	else if (strstr (pstr, "���봰��ͷɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    colorArrow.red = (r << 8);
	    colorArrow.green = (g << 8);
	    colorArrow.blue = (b << 8);
	}

	else if (strstr (pstr, "��Ӣ�Ŀ����л���=")) {
	    pstr += 17;
	    SetSwitchKey (pstr);
	}
	else if (strstr (pstr, "GBK֧��=")) {
	    pstr += 8;
	    SetHotKey (pstr, hkGBK);
	}
	else if (strstr (pstr, "�ֺ�����Ӣ��=")) {
	    pstr += 13;
	    bEngAfterSemicolon = atoi (pstr);
	}
	else if (strstr (pstr, "��д��ĸ����Ӣ��=")) {
	    pstr += 17;
	    bEngAfterCap = atoi (pstr);
	}
    else if (strstr (pstr, "���뷽ʽ��ֹ��ҳ=")) {
        pstr += 17;
        bDisablePagingInLegend = atoi (pstr);
    }
	else if (strstr (pstr, "����֧��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkLegend);
	}
	else if (strstr (pstr, "Enter����Ϊ=")) {
	    pstr += 12;
	    enterToDo = (ENTER_TO_DO) atoi (pstr);
	}
	else if (strstr (pstr, "ȫ���=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkCorner);
	}
	else if (strstr (pstr, "���ı��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkPunc);
	}
	else if (strstr (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkPrevPage);
	}
	else if (strstr (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkNextPage);
	}
	else if (strstr (pstr, "�ڶ�����ѡ��ѡ���=")) {
	    pstr += 19;
	    if (!strcasecmp (pstr, "SHIFT")) {
		i2ndSelectKey = 50;	//��SHIFT��ɨ����
		i3rdSelectKey = 62;	//��SHIFT��ɨ����
	    }
	    else if (!strcasecmp (pstr, "CTRL")) {
		i2ndSelectKey = 37;	//��CTRL��ɨ����
		i3rdSelectKey = 109;	//��CTRL��ɨ����
	    }
	}

	else if (strstr (pstr, "����ļ��Զ�����=")) {
	    pstr += 17;
	    bWBAutoSendToClient = atoi (pstr);
	}
	else if (strstr (pstr, "�Զ��������˳��=")) {
	    pstr += 17;
	    bWBAutoAdjustOrder = atoi (pstr);
	}
	else if (strstr (pstr, "��ʾ�ʿ������еĴ���=")) {
	    pstr += 21;
	    bPhraseTips = atoi (pstr);
	}
	else if (strstr (pstr, "ʹ��Z����ƴ��=")) {
	    pstr += 14;
	    bUseZPY = atoi (pstr);
	}
	else if (strstr (pstr, "Zģ��ƥ��=")) {
	    pstr += 10;
	    bWBUseZ = atoi (pstr);
	}
	else if (strstr (pstr, "��ʾ�ȷƥ��=")) {
	    pstr += 13;
	    bWBExactMatch = atoi (pstr);
	}
	else if (strstr (pstr, "��ʾ��ʱ���=")) {
	    pstr += 13;
	    bPromptWBCode = atoi (pstr);
	}
	else if (strstr (pstr, "������ʴ���=")) {
	    pstr += 13;
	    SetHotKey (pstr, hkWBAddPhrase);
	}
	else if (strstr (pstr, "�������˳��=")) {
	    pstr += 13;
	    SetHotKey (pstr, hkWBAdjustOrder);
	}
	else if (strstr (pstr, "ɾ����ʴ���=")) {
	    pstr += 13;
	    SetHotKey (pstr, hkWBDelPhrase);
	}
	
	else if (strstr (pstr, "��ǰ��ҳ=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkERBIPrevPage);
	}
	else if (strstr (pstr, "���ҳ=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkERBINextPage);
	}
	
	else if (strstr (str, "ʹ��ȫƴ=")) {
	    pstr += 9;
	    bFullPY = atoi (pstr);
	}
	else if (strstr (str, "ƴ���Զ����=")) {
	    pstr += 13;
	    bPYCreateAuto = atoi (pstr);
	}
    else if (strstr (str, "�����Զ����=")) {
        pstr += 13;
        bPYSaveAutoAsPhrase = atoi (pstr);
    }
	else if (strstr (str, "����ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYAddFreq);
	}
	else if (strstr (str, "ɾ��ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYDelFreq);
	}
	else if (strstr (str, "ɾ��ƴ���û�����=")) {
	    pstr += 17;
	    SetHotKey (pstr, hkPYDelUserPhr);
	}
	else if (strstr (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    baseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (strstr (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    phraseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (strstr (str, "ƴ�����ô����������ʽ=")) {
	    pstr += 23;
	    freqOrder = (ADJUSTORDER) atoi (pstr);
	}

	else if (strstr (str, "�Ƿ�ģ��an��ang=")) {
	    pstr += 16;
	    MHPY_C[0].bMode = atoi (pstr);
	    MHPY_S[5].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��en��eng=")) {
	    pstr += 16;
	    MHPY_C[1].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��ian��iang=")) {
	    pstr += 18;
	    MHPY_C[2].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��in��ing=")) {
	    pstr += 16;
	    MHPY_C[3].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��ou��u=")) {
	    pstr += 14;
	    MHPY_C[4].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��uan��uang=")) {
	    pstr += 18;
	    MHPY_C[5].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��c��ch=")) {
	    pstr += 14;
	    MHPY_S[0].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��f��h=")) {
	    pstr += 13;
	    MHPY_S[1].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��l��n=")) {
	    pstr += 13;
	    MHPY_S[2].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��s��sh=")) {
	    pstr += 14;
	    MHPY_S[3].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��z��zh=")) {
	    pstr += 14;
	    MHPY_S[4].bMode = atoi (pstr);
	}
    }

    fclose (fp);
}

void SaveConfig (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "config");
    fp = fopen (strPath, "wt");
    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� config��\n");
	return;
    }

    fprintf (fp, "[����]\n");
    fprintf (fp, "#��������=zh_CN\n");
    fprintf (fp, "��ʾ����=*\n");
    fprintf (fp, "��ʾ�����С=18\n");
#ifdef _USE_XFT
    fprintf (fp, "�Ƿ�ʹ��AA����=0\n");
#endif
    fprintf (fp, "�Ƿ�ʹ��Ctrl_Shift�����뷨=1\n");

    fprintf (fp, "\n[���]\n");
    fprintf (fp, "���ֺ����Ƿ���=1\n");
    fprintf (fp, "Enter����Ϊ=2\n");
    fprintf (fp, "�ֺ�����Ӣ��=1\n");
    fprintf (fp, "��д��ĸ����Ӣ��=1\n");
    fprintf (fp, "���뷽ʽ��ֹ��ҳ=1\n");

    fprintf (fp, "\n[����]\n");
    fprintf (fp, "��ѡ�ʸ���=5\n");
    fprintf (fp, "�Ƿ�ʹ��3D����=2\n");
    fprintf (fp, "�Ƿ��Զ�����������=1\n");
    fprintf (fp, "����������ģʽ=1\n");

    fprintf (fp, "�Ƿ������=1\n");

    fprintf (fp, "���ɫ=92 210 131\n");
    fprintf (fp, "�����ڱ���ɫ=230 230 230\n");
    fprintf (fp, "����������ɫ=255 0 0\n");
    fprintf (fp, "���봰����ɫ=240 240 240\n");
    fprintf (fp, "���봰����ɫ=100 200 255\n");
    fprintf (fp, "���봰��ͷɫ=255 150 255\n");

    fprintf (fp, "���봰�û�����ɫ=0 0 255\n");
    fprintf (fp, "���봰��ʾɫ=255 0 0\n");
    fprintf (fp, "���봰���ɫ=200 0 0\n");
    fprintf (fp, "���봰��һ����ѡ��ɫ=0 150 100\n");
    fprintf (fp, "#����ɫֵֻ����ƴ���е��û������\n");
    fprintf (fp, "���봰�û�����ɫ=0 0 255\n");
    fprintf (fp, "���봰��ʾ����ɫ=100 100 255\n");
    fprintf (fp, "#��ʡ�ƴ���ĵ���/ϵͳ�����ʹ�ø���ɫ\n");
    fprintf (fp, "���봰�����ı�ɫ=0 0 0\n");

    fprintf (fp, "\n[�ȼ�]\n");
    fprintf (fp, "#��Ӣ�Ŀ����л��� ��������ΪL_CTRL R_CTRL L_SHIFT R_SHIFT\n");
    fprintf (fp, "��Ӣ�Ŀ����л���=L_CTRL\n");
    fprintf (fp, "GBK֧��=CTRL_M\n");
    fprintf (fp, "����֧��=CTRL_L\n");
    fprintf (fp, "ȫ���=SHIFT_SPACE\n");
    fprintf (fp, "���ı��=ALT_SPACE\n");
    fprintf (fp, "��һҳ=- ,\n");
    fprintf (fp, "��һҳ== .\n");
    fprintf (fp, "�ڶ�����ѡ��ѡ���=SHIFT\n");

    fprintf (fp, "\n[���]\n");
    fprintf (fp, "����ļ��Զ�����=1\n");
    fprintf (fp, "�Զ��������˳��=0\n");
    fprintf (fp, "��ʾ�ʿ������еĴ���=0\n");
    fprintf (fp, "��ʾ�ȷƥ��=0\n");
    fprintf (fp, "��ʾ��ʱ���=1\n");
    fprintf (fp, "ʹ��Z����ƴ��=%d\n", bUseZPY);
    fprintf (fp, "Zģ��ƥ��=%d\n", bWBUseZ);
    fprintf (fp, "������ʴ���=CTRL_8\n");
    fprintf (fp, "�������˳��=CTRL_6\n");
    fprintf (fp, "ɾ����ʴ���=CTRL_7\n");

	fprintf (fp, "\n[����]\n");
    fprintf (fp, "��ǰ��ҳ=[\n");
    fprintf (fp, "���ҳ=]\n");

    fprintf (fp, "\n[ƴ��]\n");
    fprintf (fp, "ʹ��ȫƴ=0\n");
    fprintf (fp, "ƴ���Զ����=%d\n", bPYCreateAuto);
    fprintf (fp, "�����Զ����=%d\n", bPYSaveAutoAsPhrase);
    fprintf (fp, "����ƴ��������=CTRL_8\n");
    fprintf (fp, "ɾ��ƴ��������=CTRL_7\n");
    fprintf (fp, "ɾ��ƴ���û�����=CTRL_DELETE\n");
    fprintf (fp, "#���������ʽ˵����0-->������  1-->���ٵ���  2-->��Ƶ�ʵ���\n");
    fprintf (fp, "ƴ���������������ʽ=%d\n", baseOrder);
    fprintf (fp, "ƴ���������������ʽ=%d\n", phraseOrder);
    fprintf (fp, "ƴ�����ô����������ʽ=%d\n", freqOrder);
    fprintf (fp, "�Ƿ�ģ��an��ang=%d\n", MHPY_C[0].bMode);
    fprintf (fp, "�Ƿ�ģ��en��eng=%d\n", MHPY_C[1].bMode);
    fprintf (fp, "�Ƿ�ģ��ian��iang=%d\n", MHPY_C[2].bMode);
    fprintf (fp, "�Ƿ�ģ��in��ing=%d\n", MHPY_C[3].bMode);
    fprintf (fp, "�Ƿ�ģ��ou��u=%d\n", MHPY_C[4].bMode);
    fprintf (fp, "�Ƿ�ģ��uan��uang=%d\n", MHPY_C[5].bMode);
    fprintf (fp, "�Ƿ�ģ��c��ch=%d\n", MHPY_S[0].bMode);
    fprintf (fp, "�Ƿ�ģ��f��h=%d\n", MHPY_S[1].bMode);
    fprintf (fp, "�Ƿ�ģ��l��n=%d\n", MHPY_S[2].bMode);
    fprintf (fp, "�Ƿ�ģ��s��sh=%d\n", MHPY_S[3].bMode);
    fprintf (fp, "�Ƿ�ģ��z��zh=%d\n", MHPY_S[4].bMode);

    fclose (fp);
}

void LoadProfile (void)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    Bool            bRetVal;

    iMainWindowX = MAINWND_STARTX;
    iMainWindowY = MAINWND_STARTY;
    iInputWindowX = INPUTWND_STARTX;
    iInputWindowY = INPUTWND_STARTY;

    bRetVal = False;
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/profile");

    fp = fopen (strPath, "rt");

    if (fp) {
	for (;;) {
	    if (!fgets (str, PATH_MAX, fp))
		break;

	    i = strlen (str) - 1;
	    while (str[i] == ' ' || str[i] == '\n')
		str[i--] = '\0';

	    pstr = str;

	    if (strstr (str, "�汾=")) {
		pstr += 5;

		if (!strcasecmp (FCITX_VERSION, pstr))
		    bRetVal = True;
	    }
	    else if (strstr (str, "������λ��X=")) {
		pstr += 12;
		iMainWindowX = atoi (pstr);
		if (iMainWindowX < 0)
		    iMainWindowX = 0;
		else if ((iMainWindowX + MAINWND_WIDTH) > DisplayWidth (dpy, iScreen))
		    iMainWindowX = DisplayWidth (dpy, iScreen) - MAINWND_WIDTH;
	    }
	    else if (strstr (str, "������λ��Y=")) {
		pstr += 12;
		iMainWindowY = atoi (pstr);
		if (iMainWindowY < 0)
		    iMainWindowY = 0;
		else if ((iMainWindowY + MAINWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iMainWindowY = DisplayHeight (dpy, iScreen) - MAINWND_HEIGHT;
	    }
	    else if (strstr (str, "���봰��λ��X=")) {
		pstr += 14;
		iInputWindowX = atoi (pstr);
		if (iInputWindowX < 0)
		    iInputWindowX = 0;
		else if ((iInputWindowX + iInputWindowWidth) > DisplayWidth (dpy, iScreen))
		    iInputWindowX = DisplayWidth (dpy, iScreen) - iInputWindowWidth;
	    }
	    else if (strstr (str, "���봰��λ��Y=")) {
		pstr += 14;
		iInputWindowY = atoi (pstr);
		if (iInputWindowY < 0)
		    iInputWindowY = 0;
		else if ((iInputWindowY + INPUTWND_HEIGHT) > DisplayHeight (dpy, iScreen))
		    iInputWindowY = DisplayHeight (dpy, iScreen) - INPUTWND_HEIGHT;
	    }
	    else if (strstr (str, "�Ƿ�ȫ��=")) {
		pstr += 9;
		bCorner = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ����ı��=")) {
		pstr += 13;
		bChnPunc = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ�GBK=")) {
		pstr += 8;
		bUseGBK = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ�����=")) {
		pstr += 9;
		bUseLegend = atoi (pstr);
	    }
	    else if (strstr (str, "��ǰ���뷨=")) {
		pstr += 11;
		imeIndex = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ�ʹ��˫ƴ=")) {
		pstr += 13;
		bSP = atoi (pstr);
		if (bSP)
		    LoadSPData ();
	    }
	}

	fclose (fp);
    }

    if (!bRetVal) {
	SaveConfig ();
	SaveProfile ();
    }
}

void SaveProfile (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "profile");
    fp = fopen (strPath, "wt");

    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� profile!\n");
	return;
    }

    fprintf (fp, "�汾=%s\n", FCITX_VERSION);
    fprintf (fp, "������λ��X=%d\n", iMainWindowX);
    fprintf (fp, "������λ��Y=%d\n", iMainWindowY);
    fprintf (fp, "���봰��λ��X=%d\n", iInputWindowX);
    fprintf (fp, "���봰��λ��Y=%d\n", iInputWindowY);
    fprintf (fp, "�Ƿ�ȫ��=%d\n", (bCorner) ? 1 : 0);
    fprintf (fp, "�Ƿ����ı��=%d\n", (bChnPunc) ? 1 : 0);
    fprintf (fp, "�Ƿ�GBK=%d\n", (bUseGBK) ? 1 : 0);
    fprintf (fp, "�Ƿ�����=%d\n", (bUseLegend) ? 1 : 0);
    fprintf (fp, "��ǰ���뷨=%d\n", imeIndex);
    fprintf (fp, "�Ƿ�ʹ��˫ƴ=%d\n", bSP);

    fclose (fp);
}

void SetHotKey (char *strKeys, HOTKEYS * hotkey)
{
    char           *p;
    char            strKey[30];
    int             i, j;

    p = strKeys;

    while (*p == ' ')
	p++;
    i = 0;
    while (p[i] != ' ' && p[i] != '\0')
	i++;
    strncpy (strKey, p, i);
    strKey[i] = '\0';
    p += i + 1l;
    j = ParseKey (strKey);
    if (j != -1)
	hotkey[0] = j;
    if (j == -1)
	j = 0;
    else
	j = 1;

    i = 0;
    while (p[i] != ' ' && p[i] != '\0')
	i++;
    if (p[0]) {
	strncpy (strKey, p, i);
	strKey[i] = '\0';

	i = ParseKey (strKey);
	if (i == -1)
	    i = 0;
    }
    else
	i = 0;

    hotkey[j] = i;
}

/*
 * �����ļ����ж�����
 * ע��:�ļ��еĿ���Ҳ��Ϊһ�д���
 */
int CalculateRecordNumber (FILE * fpDict)
{
    char            strText[101];
    int             nNumber = 0;

    for (;;) {
	if (!fgets (strText, 100, fpDict))
	    break;

	nNumber++;
    }
    rewind (fpDict);

    return nNumber;
}

void SetSwitchKey (char *str)
{
    if (!strcasecmp (str, "R_CTRL"))
	switchKey = R_CTRL;
    else if (!strcasecmp (str, "R_SHIFT"))
	switchKey = R_SHIFT;
    else if (!strcasecmp (str, "L_SHIFT"))
	switchKey = L_SHIFT;
    else
	switchKey = L_CTRL;
}
