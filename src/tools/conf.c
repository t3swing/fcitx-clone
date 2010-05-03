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
#include <string.h>
#include "tools/conf.h"
#include "tools/util.h"

ConfigFile *ParseConfigFile(char *filename, ConfigFileDesc* fileDesc)
{
    ConfigFile *cfile = ParseIni(filename);
    if (CheckConfig(cfile, fileDesc))
    {
        return cfile;
    }
    return NULL;
}

Bool CheckConfig(ConfigFile *cfile, ConfigFileDesc* cfdesc)
{
    ConfigGroupDesc *cgdesc = NULL;
    for(cgdesc = cfdesc->groupsDesc;
        cgdesc != NULL;
        cgdesc = (ConfigGroupDesc*)cgdesc->hh.next)
    {
        ConfigOptionDesc *codesc = NULL;
        ConfigGroup* group;
        HASH_FIND_STR(cfile->groups, cgdesc->groupName, group);
        if (!group)
        {
            group = malloc(sizeof(ConfigGroup));
            group->groupName = strdup(cgdesc->groupName);
            group->groupDesc = cgdesc;
            group->options = NULL;
            HASH_ADD_KEYPTR(hh, cfile->groups, group->groupName, strlen(group->groupName), group);
        }
        for(codesc = cgdesc->optionsDesc;
            codesc != NULL;
            codesc = (ConfigOptionDesc*)codesc->hh.next)
        {
            ConfigOption *option;
            HASH_FIND_STR(group->options, codesc->optionName, option);
            if (!option)
            {
                if (!codesc->rawDefaultValue)
                {
                    FcitxLog(ERROR, "missing value: %s", codesc->optionName);
                    return False;
                }
                option = malloc(sizeof(ConfigOption));
                option->optionName = strdup(codesc->optionName);
                option->rawValue = strdup(codesc->rawDefaultValue);
                HASH_ADD_KEYPTR(hh, group->options, option->optionName, strlen(option->optionName), option);
            }
        }
    }
    return True;
}

/** 
 * @brief 
 * 
 * @param filename
 * 
 * @return 
 */
ConfigFileDesc *ParseConfigFileDesc(char* filename)
{
    ConfigFile *cfile = ParseIni(filename);
    ConfigFileDesc *cfdesc = malloc(sizeof(ConfigFileDesc));
    ConfigGroup* group;
    for(group = cfile->groups;
        group != NULL;
        group = (ConfigGroup*)group->hh.next)
    {
        ConfigGroupDesc *cgdesc = NULL;
        ConfigOption *options = group->options, *option = NULL;
        
        char * p = strchr(group->groupName, '/');
        if (p == NULL)
            continue;
        int groupNameLen = p - group->groupName;
        int optionNameLen = strlen(p + 1);
        if (groupNameLen != 0 && optionNameLen != 0)
            continue;
        
        char *groupName = malloc(sizeof(char) * groupNameLen);
        strncpy(groupName, group->groupName, groupNameLen);
        char *optionName = strdup(p + 1);
        
        HASH_FIND_STR(cfdesc->groupsDesc, groupName, cgdesc);
        if (!cgdesc)
        {
            cgdesc->groupName = groupName;
            cgdesc->optionsDesc = NULL;
            HASH_ADD_KEYPTR(hh, cfdesc->groupsDesc, cgdesc->groupName, groupNameLen, cgdesc);
        }
        else
            free(groupName);

        ConfigOptionDesc *codesc = malloc(sizeof(ConfigOptionDesc));
        codesc->optionName = optionName;
        codesc->rawDefaultValue = NULL;
        HASH_ADD_KEYPTR(hh, cgdesc->optionsDesc, codesc->optionName, optionNameLen, codesc);
        HASH_FIND_STR(options, "Description", option);
        if (option)
            codesc->desc= strdup(option->rawValue);
        else
            codesc->desc = strdup("");
        /* Processing Type */
        HASH_FIND_STR(options, "Type", option);
        if (option)
        {
            if (strcmp(option->rawValue, "Integer"))
                codesc->type = Integer;
            if (strcmp(option->rawValue, "Color"))
                codesc->type = Color;
            if (strcmp(option->rawValue, "String"))
                codesc->type = String;
            else
            {
                FcitxLog(WARNING, _("Unknown type, take it as string: %s:%s"), option->rawValue, filename);
                codesc->type = String;
            }
        }
        else
        {
            FcitxLog(WARNING, _("Missing type, take it as string: %s"), filename);
            codesc->type = String;
        }

        /* processing default value */
        HASH_FIND_STR(options, "DefaultValue", option);
        if (!option)
        {
            FcitxLog(WARNING, _("Not Reqiured option must have default value, make it required: %s"), filename);
        }
        codesc->rawDefaultValue = strdup(option->rawValue);
    }
    FreeConfigFile(cfile);
    return cfdesc;
}

