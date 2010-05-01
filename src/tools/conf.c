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
 * @file conf.c
 * @author CSSlayer wengxt@gmail.com
 * @date 2010-04-30
 *
 * @brief 新配置文件读写
 */

#include <stdio.h>
#include "conf.h"

ConfigFile *ParseConfigFile(char *filename, ConfigFileDesc* fileDesc)
{
}

Bool CheckConfig(ConfigFile *configFile, ConfigFileDesc* fileDesc)
{
}

ConfigFileDesc *ParseConfigFileDesc(char* filename)
{
    int i;
    ConfigFile *cfile = ParseIni(filename);
    ConfigFileDesc *cdesc = malloc(sizeof(ConfigFileDesc));
    ConfigGroup* group;
    Foreach(i, cfile->groups)
    {
        
    }
    return 0;
}

ConfigFile* ParseIni(char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp)
        return NULL;
    char *line = NULL, *buf = NULL;
    size_t len = 0;
    int lineLen = 0;
    ConfigFile* cfile = malloc(sizeof(ConfigFile));
    cfile->groups = NULL;
    ConfigGroup* curGroup = NULL;
    int lineNo = 0;
    while(getline(&buf, &len, fp) != 1)
    {
        lineNo ++;
        line = trim(buf);
        lineLen = strlen(line);

        if (lineLen == 0 || line[0] == '#')
            continue;

        if (line[0] == '[')
        {
            if (line[lineLen - 1] == ']' && lineLen != 2)
            {
                ConfigGroup cgroup;
                ArrayAddNew(cfile->groups, ConfigGroup);
                curGroup = ArrayLast(cfile->group);
            }
            else
            {
                FcitxLog(ERROR, _("Configure group name error: %s:%d", fileName, lineNo));
                return NULL;
            }
            char *groupName;
            groupName = malloc(sizeof(char) * (lineLen - 2 + 1) );
            strncpy(groupName, line + 1, lineLen - 2);
            groupName[lineLen - 2] = '\0';
            curGroup->groupName = groupName;
            curGroup->options = NULL;
            curGroup->groupDesc = NULL;
        }
        else
        {
            if (!curGroup)
            {
            }
            char *value = strchr(line, '=');
            if (!value)
            {
                FcitxLog(WARNING, _("Invalid Entry: %s:%d missing '='", fileName, lineNo));
                goto next_line;
            }
            value = '\0';
            value ++;
            char *name = value;
            ConfigOption *option;
            ArrayAddNew(curGroup->options, ConfigOption);
            option = ArrayLast(curGroup->options);
            option->optionName = strdup(name);
            option->rawValue = strdup(value);
        }

next_line:
        free(line);
        continue;
    }

}


