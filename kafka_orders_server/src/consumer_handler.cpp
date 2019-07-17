#include "consumer_handler.h"
#include "db_mysql.h"
#include "json.hpp"
#include <iostream>
#include "common.h"
#include <boost/date_time/posix_time/posix_time.hpp>

static ConfManager gs_conf;
using json = nlohmann::json;
static DBMysql* pdb = new DBMysql() ;

ConsumerHandler::ConsumerHandler()
{
	DBMysql::MysqlConnect* connect = new  DBMysql::MysqlConnect();

	connect->use_db = gs_conf.getArgs("mysqldb","api_tran");
	connect->user_pass = gs_conf.getArgs("mysqlpass","a");
	connect->port = std::atoi(gs_conf.getArgs("mysqlport","3306").c_str());
	connect->user_name = gs_conf.getArgs("mysqluser","snort");
	connect->url = gs_conf.getArgs("mysqlserver","192.168.0.18");

	pdb->SetConnect(connect);
	pdb->OpenDB();
	time_t timep;
	time(&timep);
	last_time_step_ = (uint64_t)timep;
}

ConsumerHandler::~ConsumerHandler()
{

}

void ConsumerHandler::UpdateDB(std::string msg_update,int offset)
{
	time_t timep;
	time(&timep);
	uint64_t current_time = (uint64_t)timep;


	std::cout << "UpdateDB : " << msg_update << std::endl;
	json json_data = json::parse(msg_update);

	uint16_t event = json_data["event"].get<uint16_t>();
	std::string money = json_data["money"].get<std::string>();
	std::string stock = json_data["stock"].get<std::string>();

	json json_order = json_data["order"];
        if (json_order["discount"].is_null())
                return;

	std::string deal_money = json_order["deal_money"].get<std::string>();
	uint64_t id = json_order["id"].get<uint64_t>();

	if (event == 3)
	{
		std::string delete_sql =  "delete from kafka_orders where id = '" + std::to_string(id) +"';";
		pdb->DeleteData(delete_sql);
		return ;
	}

	if (event == 2)
	{
		std::string delete_sql = "delete from kafka_orders where event = 1 and id = '" + std::to_string(id) +"';";
		pdb->DeleteData(delete_sql);
	}

	uint64_t user = json_order["user"].get<int64_t>();
	std::string source = "UNKNOW";
	std::string deal_fee = json_order["deal_fee"].get<std::string>();
	std::string market = json_order["market"].get<std::string>();
	uint16_t side = json_order["side"].get<uint16_t>();
	uint16_t type = json_order["type"].get<uint16_t>();
	std::string taker_fee = json_order["taker_fee"].get<std::string>();
	std::string price = json_order["price"].get<std::string>();
	uint64_t ctime = (uint64_t) json_order["ctime"].get<double>();
	uint64_t mtime = (uint64_t) json_order["mtime"].get<double>();
	std::string left = json_order["left"].get<std::string>();
	std::string amount = json_order["amount"].get<std::string>();
	std::string deal_stock = json_order["deal_stock"].get<std::string>();

	std::string token = json_order["token"].get<std::string>();
	std::string discount = json_order["discount"].get<std::string>();
	std::string token_rate = json_order["token_rate"].get<std::string>();
	std::string asset_rate = json_order["asset_rate"].get<std::string>();
	std::string deal_token = json_order["deal_token"].get<std::string>();

	if (json_order.find("source") != json_order.end())
	{
		source =  json_order["source"].get<std::string>();
	}

	std::string insert_sql = "INSERT INTO `kafka_orders` (`offset`, `event`, `money`, `stock`, `deal_money`, `id`, `user`, `deal_fee`, `market`, `side`, `type`, `taker_fee`, `price`, `ctime`, `mtime`, `left`, `amount`, `deal_stock`,`source`, `token`, `discount`, `token_rate`, `asset_rate`, `deal_token`) VALUES ";

	std::string insert_value = "('" + std::to_string(offset) + "','" + std::to_string(event) + "','" + money + "','"+
		stock + "','" + deal_money + "','" + std::to_string(id) + "','" + 
		std::to_string(user) + "','" + deal_fee + "','" + market +"','" + std::to_string(side)+"','" +
		std::to_string(type) + "','" + taker_fee + "','" + price + "','" + std::to_string(ctime) +"','"+ std::to_string(mtime) +"','" + 
		left + "','" + amount + "','" + deal_stock + "','"+ source + "','" + token + "','" + discount + "','" + token_rate + "','" + asset_rate + "','" + deal_token + "');" ;

	insert_sql = insert_sql + insert_value;
	pdb->InsertData(insert_sql);
	//vect_sql_.push_back(insert_sql);
}




