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
/**
 * @file   skin.c
 * @author Yuking yuking_net@sohu.com  t3swing  t3swing@sina.com
 *        
 * @date   2009-10-9
 * 
 * @brief  皮肤设置相关定义及初始化加载工作
 * 
 * 
 */
#include "skin.h"


//定义全局皮肤配置结构
_Skin_Config skin_config;

//指定皮肤类型 在config文件中配置
char  skinType[64];
//指定皮肤所在的文件夹 一般在/usr/share/fcitx/xpm目录下面
_Skin_Dir skinBuf[10];
int skinCount;

Pixmap  Bar;
Pixmap  Logo;
Pixmap  Punc[2];
Pixmap  GBK[2];
Pixmap  Corner[2];	
Pixmap  LX[2];	
Pixmap  GBKT[2];	
Pixmap  Lock[2];	
Pixmap  VK;
Pixmap  Input;
Pixmap  Prev;
Pixmap  Next;
Pixmap  English;
Pixmap  Pinyin;
Pixmap  Shuangpin;
Pixmap  Wubi;
Pixmap  Mix;
Pixmap  Erbi;
Pixmap  CangJi;
Pixmap  Wanfeng;
Pixmap  Bingcan;
Pixmap  Ziran;
Pixmap  Quwei;
Pixmap  Otherim;

Pixmap 	Barmask;
Pixmap  Logomask;
Pixmap  Puncmask[2];
Pixmap  GBKmask[2];
Pixmap  Cornermask[2];	
Pixmap  LXmask[2];	
Pixmap  GBKTmask[2];	
Pixmap  Lockmask[2];	
Pixmap  VKmask;
Pixmap  Inputmask;
Pixmap  Prevmask;
Pixmap  Nextmask;
Pixmap  Englishmask;
Pixmap 	Pinyinmask;
Pixmap  Shuangpinmask;
Pixmap  Wubimask;
Pixmap  Mixmask;
Pixmap  Erbimask;
Pixmap  CangJimask;
Pixmap  Wanfengmask;
Pixmap  Bingcanmask;
Pixmap  Ziranmask;
Pixmap  Quweimask;
Pixmap  Otherimmask;

Pixmap  Inputshapemask;
Pixmap  inputtmpmask,shapemask;
Pixmap  inputbar;


static GC maskgc,inputbargc;
static unsigned int bitw,bith,hotx,hoty;

#define MAXROW 96
#define MAXLINE 128
static int line;
static int Number1;
char ReadBuf[MAXLINE][MAXROW];
/*
* 
*/
int str2skin_xpm(char * str,skin_xpm * xpm)
{
	
	if(sscanf(str, "%s %c %d %d %d %d %d %d %d %d", xpm->xpmname,&xpm->enable,&xpm->position_x,\
		&xpm->position_y,&xpm->width,&xpm->height,&xpm->response_x,&xpm->response_y,\
		&xpm->response_w,&xpm->response_h) <= 0 )
		return -1;
	
	return 0;
}

void ltrim( char *str)
{
    char* s=str;
    while ( *s )
        if ( isspace(*s) )
            ++s;
        else
            break;
    if ( s>str )
        strcpy( str, s );
}


void rtrim(char *str )
{
    char* s=str;
    while ( *s )
        ++s;
    --s;
    while ( s>=str )
        if ( isspace(*s) )
            --s;
        else
            break;
    *(s+1)=0;
}

void trim(char *str)
{
	ltrim(str) ;
	rtrim(str) ;
}

void WriteBefore(FILE *fp)
{
    int i;

    for (i=0;i<Number1;i++)
    {
	fprintf(fp,"%s",ReadBuf[i]);
    }
}
void WriteAfter(FILE *fp)
{
    int i;
    for (i=Number1+1;i<line;i++)
       fprintf(fp,"%s",ReadBuf[i]);
}

int FindKeyName(int Number,char *KeyName)
{
    int i,len;
    char *str;

    len = strlen( KeyName );
    for (i=Number;i<line;i++)
    {
        str = strstr( ReadBuf[i],KeyName );
	if (ReadBuf[i][0]=='['){
	   if(ReadBuf[i-1][0]==' '){
	       Number1=i;
	       return 2;
	   }
	   Number1 = i-1 ;
	   return 2;
	}
	else if (str!=NULL)
            if ((str[len]==' ')||(str[len]=='='))
            {
                Number1=i;
                return 0;
            }
    }
    Number1 = i+1;
    return 1;
}

