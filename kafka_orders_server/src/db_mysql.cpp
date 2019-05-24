#include "db_mysql.h"
#include "easylogging++.h"
#include <utility>
#include "json.hpp"
using json = nlohmann::json;


DBMysql* g_db_mysql = new DBMysql();

DBMysql::DBMysql()
{
	connect_ = nullptr;
}

DBMysql::~DBMysql()
{
	delete connect_;
}


bool DBMysql::OpenDB()
{
	if (!connect_)
		return false;
    

    char value = 1;
	if (mysql_init(&mysql_) == NULL) {
		//std::cout << "初始化数据库失败" << std::endl;
		return false;
	}
    mysql_options(&mysql_, MYSQL_OPT_RECONNECT, (char *)&value);

	if (!mysql_real_connect(&mysql_, connect_->url.c_str(), connect_->user_name.c_str(),
							connect_->user_pass.c_str(),connect_->use_db.c_str(), 
							connect_->port, NULL, 0))
	{
		std::string error= mysql_error(&mysql_);
		LOG(ERROR) << "openDB : " << "数据库连接失败:"<<error;
		return false;
	}

	return true;
}

void DBMysql::SetConnect(MysqlConnect*connect)
{
	connect_ = connect;
}

void DBMysql::GetConnect(MysqlConnect*connect)
{
	connect = connect_;
}
void DBMysql::InsertData(const std::string& insert_sql)
{

	int ret = mysql_real_query(&mysql_, insert_sql.c_str(), strlen(insert_sql.c_str()));
	std::cout << insert_sql << std::endl;
	if (ret != 0 && mysql_errno(&mysql_) != 1062) 
	{
		LOG(INFO) << "exec sql failed" << insert_sql ;
	}	

}

void DBMysql::DeleteData(const std::string& delete_sql)
{
   int ret = mysql_real_query(&mysql_, delete_sql.c_str(), strlen(delete_sql.c_str()));
   if (ret != 0 ) 
    {
         LOG(INFO) << "exec sql failed" << delete_sql ;
    }
}

bool DBMysql::DoSqlQuery(const std::string& sql_query,const std::string& query_type ,int task_id )
{
	
  int res = mysql_query(&mysql_, sql_query.c_str());
  if (res)
  {
  	LOG(ERROR) << "task id : " << task_id << "SQL FAIL:" << sql_query << "\n";
  	return false;
  }

  if (query_type == "1")
  {
	  MYSQL_RES *result = mysql_store_result(&mysql_);
	  size_t num_rows = mysql_num_rows(result);
	  for (size_t i = 0; i < num_rows; ++i) 
	  {
		  MYSQL_ROW row = mysql_fetch_row(result);
		  LOG(INFO) << "task id : " << task_id << "column[0]:"<<row[0] << "\n";
	  }   
	  mysql_free_result(result);
  }

  return true;
}

bool DBMysql::GetDataFromSql(const std::string& select_sql, std::string& data_response)
{
	int ret = mysql_real_query(&mysql_, select_sql.c_str(), strlen(select_sql.c_str()));
	MYSQL_RES *result = mysql_store_result(&mysql_);
	size_t num_rows = mysql_num_rows(result);
	for (size_t i = 0; i < num_rows; ++i) 
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		/*last_id = strtoull(row[0], NULL, 0);
		uint32_t user_id = strtoul(row[1], NULL, 0);
		const char *asset = row[2];
		if (!asset_exist(asset)) {
			continue;
		}
		uint32_t type = strtoul(row[3], NULL, 0);
		mpd_t *balance = decimal(row[4], asset_prec(asset));*/
	}
	mysql_free_result(result);
	return true;	
}

uint64_t DBMysql::GetMaxOffset(const std::string&sql_select)
{
	int ret = mysql_real_query(&mysql_,sql_select.c_str(), strlen(sql_select.c_str()));
	MYSQL_RES *result = mysql_store_result(&mysql_);
	if (!result)
	{
            return 0;
	}	 

	size_t num_rows = mysql_num_rows(result);
	int offset = 0;
	for (size_t i = 0; i < num_rows; ++i) 
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		std::string str_offset = row[0];
		if ( str_offset == "")
		{
			continue;
		}
		offset = std::atoi(str_offset.c_str());	
    }
	mysql_free_result(result);
	return offset;
}

void DBMysql::GetFeeRateFrom(const std::string& stock, const std::string& money,const uint64_t& time,std::string& ask_fee_rate,std::string& bid_fee_rate)
{
	std::string sql_select = "select price FROM kafka_deals where time < "  + std::to_string(time) + " and market = '" +stock+"CNY' order by  offset Desc limit 1;"; 
	int ret = mysql_real_query(&mysql_,sql_select.c_str(), strlen(sql_select.c_str()));
	MYSQL_RES *result = mysql_store_result(&mysql_);
	size_t num_rows = mysql_num_rows(result);
	for (size_t i = 0; i < num_rows; ++i) 
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		ask_fee_rate = row[0];
    }
	mysql_free_result(result);

	sql_select = "select price FROM kafka_deals where time < "  + std::to_string(time) + " and market = '" +money+"CNY' order by  offset Desc limit 1;"; 
	ret = mysql_real_query(&mysql_,sql_select.c_str(), strlen(sql_select.c_str()));
	MYSQL_RES *result1 = mysql_store_result(&mysql_);
	 num_rows = mysql_num_rows(result1);
	for (size_t i = 0; i < num_rows; ++i) 
	{
		MYSQL_ROW row = mysql_fetch_row(result1);
		bid_fee_rate = row[0];
    }
	mysql_free_result(result1);


}
void DBMysql::TransferData(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method)
{
}

void DBMysql::TransferDataMutil(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id)
{
}

void DBMysql::CheckDataMutil(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id)
{
}
void DBMysql::CheckData(const std::string&sql_select, const std::string&rpc_url,const std::string&rpc_method, int thread_count,int init_id)
{
}

void DBMysql::BatchSqlQuery(const std::vector<std::string>& vect_sql)
{
    mysql_query(&mysql_,"START TRANSACTION");
    std::string sql_query;
    int ret = 0;
    for(uint32_t i = 0; i < vect_sql.size(); i ++)
    {
        sql_query = vect_sql.at(i);
        ret = mysql_real_query(&mysql_, sql_query.c_str(), strlen(sql_query.c_str()));
        if (ret != 0 && mysql_errno(&mysql_) != 1062) 
        {
            LOG(INFO) << "exec sql failed" << sql_query ;
        }
    }
    mysql_query(&mysql_,"COMMIT");
}

void DBMysql::CloseDB()
{
	mysql_close(&mysql_);
}
