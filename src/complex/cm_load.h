#ifndef _DBP_DBCONFIG_H_
#define _DBP_DBCONFIG_H_

#include "global.h"
#include "yaml-cpp/yaml.h"
#include "cm_conn.h"

class TableInfo;

int GetTableIdx(void* Key, int FieldType, TableInfo* dbConfig);

class ConfigHelper 
{
private:
	std::string m_data;
	YAML::Node table;
	YAML::Node dtc;

public:
	DBHost shardingsphere;

public:
	ConfigHelper ();
	~ConfigHelper ();

    int GetIntValue(const char* key, int default_value = 0);
    std::string GetStringValue(const char* key, std::string default_value = "");
	std::vector<int> GetIntArray(const char* key);

	bool load_dtc_config();
	
	bool load_hot_inst_info();
};

#endif