int GetPrivateProfileString(FILE * fptmp,char *SelectName, char *KeyName,char *DefaultString, char *ReturnString,int Size)
{
	
    char chBuf1[256],chBuf[256],chMsg[256],ch;
    char *pchBegin;
	FILE * fp=fptmp;
    pchBegin = NULL;
    sprintf(chBuf,"[%s]",SelectName);
    fseek(fp,0L,SEEK_SET);

    while (1) {
			if( feof(fp) ) {
				strcpy(ReturnString,DefaultString);
			//	fclose(fp) ;
				return -1;
			}
			
			memset(chBuf1,'\0',256);
			fgets(chBuf1,sizeof(chBuf1),fp);
       
      // 过滤注释
      if (chBuf1[0] == '#') continue;
       
		
      // 段名
		pchBegin = strstr( chBuf1, chBuf );
      if( pchBegin == NULL )
				continue;
			else 
				break;
		}

    pchBegin = NULL;
    
    while (1) {
			if( feof(fp) ) {
				strcpy(ReturnString,DefaultString);
			//	fclose(fp) ;
				return -2;
			}
	
			memset(chBuf1,'\0',256);
			fgets(chBuf1,sizeof(chBuf1),fp);
        
      // 过滤注释
      if (chBuf1[0] == '#') continue;
      	trim(chBuf1);
      // 关键字名 
			pchBegin = strstr( chBuf1, KeyName );

			if( pchBegin == NULL )
				continue;
		if( memcmp(KeyName,chBuf1,strlen(KeyName)) != 0)
				continue;
			ch = pchBegin[strlen(KeyName)];
			if ( ch!=' ' && ch!='=' )
	    	continue;
			
			pchBegin = NULL;
			pchBegin = strstr( chBuf1, "=" );

			if( pchBegin == NULL )
				continue;
			else
				break;
		}
    
    
    strcpy( chMsg, pchBegin+1 ); 
    trim(chMsg) ;
    memcpy(ReturnString,chMsg,Size);
    ReturnString[Size] = '\0';

    return 0;
}

int WritePrivateProfileString(char *SelectName, char *KeyName, char *String, char *FileName)
{
    FILE *fp=NULL;
    char chBuf[256],*str;
    int i,mode=0;

    if((fp=fopen(FileName,"r"))==NULL)
    {
        fp = fopen( FileName,"w");
        fprintf( fp,"[%s]\n",SelectName );
        fprintf( fp,"%s=%s\n",KeyName,String );
        fclose(fp);
		return 0;
    }
    for (i=0;i<MAXLINE;i++)
    {
		fgets(ReadBuf[i],MAXROW,fp);
		if (feof(fp))
		{
		    line = i;
		    break;
		}
    }
    fclose(fp);
	fp=NULL;
    if( (fp = fopen(FileName,"w+")) == NULL)
	{
		perror("WritePrivateProfileString fopen error");
	}
	
    memset( chBuf,'\0',256 );
    sprintf( chBuf,"[%s]",SelectName );
    for (i=0;i<line;i++)
    {
        str = strstr( ReadBuf[i],chBuf );
        if ( str!=NULL )
        {
            mode = FindKeyName(i+1,KeyName);
            if( mode==0 )
            {
                WriteBefore(fp);
                fprintf(fp,"%s=%s\n",KeyName,String);
                WriteAfter(fp);
                fclose(fp);
                return 0;
            }
            else if(mode == 2)
            {
                WriteBefore(fp);
                fprintf(fp,"%s=%s\n",KeyName,String);
                WriteAfter(fp);
                fclose(fp);
                return 0;
            }
            else{
                WriteBefore(fp);
                fprintf(fp,"%s=%s\n",KeyName,String);
                fclose(fp);
                return 0;
            }
        }
    }
    for(i=0;i<line;i++)
        fprintf(fp,"%s",ReadBuf[i]);
    fprintf(fp,"\n[%s]\n",SelectName);
    fprintf(fp,"%s=%s\n",KeyName,String);
    fclose(fp);
    return 0;
}

