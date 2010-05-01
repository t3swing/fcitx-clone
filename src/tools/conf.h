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
 * @file conf.h
 * @author CSSlayer wengxt@gmail.com
 * @date 2010-04-30
 *
 * @brief 新配置文件读写
 */

#include "core/ime.h"
#include "tools/uthash.h"

typedef enum ConfigType
{
	Integer
} ConfigType;

typedef struct ConfigOptionDesc
{
    char *optionName;
    char *descShort;
    char *descLong;
    ConfigType type;
    Bool required;
} ConfigOptionDesc;

typedef struct ConfigGroupDesc
{
    char *groupName;
    ConfigOptionDesc *optionsDesc;
    Bool required;
    UT_hash_handle hh;
} ConfigGroupDesc;

typedef struct ConfigFileDesc
{
    ConfigGroupDesc *groupsDesc;
    UT_hash_handle hh;
} ConfigFileDesc;

typedef struct ConfigOption
{
    char *optionName;
    char *rawValue;
    ConfigOptionDesc *optionDesc;
} ConfigOption;

typedef struct ConfigGroup
{
    char *groupName;
    ConfigGroupDesc *groupDesc;
    ConfigOption* options;
    UT_hash_handle hh;
} ConfigGroup;

typedef struct ConfigFile
{
    ConfigFileDesc *fileDesc;
    ConfigGroup* groups;
    UT_hash_handle hh;
} ConfigFile;

ConfigFile *ParseConfigFile(char *filename, ConfigFileDesc*);
Bool CheckConfig(ConfigFile *configFile, ConfigFileDesc* fileDesc);
ConfigFileDesc *ParseConfigFileDesc(char* filename);
ConfigFile* ParseIni(char* filename);
