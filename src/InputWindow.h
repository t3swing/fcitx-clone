#ifndef _INPUT_WINDOW_H
#define _INPUT_WINDOW_H

#include <X11/Xlib.h>

#define INPUTWND_STARTX	300
#define INPUTWND_WIDTH	100
#define INPUTWND_STARTY	420
#define INPUTWND_HEIGHT	40

#define INPUTWND_START_POS_UP	5
#define INPUTWND_START_POS_DOWN	8

#define MESSAGE_MAX_LENGTH	300	//����������ʾ������ȣ����ַ���

/* ������������ʾ�����ݷ�Ϊ���¼��� */
#define MESSAGE_TYPE_COUNT	7

typedef enum {
    MSG_TIPS,			//��ʾ�ı�
    MSG_INPUT,			//�û�������
    MSG_INDEX,			//��ѡ��ǰ������
    MSG_FIRSTCAND,		//��һ����ѡ��
    MSG_USERPHR,		//�û�����
    MSG_CODE,			//��ʾ�ı���
    MSG_OTHER			//�����ı�
} MSG_TYPE;

typedef struct {
    char            strMsg[MESSAGE_MAX_LENGTH + 1];
    MSG_TYPE        type;
} MESSAGE;

Bool            CreateInputWindow (void);
void            DisplayInputWindow (void);
void            InitInputWindowColor (void);
void            CalculateInputWindowHeight (void);
void            DrawCursor (int iPos);
void            DisplayMessageUp (void);
void            DisplayMessageDown (void);
void            DisplayMessage (void);
void            DrawInputWindow (void);
void            ResetInputWindow (void);

#endif