/*
*
*把字符串中配色加载到xcolor中
*/
int convertstr2color(XColor * color,char * str)
{
	int r,g,b;
	if(sscanf( str, "%d %d %d",&r,&g,&b) !=3)
	{
		//printf("color format error!\n");
		return -1;
	}
	color->red=r<<8;
	color->green=g<<8;
	color->blue=b<<8;
	
	return 0;   
}

int fill_skin_config_xpm(FILE * fptmp,char * SelectName ,char * KeyName, skin_xpm * xpm)
{	
	FILE * fp=fptmp;
	char    valuebuf[128]={0};
	GetPrivateProfileString(fp,SelectName,KeyName,"",(char*)valuebuf,sizeof(valuebuf));
	if(strlen(valuebuf) != 0 )
		str2skin_xpm(valuebuf,xpm);
	
	//测试解析是否正常
	//printf("%s: %s %c %d %d %d %d %d %d %d %d\n",KeyName, xpm->xpmname,xpm->enable,xpm->position_x,
	//	xpm->position_y,xpm->width,xpm->height,xpm->response_x,xpm->response_y,
	//	xpm->response_w,xpm->response_h);
	if( xpm->enable == 0)
		memset(xpm,0,sizeof(skin_xpm));
	
	return 0;
}

int fill_skin_config_str(FILE * fptmp,char * SelectName,char * KeyName,char * str)
{	
	FILE * fp=fptmp;
	char    valuebuf[128]={0};
	int ret;
	ret=GetPrivateProfileString(fp,SelectName,KeyName,"",(char*)valuebuf,sizeof(valuebuf));
	if(strlen(valuebuf) != 0)
		strcpy(str,valuebuf);
	//printf("%d %s:%s\n",ret,KeyName,str);
	return ret;
}

int fill_skin_config_int(FILE * fptmp,char * SelectName,char * KeyName)
{	
	FILE * fp=fptmp;
	char    valuebuf[128]={0};
	GetPrivateProfileString(fp,SelectName,KeyName,"",(char*)valuebuf,sizeof(valuebuf));
	if(strlen(valuebuf) != 0)
	{	
		//printf("%s:%d\n",KeyName,atoi(valuebuf));
		return atoi(valuebuf);
	}
	
	return 0;
}

int fill_skin_config_color(FILE * fptmp,char * SelectName,char * KeyName,XColor * color)
{
	char str[64]={0};
	int ret;
	ret=fill_skin_config_str(fptmp,SelectName,KeyName,str);
	convertstr2color(color,str);
	return ret;
}

