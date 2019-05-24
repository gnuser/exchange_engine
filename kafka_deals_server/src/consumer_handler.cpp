#include "db_mysql.h"
#include "json.hpp"
#include <iostream>
#include "common.h"
#include "consumer_handler.h"
static ConfManager gs_conf;

using json = nlohmann::json;
static DBMysql* pdb = new DBMysql();
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
}

ConsumerHandler::~ConsumerHandler()
{

}

void ConsumerHandler::UpdateDB(std::string msg_update,int offset)
{
	json json_data = json::parse(msg_update);
	if (json_data.size() < 24)
		return;

	std::cout << "offset :" << offset << std::endl;
	std::cout << "UpdateDB :" << msg_update << std::endl;

	uint64_t time = (uint64_t) json_data.at(0).get<double>();
	std::string market = json_data.at(1).get<std::string>();
	uint64_t ask_id = json_data.at(2).get<uint64_t>();
	uint64_t bid_id = json_data.at(3).get<uint64_t>();
	uint64_t ask_user_id = json_data.at(4).get<uint64_t>();
	uint64_t bid_user_id = json_data.at(5).get<uint64_t>();
	std::string price = json_data.at(6).get<std::string>();
	std::string amount = json_data.at(7).get<std::string>();
	std::string ask_fee = json_data.at(8).get<std::string>();
	std::string bid_fee = json_data.at(9).get<std::string>();
	int side = json_data.at(10).get<int>();
	uint64_t id = json_data.at(11).get<uint64_t>();
	std::string stock = json_data.at(12).get<std::string>();
	std::string money = json_data.at(13).get<std::string>();

	std::string ask_token = json_data.at(14).get<std::string>();
	std::string ask_discount = json_data.at(15).get<std::string>();
	std::string ask_token_rate = json_data.at(16).get<std::string>();
	std::string ask_asset_rate = json_data.at(17).get<std::string>();
	std::string ask_deal_token = json_data.at(18).get<std::string>();
	std::string bid_token = json_data.at(19).get<std::string>();
	std::string bid_discount = json_data.at(20).get<std::string>();
	std::string bid_token_rate = json_data.at(21).get<std::string>();
	std::string bid_asset_rate = json_data.at(22).get<std::string>();
	std::string bid_deal_token = json_data.at(23).get<std::string>();

	std::string ask_fee_rate = "0.0";
	std::string bid_fee_rate = "0.0";

	if (money.compare("CNY") != 0)
	{
		pdb->GetFeeRateFrom(stock,money,time,ask_fee_rate,bid_fee_rate);
		std::cout << "ask_fee_rate " << ask_fee_rate << "bid_fee_rate " << bid_fee_rate <<std::endl; 
	}
	std::string insert_sql = "INSERT INTO `kafka_deals` (`offset`, `time`, `market`, `ask_id`, `bid_id`, `ask_user_id`, `bid_user_id`, `price`, `amount`, `ask_fee`, `bid_fee`, `side`, `id`, `stock`, `money`,`ask_fee_rate`,`bid_fee_rate`, `ask_token`, `ask_discount`, `ask_token_rate`, `ask_asset_rate`, `ask_deal_token`, `bid_token`, `bid_discount`, `bid_token_rate`, `bid_asset_rate`, `bid_deal_token`) VALUES ";

	std::string insert_value = "('" + std::to_string(offset) + "','" + std::to_string(time) + "','" + market + "','"+
	std::to_string(ask_id) + "','" + std::to_string(bid_id) + "','" + std::to_string(ask_user_id) + "','" + 
	std::to_string(bid_user_id) + "','" + price + "','" + amount +"','"+ask_fee +"','"+bid_fee+"','" +
	std::to_string(side) + "','" + std::to_string(id) + "','" + stock + "','" + money +"','"+ask_fee_rate +"','" + bid_fee_rate + "','" + 
	ask_token + "','" + ask_discount + "','" + ask_token_rate + "','" + ask_asset_rate + "','" + ask_deal_token + "','" + 
        bid_token + "','" + bid_discount + "','" + bid_token_rate + "','" + bid_asset_rate + "','" + bid_deal_token + "');" ;
	insert_sql = insert_sql + insert_value;

	pdb->InsertData(insert_sql);
}




