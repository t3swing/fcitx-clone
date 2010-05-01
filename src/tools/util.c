/***************************************************************************
 *   Copyright (C) 20010~2010 by CSSlayer                                  *
 *   wengxt@gmail.com                                                      *
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
 * @file util.c
 * @brief 通用函数
 * @author CSSlayer
 * @version 4.0.0
 * @date 2010-05-02
 */

/** 
 * @brief 去除字符串首末尾空白字符
 * 
 * @param s
 * 
 * @return malloc的字符串，需要free
 */
char *trim(char *s)
{
    register char *end;
    register char csave;
    
    while (isspace(*s))                 /* skip leading space */
        ++s;
    end = strchr(s,'\0') - 1;
    while (isspace(*end))               /* skip trailing space */
        --end;
    
    csave = end[1];
    end[1] = '\0';
    s = strdup(s);
    end[1] = csave;
    return (s);
}


/** 
 * @brief 获得字符串数组，返回对应的XDGPath
 * 
 * @param 用于返回字符串数组长度的指针
 * @param XDG_*_HOME环境变量名称
 * @param 默认XDG_*_HOME路径
 * @param XDG_*_DIRS环境变量名称
 * @param 默认XDG_*_DIRS路径
 * 
 * @return 字符串数组
 */
char *GetXDGPath(
        size_t *len,
        const char* homeEnv,
        const char* homeDefault,
        const *dirsEnv,
        const *dirsDefault)
{
    char* dirHome;
    const char *xdgDirHome = getenv(homeEnv);
    if (xdgDirHome && xdgDirHome[0])
    {
        dirHome = strdup(dirNome);
    }
    else
    {
        const char *home = getenv("HOME");
        dirHome = malloc(strlen(home) + 1 + strlen(homeDefault) + 1);
        sprintf(dirHome, "%s/%s", home, home_default);
    }

    char *dirs;
    const char *xdgDirs = getenv(dirsEnv);

    if (xdgDirs && xdgDirs[0])
    {
        dirs = malloc( strlen(dirHome) + 1 + strlen(xdgDirs) + 1);
        sprintf(dirs, "%s:%s", dirHome, xdgDirs);
    }
    else
    {
        dirs = malloc (strlen(dirHome) + 1 + strlen(dirs_default) + 1);
        sprintf(dirs, "%s:%s", dirHome, dirsDefault);
    }
    
    xfree(dirHome);
    
    /* count dirs and change ':' to '\0' */
    size_t dirsCount = 1;
    char *tmp = dirs;
    while (*tmp) {
        if (*tmp == ':') {
            *tmp = '\0';
            dirsCount++;
        }
        tmp++;
    }
    
    /* alloc char pointer array and puts locations */
    size_t i;
    char **dirsArray = malloc(dirsCount * sizeof(char*));
    for (i = 0; i < dirs_count; ++i) {
        dirsArray[i] = dirs;
        while (*dirs) {
            dirs++;
        }
        dirs++;
    }
    
    *len = dirsCount;
    return dirsArray;
}

/** 
 * @brief 返回申请后的内存，并清零
 * 
 * @param 申请的内存长度
 * 
 * @return 申请的内存指针
 */
void *malloc0(size_t bytes)
{
    void *p = malloc(bytes);
    if (!p)
        return NULL;

    memset(p, 0, bytes);
    return 0;
}