/**
@加载皮肤配置文件
*/
int load_skin_config()
{	
	FILE    *fp;
    char    buf[PATH_MAX]={0};
	

	memset(&skin_config,0,sizeof(_Skin_Config));
  
  	if(ISDEFAULT)
  		return 0;
    //获取配置文件的绝对路径
	sprintf(buf, "%s/xpm/%s/fcitx_skin.conf", PKGDATADIR, skinType);
//	printf("path:%s\n",buf);
	fp = fopen(buf, "r");
	
	if(!fp){
		perror("fopen");
		exit(1);	// 如果安装目录里面也没有配置文件，那就只好告诉用户，无法运行了
	}

	fill_skin_config_str(fp,"SkinInfo","skinname",skin_config.SkinInfo.skinname);
	fill_skin_config_str(fp,"SkinInfo","skinversion",skin_config.SkinInfo.skinversion);
	fill_skin_config_str(fp,"SkinInfo","skinauthor",skin_config.SkinInfo.skinauthor);
	fill_skin_config_str(fp,"SkinInfo","shinremark",skin_config.SkinInfo.shinremark);
	
	skin_config.SkinFontOption.fontsize = fill_skin_config_int(fp,"SkinFontOption","fontsize");
	fill_skin_config_str(fp,"SkinFontOption","font",skin_config.SkinFontOption.font);
	fill_skin_config_str(fp,"SkinFontOption","inputcharcolor",skin_config.SkinFontOption.inputcharcolor);
	fill_skin_config_str(fp,"SkinFontOption","outputcharcolor",skin_config.SkinFontOption.outputcharcolor);
	fill_skin_config_str(fp,"SkinFontOption","charnocolor",skin_config.SkinFontOption.charnocolor);
	fill_skin_config_str(fp,"SkinFontOption","firstcharcolor",skin_config.SkinFontOption.firstcharcolor);
	fill_skin_config_str(fp,"SkinFontOption","othercolor",skin_config.SkinFontOption.othercolor);
		
	fill_skin_config_xpm(fp,"SkinMainBar","mbbg_xpm",&skin_config.SkinMainBar.mbbg_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","mbmask_xbm",&skin_config.SkinMainBar.mbmask_xbm);
	fill_skin_config_xpm(fp,"SkinMainBar","logo_xpm",&skin_config.SkinMainBar.logo_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","zhpunc_xpm",&skin_config.SkinMainBar.zhpunc_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","enpunc_xpm",&skin_config.SkinMainBar.enpunc_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","chs_xpm",&skin_config.SkinMainBar.chs_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","cht_xpm",&skin_config.SkinMainBar.cht_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","halfcorner_xpm",&skin_config.SkinMainBar.halfcorner_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","fullcorner_xpm",&skin_config.SkinMainBar.fullcorner_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","unlock_xpm",&skin_config.SkinMainBar.unlock_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","lock_xpm",  &skin_config.SkinMainBar.lock_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","gbkoff_xpm",&skin_config.SkinMainBar.gbkoff_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","gbkon_xpm",&skin_config.SkinMainBar.gbkon_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","lxoff_xpm",&skin_config.SkinMainBar.lxoff_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","lxon_xpm",&skin_config.SkinMainBar.lxon_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","vkhide_xpm",&skin_config.SkinMainBar.vkhide_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","vkshow_xpm",&skin_config.SkinMainBar.vkshow_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","english_xpm",&skin_config.SkinMainBar.english_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","pinyin_xpm",&skin_config.SkinMainBar.pinyin_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","shuangpin_xpm",&skin_config.SkinMainBar.shuangpin_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","quwei_xpm",&skin_config.SkinMainBar.quwei_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","wubi_xpm",&skin_config.SkinMainBar.wubi_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","mixpywb_xpm",&skin_config.SkinMainBar.mixpywb_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","erbi_xpm",&skin_config.SkinMainBar.erbi_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","cj_xpm",&skin_config.SkinMainBar.cj_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","wanfeng_xpm",&skin_config.SkinMainBar.wanfeng_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","bingcan_xpm",&skin_config.SkinMainBar.bingcan_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","ziran_xpm",&skin_config.SkinMainBar.ziran_xpm);
	fill_skin_config_xpm(fp,"SkinMainBar","otherim_xpm",&skin_config.SkinMainBar.otherim_xpm);

	fill_skin_config_xpm(fp,"SkinInputBar","ibbg_xpm",&skin_config.SkinInputBar.ibbg_xpm);
	fill_skin_config_xpm(fp,"SkinInputBar","ibmask_xbm",&skin_config.SkinInputBar.ibmask_xbm);
	skin_config.SkinInputBar.resize          =fill_skin_config_int(fp,"SkinInputBar","resize");
	skin_config.SkinInputBar.resizeposition_x=fill_skin_config_int(fp,"SkinInputBar","resizeposition_x");
	skin_config.SkinInputBar.resizeposition_w=fill_skin_config_int(fp,"SkinInputBar","resizeposition_w");
	skin_config.SkinInputBar.inputposition   =fill_skin_config_int(fp,"SkinInputBar","inputposition");
	skin_config.SkinInputBar.outputposition  =fill_skin_config_int(fp,"SkinInputBar","outputposition");
	skin_config.SkinInputBar.layoutleft   =fill_skin_config_int(fp,"SkinInputBar","layoutleft");
	skin_config.SkinInputBar.layoutright  =fill_skin_config_int(fp,"SkinInputBar","layoutright");
	fill_skin_config_xpm(fp,"SkinInputBar","backarrow_xpm",&skin_config.SkinInputBar.backarrow_xpm);
	fill_skin_config_xpm(fp,"SkinInputBar","forwardarrow_xpm",&skin_config.SkinInputBar.forwardarrow_xpm);

	fill_skin_config_xpm(fp,"SkinTrayIcon","active_xpm",&skin_config.SkinTrayIcon.active_xpm);
	fill_skin_config_xpm(fp,"SkinTrayIcon","inactive_xpm",&skin_config.SkinTrayIcon.inactive_xpm);
/**/
	fclose(fp);
	
	/*皮肤配置文件的字体和字体颜色信息也在此加载 */
	
	if ( strlen(skin_config.SkinFontOption.inputcharcolor) != 0)
	{
		convertstr2color( &messageColor[1].color,skin_config.SkinFontOption.inputcharcolor );
		convertstr2color( &messageColor[5].color,skin_config.SkinFontOption.inputcharcolor );
	}
	if( strlen(skin_config.SkinFontOption.outputcharcolor) != 0)
		convertstr2color( &messageColor[0].color,skin_config.SkinFontOption.outputcharcolor);
	if( strlen(skin_config.SkinFontOption.charnocolor) != 0)
		convertstr2color( &messageColor[2].color,skin_config.SkinFontOption.charnocolor);
	if( strlen(skin_config.SkinFontOption.firstcharcolor) !=0)
		convertstr2color(&messageColor[3].color,skin_config.SkinFontOption.firstcharcolor);
	if( strlen(skin_config.SkinFontOption.othercolor) !=0)
		convertstr2color(&messageColor[6].color,skin_config.SkinFontOption.othercolor);

	return 0;

}

/*
*加载皮肤配置文件测试
*/
void configprint()
{

}
//皮肤配置文件加载函数完成
/*-------------------------------------------------------------------------------------------------------------*/
//xpm图片文件加载函数开始

/*
* 把生成的pixmap图片插入到窗口中 x y图片需要显示的起始点(全部显示填 0 0)
* x1,y1相对主窗口的位置  width height显示的长宽
*/
void putXpmImage(Window w,GC gc,Pixmap image,Pixmap  image_mask,int x, int y, int x1, int y1, int width, int height)
{
	XGCValues xgcv;
	Pixmap img;
	memset(&img,0,sizeof(Pixmap));

	if( memcmp(&img,&image,sizeof(Pixmap)) == 0)
		return ;
	
	XSetFillStyle(dpy, gc, FillTiled);
	XSetTile(dpy, gc, image);
	XSetClipMask(dpy, gc, image_mask);
	
	xgcv.ts_x_origin = x1-x;
	xgcv.ts_y_origin = y1-y;
	xgcv.clip_x_origin = x1-x;
	xgcv.clip_y_origin = y1-y;

	XChangeGC (dpy,gc,GCClipXOrigin | GCClipYOrigin | GCTileStipXOrigin | GCTileStipYOrigin, &xgcv);
	XFillRectangle(dpy, w, gc,x+x1,y+y1, width, height);
}

void putMainBarImage(GC gc,Pixmap image,Pixmap  image_mask,skin_xpm xpm)
{
	putXpmImage(mainWindow,gc,image,image_mask,0, 0,xpm.position_x, xpm.position_y,xpm.width,xpm.height);
}

void loadXpm2Pixmap(Window w,skin_xpm * xpm,Pixmap * image,Pixmap * image_mask)
{
	Colormap cmap;
	XpmAttributes   attrib;
	char xpmpath[128]={0};
	
	//如果没有图的话，不用加载 
	if( strcmp(xpm->xpmname,"NONE.xpm") == 0 || strlen(xpm->xpmname) == 0 )
	{
		return ;
	}
	sprintf(xpmpath, "%s/xpm/%s/%s", PKGDATADIR, skinType,xpm->xpmname);

	cmap = DefaultColormap(dpy, XDefaultScreen(dpy) );
	XSetWindowColormap(dpy, w, cmap);
	attrib.valuemask = XpmColormap | XpmCloseness;
	attrib.colormap  = cmap;
	attrib.closeness = 65535;

	XpmReadFileToPixmap(dpy,w,xpmpath,image,image_mask,&attrib);
	
		if(xpm->width == 0)
		xpm->width=attrib.width;
	if(xpm->height ==0)
		xpm->height=attrib.height;
		
	if(xpm->response_x == 0)
		xpm->response_x=xpm->position_x;
	if(xpm->response_y == 0)
		xpm->response_y =xpm->position_y;
	if(xpm->response_w == 0)
		xpm->response_w=xpm->width;
	if(xpm->response_h == 0)
		xpm->response_h =xpm->height;
	
	//printf("%s:%c  %d %d %d %d  %d %d %d %d\n",xpm->xpmname,xpm->enable,xpm->position_x,xpm->position_y,\
			xpm->width,xpm->height,xpm->response_x,xpm->response_y,xpm->response_w,xpm->response_h);
}


void loadPixmap2MainWindow()
{
	unsigned int bitw,bith,hotx,hoty;
	Pixmap         shapemask;
	char xbmpath[128]={0}; 
    XResizeWindow(dpy,mainWindow,skin_config.SkinMainBar.mbbg_xpm.width,skin_config.SkinMainBar.mbbg_xpm.height);
	if( strlen(skin_config.SkinMainBar.mbmask_xbm.xpmname) != 0)
	{
		//掩图信息也在此加载
		sprintf(xbmpath, "%s/xpm/%s/%s", PKGDATADIR, skinType,skin_config.SkinMainBar.mbmask_xbm.xpmname);
		//printf("SkinMainBar.mbmask_xbm.xpmname%s/%s\n",xbmpath,skin_config.SkinMainBar.mbmask_xbm.xpmname);
		//为增加透明层,建立掩码
		XReadBitmapFile(dpy, mainWindow,xbmpath, &bitw, &bith, &shapemask,(int *)&hotx, (int *)&hoty);
	    XShapeCombineMask(dpy,mainWindow,0,0,0,shapemask,0);
	 }

	//加载背景图
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.mbbg_xpm,&Bar,&Barmask);
	//加载logo&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.logo_xpm,&Logo,&Logomask);
	//加载中英标点&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.zhpunc_xpm,&Punc[0],&Puncmask[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.enpunc_xpm,&Punc[1],&Puncmask[1]);
	//加载半角全角&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.halfcorner_xpm,&Corner[0],&Cornermask[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.fullcorner_xpm,&Corner[1],&Cornermask[1]);
	//加载GBk&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.gbkoff_xpm,&GBK[0],&GBKmask[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.gbkon_xpm, &GBK[1],&GBKmask[1]);
	//加载联想图标&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.lxoff_xpm,&LX[0],&LX[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.lxon_xpm, &LX[1],&LX[1]);
	//加载简体繁体图标&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.chs_xpm,&GBKT[0],&GBKTmask[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.cht_xpm,&GBKT[1],&GBKTmask[1]);
	//加载加解锁图标&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.unlock_xpm,&Lock[0],&Lockmask[0]);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.lock_xpm,  &Lock[1],&Lockmask[1]);
	//加载虚拟键盘图标&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.vkhide_xpm,&VK,&VKmask);
	//加载输入法种类图标&
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.english_xpm,&English,&Englishmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.pinyin_xpm,&Pinyin,&Pinyinmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.shuangpin_xpm,&Shuangpin,&Shuangpinmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.quwei_xpm,&Quwei,&Quweimask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.wubi_xpm,&Wubi,&Wubimask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.mixpywb_xpm,&Mix,&Mixmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.erbi_xpm,&Erbi,&Erbimask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.cj_xpm,&CangJi,&CangJimask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.wanfeng_xpm,&Wanfeng,&Wanfengmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.bingcan_xpm,&Bingcan,&Bingcanmask);
	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.ziran_xpm,&Ziran,&Ziranmask);

	loadXpm2Pixmap(mainWindow,&skin_config.SkinMainBar.otherim_xpm,&Otherim,&Otherimmask);
	
}


