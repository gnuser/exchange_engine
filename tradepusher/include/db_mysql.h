#ifndef DB_MYSQL_H
#define DB_MYSQL_H

#include <string>
#include <mutex>
#include <mysql/mysql.h>
#include "json.hpp"

using json = nlohmann::json;
class DBMysql
{
public:
    DBMysql();
    ~DBMysql();

    struct MysqlConnect
    {
	std::string url;
	int port;
	std::string user_name;
	std::string user_pass;
	std::string use_db;
    };

    enum JsonDataType
    {
        INT = 1,
        DOUBLE = 2,
        STRING =3
    };

    struct JsonDataFormat
    {
        uint32_t column_size;
        std::map<uint32_t, JsonDataType> map_column_type;
    };

public:	
    bool OpenDB();
    void CloseDB();
    void SetConnect(MysqlConnect*connect);
    void GetConnect(MysqlConnect*connect);

    bool ExecuteSql(const std::string& sql);
    bool GetDataAsJson(const std::string& select_sql, JsonDataFormat* json_data_format ,json& json_data);

private:
    MYSQL mysql_;
    MysqlConnect* connect_;
    std::mutex mtx;
};

extern DBMysql* g_db_mysql;

#endif
