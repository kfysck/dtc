#include <iostream>
#include <vector>
#include <string.h>
#include <time.h>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <map>

#include "cm_load.h"
#include "config.h"
#include "protocol.h"
#include "cm_conn.h"
#include "log.h"

#define TABLE_CONF_NAME "/etc/dtc/table.yaml"
#define CACHE_CONF_NAME "/etc/dtc/dtc.yaml"

using namespace std;

#define STRCPY(d,s) do{ strncpy(d, s, sizeof(d)-1); d[sizeof(d)-1]=0; }while(0)
#define MIN(x,y) ((x)<=(y)?(x):(y))

#define GET_STR_CFG(s, k, d) do{ p = stCfgFile.GetStrVal(s, k); \
    if(p==NULL || p[0]==0){ \
	fprintf(stderr, "%s is null!\n", k); \
	return(-1); \
    } \
    STRCPY(d, p); \
}while(0)

std::map<string, TableInfo> g_table_set;


/* 前置空格已经过滤了 */
static char* skip_blank(char *p)
{
	char *iter = p;
	while(!isspace(*iter) && *iter != '\0')
		++iter;

	*iter = '\0';
	return p;
}

int init_servers(TableInfo* ti, const char* tableName)
{
	return 0;
}

ConfigHelper::ConfigHelper ()
{
}

ConfigHelper::~ConfigHelper ()
{
}

bool ConfigHelper::load_dtc_config()
{
	try 
	{
        dtc = YAML::LoadFile(CACHE_CONF_NAME);
		if(dtc.IsNull())
		{
			log4cplus_error("dtc null");
			return false;
		}
	}
	catch (const YAML::Exception &e) 
	{
		log4cplus_error("load dtc file error:%s", e.what());
		return false;
	}

	try 
	{
        table = YAML::LoadFile(TABLE_CONF_NAME);
		if(table.IsNull())
		{
			log4cplus_error("table null");
			return false;
		}
	}
	catch (const YAML::Exception &e) 
	{
		log4cplus_error("load table file error:%s", e.what());
		return false;
	}
	
	if(!load_hot_inst_info())
	{
		log4cplus_error("load hot db config error");
		return false;
	}

	return true;
}

bool ConfigHelper::load_hot_inst_info()
{
	memset(&shardingsphere, 0, sizeof(DBHost));

	if(!dtc["vhot"])
		return false;

	if( !dtc["vhot"]["addr"] || 
		!dtc["vhot"]["username"] ||
		!dtc["vhot"]["password"] ||
		!dtc["vhot"]["database"])
		return false;

	const char* addr = dtc["vhot"]["addr"].as<string>().c_str();
	const char* p = strrchr(addr, ':');
	if(p == NULL){
		strncpy(shardingsphere.Host, addr, sizeof(shardingsphere.Host)-1 );
		shardingsphere.Port = 0;
	}
	else{
		strncpy(shardingsphere.Host, addr, MIN(p - addr, (int)sizeof(shardingsphere.Host)-1) );
		shardingsphere.Port = atoi(p+1);
	}
	
	strncpy(shardingsphere.User, dtc["vhot"]["username"].as<string>().c_str(), sizeof(shardingsphere.User)-1 );
	strncpy(shardingsphere.Password, dtc["vhot"]["password"].as<string>().c_str(), sizeof(shardingsphere.Password)-1 );
	strncpy(shardingsphere.DbName, dtc["vhot"]["database"].as<string>().c_str(), sizeof(shardingsphere.DbName)-1 );
	
	shardingsphere.ConnTimeout = 10;
	shardingsphere.ReadTimeout = 10;

	log4cplus_debug("shardingsphere Host:%s:%d, user:%s, pwd:%s, db:%s",
		shardingsphere.Host, shardingsphere.Port, shardingsphere.User, shardingsphere.Password, shardingsphere.DbName);
	return true;
}

int ConfigHelper::GetIntValue(const char* key, int default_value)
{
	return default_value;
}

std::vector<int> ConfigHelper::GetIntArray(const char* key)
{
	std::vector<int> v;

	return v;
}

std::string ConfigHelper::GetStringValue(const char* key, std::string default_value)
{
	return default_value;
}