void loadPixmap2InputWindow()
{
	char xbmpath[128]={0};
	int depth ; 
	XGCValues values;
	
    if( skin_config.SkinInputBar.ibmask_xbm.xpmname != 0)
    {
		sprintf(xbmpath, "%s/xpm/%s/%s", PKGDATADIR, skinType,skin_config.SkinInputBar.ibmask_xbm.xpmname);
		//printf("ibmask_xbm.xpmname:%s/%s\n",xbmpath,skin_config.SkinInputBar.ibmask_xbm.xpmname);
		XReadBitmapFile(dpy, inputWindow, xbmpath, &bitw, &bith, &Inputshapemask,(int *)&hotx,(int *) &hoty); 

	}
	//加载输入窗图片
	loadXpm2Pixmap(inputWindow,&skin_config.SkinInputBar.ibbg_xpm,&Input,&Inputmask);
	loadXpm2Pixmap(inputWindow,&skin_config.SkinInputBar.backarrow_xpm,&Prev,&Prevmask);
	loadXpm2Pixmap(inputWindow,&skin_config.SkinInputBar.forwardarrow_xpm,&Next,&Nextmask);
	
	//确定mask的gc值
	values.foreground = WhitePixel(dpy,0);
	values.background = BlackPixel(dpy,0);
	values.function = GXcopy;

	//为掩图而创建的pixmap区,长度最多为1000,色深定为一
	inputtmpmask = XCreatePixmap(dpy,inputWindow, 1000, skin_config.SkinInputBar.ibbg_xpm.height, 1);
	maskgc = XCreateGC(dpy,inputtmpmask ,(GCForeground | GCBackground), &values);
	
	//为输入条创建gc和pixmap缓冲区，长度为1000,色深默认
	depth = DefaultDepth(dpy, DefaultScreen(dpy));
	inputbar = XCreatePixmap(dpy,inputWindow, 1000, skin_config.SkinInputBar.ibbg_xpm.height, depth);		
	inputbargc = XCreateGC(dpy,inputWindow ,0,0);
}