void FreeConfigFile(ConfigFile* cfile)
{
    ConfigGroup *groups = cfile->groups, *curGroup;
    while(groups)
    {
        curGroup = groups;
        HASH_DEL(groups, curGroup);
        FreeConfigGroup(curGroup);
    }
}

void FreeConfigGroup(ConfigGroup *group)
{
    ConfigOption *option = group->options, *curOption;
    while(option)
    {
        curOption = option;
        HASH_DEL(option, curOption);
        FreeConfigOption(curOption);
    }
}

void FreeConfigOption(ConfigOption *option)
{
    free(option->optionName);
    free(option->rawValue);
    free(option);
}

/** 
 * @brief 
 * 
 * @param filename
 * 
 * @return 
 */
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
            if (!(line[lineLen - 1] == ']' && lineLen != 2))
            {
                FcitxLog(ERROR, _("Configure group name error: %s:%d"), filename, lineNo);
                return NULL;
            }
            char *groupName;
            groupName = malloc(sizeof(char) * (lineLen - 2 + 1) );
            strncpy(groupName, line + 1, lineLen - 2);
            groupName[lineLen - 2] = '\0';
            HASH_FIND_STR(cfile->groups, groupName, curGroup);
            if (curGroup)
                FcitxLog(WARNING, _("Duplicate group name, merge with the previous: %s %s:%d"),groupName, filename, lineNo);
            else
            {
                curGroup = malloc(sizeof(ConfigGroup));
                curGroup->groupName = groupName;
                curGroup->options = NULL;
                curGroup->groupDesc = NULL;
            }
            HASH_ADD_KEYPTR(hh, cfile->groups, curGroup->groupName, strlen(curGroup->groupName), curGroup);
        }
        else
        {
            if (!curGroup)
            {
            }
            char *value = strchr(line, '=');
            if (!value)
            {
                FcitxLog(WARNING, _("Invalid Entry: %s:%d missing '='"), filename, lineNo);
                goto next_line;
            }
            value = '\0';
            value ++;
            char *name = value;
            ConfigOption *option;
            HASH_FIND_STR(curGroup->options, name, option);
            if (option)
            {
                FcitxLog(WARNING, _("Duplicate option, ignore: %s:%d"), filename, lineNo);
            }
            else
            {
                option = malloc(sizeof(ConfigOption));
                option->optionName = strdup(name);
                option->rawValue = strdup(value);
                HASH_ADD_KEYPTR(hh, curGroup->options, option->optionName, strlen(option->optionName), option);
            }
        }

next_line:
        free(line);
        continue;
    }
    fclose(fp);
    return cfile;
}

Bool SaveConfig(char *filename, ConfigFile *cfile)
{
    FILE* fp = fopen(filename, "w");
    if (!fp)
        return False;
    ConfigGroup* group;
    for(group = cfile->groups;
        group != NULL;
        group = (ConfigGroup*)group->hh.next)
    {
        ConfigOption *option;
        if (!group->groupDesc)
            continue;
        fprintf(fp, "[%s]\n", group->groupName);
        for(option = group->options;
            option != NULL;
            option = (ConfigOption*)option->hh.next)
        {
            if (!option->optionDesc)
                continue;
            fprintf(fp, "%s=%s\n", option->optionName, option->rawValue);
        }
    }
    fclose(fp);
    return True;
}
