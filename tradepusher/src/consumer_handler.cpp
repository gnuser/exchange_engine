#include <iostream>
#include "consumer_handler.h"
#include "json.hpp"
#include "common.h"
#include "db_mysql.h"

static ConfManager gs_conf;

using json = nlohmann::json;

ConsumerHandler::ConsumerHandler()
{
}

ConsumerHandler::~ConsumerHandler()
{
}

bool IsUserIDs(uint64_t uid)
{
    for (int i = 0; i < users.size(); ++i)
    {
        if (users.at(i) == uid)
	    return true;
    }	 
    return false;
}

void ConsumerHandler::UpdateDB(std::string msg, int offset)
{
    json json_data = json::parse(msg);
    uint64_t time = (uint64_t) json_data.at(0).get<double>();
    std::string market = json_data.at(1).get<std::string>();
    uint64_t ask_id = json_data.at(2).get<uint64_t>();
    uint64_t bid_id = json_data.at(3).get<uint64_t>();
    uint64_t ask_user_id = json_data.at(4).get<uint64_t>();
    uint64_t bid_user_id = json_data.at(5).get<uint64_t>();
    std::string price = json_data.at(6).get<std::string>();
    std::string amount = json_data.at(7).get<std::string>();

    json trade;
    trade["time"] = time;
    trade["market"] = market;
    trade["price"] = price;
    trade["amount"] = amount;
    if (IsUserIDs(ask_user_id) && !IsUserIDs(bid_user_id))
    {    
	trade["side"] = 1;
	trade["user_id"] = ask_user_id;
	trade["order_id"] = ask_id;
        std::string buffer = trade.dump();
        std::cout << "kafka_push: " << buffer << std::endl;

        if (PUSH_DATA_SUCCESS != g_kafka_producer->push_data_to_kafka(buffer.c_str(), buffer.size()))
            std::cout << "push kafka fail: " << buffer << std::endl;
    }

    if (IsUserIDs(bid_user_id) && !IsUserIDs(ask_user_id))
    {    
	trade["side"] = 2;
	trade["user_id"] = bid_user_id;
	trade["order_id"] = bid_id;
        std::string buffer = trade.dump();
        std::cout << "kafka_push: " << buffer << std::endl;

        if (PUSH_DATA_SUCCESS != g_kafka_producer->push_data_to_kafka(buffer.c_str(), buffer.size()))
            std::cout << "push kafka fail: " << buffer << std::endl;
    }

    std::string sql = "insert into kafka_offset (offset) values (" + std::to_string(offset)  + ")";
    if (!g_db_mysql->ExecuteSql(sql))
        std::cout << "insert kafka offset failed." << std::endl;
}