/*
**根据由输入和显示字的长度确定的输入条长度来画输入条,目前只支持重复某一段来延伸输入条长度
*/
void DisplayInputBar(int barlen)
{
	int i=0;
	
	//unsigned int bitwtmp,bithtmp,hotxtmp,hotytmp;

	int repainttimes=0;
	int remain_width=0;
	int barwidth=skin_config.SkinInputBar.ibbg_xpm.width;
	int barheight=skin_config.SkinInputBar.ibbg_xpm.height;
	int resize_x=skin_config.SkinInputBar.resizeposition_x;
	int resize_w=skin_config.SkinInputBar.resizeposition_w;
	
	if(barlen>=1000)
		barlen=1000;
	if(barlen < barwidth)
			barlen=barwidth;
	//如果输出条图比输出长度宽,则显示完整图片  resize=0 长度不智能变化  如果重复段的长度太短,可能导致重复太频繁,更新速度慢.
	if(  barlen<=barwidth || skin_config.SkinInputBar.resize == 0 ||  skin_config.SkinInputBar.resizeposition_w <=30 )
	{
		//掩图存在则接合掩图
		if( strlen(skin_config.SkinInputBar.ibmask_xbm.xpmname) != 0 )
				XShapeCombineMask(dpy,inputWindow,0,0,0,Inputshapemask,0);
		putXpmImage(inputWindow, inputbargc,Input,0, 0, 0,
			skin_config.SkinInputBar.ibbg_xpm.position_x,
			skin_config.SkinInputBar.ibbg_xpm.position_y, 
			skin_config.SkinInputBar.ibbg_xpm.width,
			skin_config.SkinInputBar.ibbg_xpm.height); 
		return ;
	}

	//重绘的次数
	repainttimes=(barlen-barwidth)/resize_w;
	//不够汇一次的剩余的长度
	remain_width=(barlen-barwidth)%resize_w;

	//开始组掩图
	if( strlen(skin_config.SkinInputBar.ibmask_xbm.xpmname) != 0 )
	{		
		//根据次数和剩余长度组掩图 最先画出开始到重绘点+重绘长度

		XCopyArea(dpy, Inputshapemask, inputtmpmask, maskgc,0, 0,resize_x+resize_w,barheight,0,0);

		//根据重绘次数重绘
		for(i=0;i< repainttimes;i++)
		{
			XCopyArea(dpy, Inputshapemask, inputtmpmask, maskgc,resize_x, 0,resize_w,barheight,resize_x+resize_w+i*resize_w, 0);
		}

		//剩余段 和末段
		XCopyArea(dpy, Inputshapemask, inputtmpmask, maskgc,resize_x, 0,remain_width, barheight,resize_x+resize_w+repainttimes*resize_w, 0);
		XCopyArea(dpy, Inputshapemask, inputtmpmask, maskgc,
					resize_x+resize_w, 0,
					barwidth-resize_x-resize_w,barheight,
					resize_x+resize_w+repainttimes*resize_w+remain_width,0);
		
		XShapeCombineMask(dpy,inputWindow,0,0,0,inputtmpmask,0);
	}

	//开始组输入条
	XCopyArea(dpy, Input, inputbar, inputbargc,0, 0,resize_x+resize_w,barheight,0,0);
	
	//根据重绘次数重绘
	for(i=0;i< repainttimes;i++)
	{
		XCopyArea(dpy, Input, inputbar, inputbargc,resize_x, 0,resize_w,barheight,resize_x+resize_w+i*resize_w, 0);
	}
	
	//剩余段 和末段
	XCopyArea(dpy, Input, inputbar, inputbargc,resize_x, 0,remain_width, barheight,resize_x+resize_w+repainttimes*resize_w, 0);
	XCopyArea(dpy, Input, inputbar,inputbargc,
				resize_x+resize_w, 0,
				barwidth-resize_x-resize_w,barheight,
				resize_x+resize_w+repainttimes*resize_w+remain_width,0);
	putXpmImage(inputWindow,inputbargc, inputbar, 0,0, 0,0, 0,barlen,barheight);
}

