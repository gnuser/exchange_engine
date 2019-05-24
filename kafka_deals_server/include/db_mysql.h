#ifndef DB_MYSQL_H
#define DB_MYSQL_H
#include <string>
#include <mysql/mysql.h>

class DBMysql
{
public:
	DBMysql();
	~DBMysql();

	struct MysqlConnect
	{
		std::string url;
		int			port;
		std::string user_name;
		std::string user_pass;
		std::string use_db;
	};

	
 	bool OpenDB();

	void CloseDB();

	void SetConnect(MysqlConnect*connect);

	void GetConnect(MysqlConnect*connect);

	void InsertData(const std::string&insert_sql);

    bool DoSqlQuery(const std::string& sql_query,const std::string& query_type,int task_id);
	
	bool GetDataFromSql(const std::string& select_sql, std::string&data_response);

	void TransferData(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_data);

	void TransferDataMutil(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id);

	void CheckDataMutil(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id);
	void CheckData(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id);
	
	uint64_t GetMaxOffset(const std::string&sql_select);

	void GetFeeRateFrom(const std::string& stock, const std::string& money,const uint64_t& time,std::string& ask_fee_rate,std::string& bid_fee_rate);

private:
	MYSQL mysql_;
	MysqlConnect* connect_;
};

extern DBMysql* g_db_mysql;
#endif