void DisplaySkin(char * skinname)
{
	memset(skinType,0,sizeof(skinType));//buf长度太短，注意改进
	strcpy(skinType,skinname);
	XUnmapWindow (dpy, mainWindow);
	XUnmapWindow (dpy, inputWindow);

	XDestroyWindow(dpy, mainWindow);
	XDestroyWindow(dpy, inputWindow);
	CreateMainWindow ();
	CreateInputWindow ();

 	if( !ISDEFAULT)
 	{
		loadPixmap2MainWindow();
		loadPixmap2InputWindow();

		XResizeWindow (dpy, mainWindow, skin_config.SkinMainBar.mbbg_xpm.width, skin_config.SkinMainBar.mbbg_xpm.height);
	}
	DrawMainWindow ();
	DrawInputWindow ();
	if( !ISDEFAULT)
	{
		DisplayMainWindow();
		//DisplayInputWindow();
	}
	else 
	{
		DisplayMainWindow ();
		//DisplayInputWindow();
	}
}
//xpm图片文件加载函数完成
/*-------------------------------------------------------------------------------------------------------------*/
//xpm目录下读入skin的文件夹名

int loadSkinDir()
{
	DIR * dir;
	struct dirent * drt;
	struct stat file_stat;
	char pathbuf[128]={0};	
	int i=0;
	skinCount=1;

	for(i=0;i<10;i++)
	{
		memset(&skinBuf[i],0,sizeof(_Skin_Dir));
	}

	sprintf(pathbuf, "%s/xpm/", PKGDATADIR);

	dir= opendir(pathbuf);

	if (dir == NULL)
	{	
		perror("can't open the dir");
		return -1;
	}
	
	strcpy(skinBuf[0].dirbase,"default");
	i=1;
	while((drt = readdir(dir)) != NULL)
	{
		memset(pathbuf,0,sizeof(pathbuf));
		if( strcmp( drt->d_name, ".") == 0 || strcmp(drt->d_name,"..") == 0)
			continue;
		sprintf(pathbuf,"%s/xpm/%s",PKGDATADIR,drt->d_name);
		//printf("filename:%s\n",pathbuf);
		if( stat(pathbuf,&file_stat) == -1)
		{
			printf("can't get dirinfo\n");
			//return -1;
		}
	    //如果是目录的话则读入到dirbuf中
	
		if ( file_stat.st_mode & S_IFDIR)
		{	
			
			if (strlen(pathbuf) != 0)
			{	
				strcpy(skinBuf[i].dirbuf,pathbuf);
				strcpy(skinBuf[i].dirbase,drt->d_name);
				i++;
				//printf("*******%s\n",pathbuf);
			}	
		}	
	}
	
	
	for(i=0 ;i<10;i++)
	{	
		if(strlen(skinBuf[i].dirbuf) == 0)
			continue;
		skinCount++;	
	}
	//printf("skinCount:%d \n",skinCount);
	closedir(dir);

	return 0;
}


