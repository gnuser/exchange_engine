#include "easylogging++.h"
#include <iostream>
#include "consumer.h"
#include "db_mysql.h"
#include "common.h"
#include <typeinfo>
#include <unistd.h> 

INITIALIZE_EASYLOGGINGPP
static uint64_t g_offset = 0;
static ConfManager g_conf;

void InitKafka()
{
	std::string topics  = g_conf.getArgs("topics","orders");//= g_conf.getArgs("");//"deals";
	std::string brokers = g_conf.getArgs("brokers","47.99.82.55:9092");//= "47.99.82.55:9092";
	std::string group = g_conf.getArgs("groups","1");// = "1";
	signal(SIGINT, sigterm);
	signal(SIGTERM, sigterm);

	std::shared_ptr<CConsumer> kafka_consumer_client_ = std::make_shared<CConsumer>(brokers, topics, group, g_offset);
	//std::shared_ptr<kafka_consumer_client> kafka_consumer_client_ = std::make_shared<kafka_consumer_client>();
	if (!kafka_consumer_client_->Init())
	{
		std::cerr << "init failed " << std::endl;
	}
	else
	{
		std::cout << "start kafka consumer\n";
		kafka_consumer_client_->Consume(1000);
	}

	std::cout <<  "consumer exit successfully!" << std::endl;
}

void InitMaxOffset()
{
	std::string sql_select = "select offset from kafka_orders order by offset DESC limit 1;";
	DBMysql::MysqlConnect* connect = new DBMysql::MysqlConnect();
	connect->use_db = g_conf.getArgs("mysqldb","api_tran");
	connect->user_pass = g_conf.getArgs("mysqlpass","a");
	connect->port = std::atoi(g_conf.getArgs("mysqlport","3306").c_str());
	connect->user_name = g_conf.getArgs("mysqluser","snort");
	connect->url = g_conf.getArgs("mysqlserver","192.168.0.18");
	DBMysql db;
	g_db_mysql->SetConnect(connect);
	g_db_mysql->OpenDB();	
	g_offset = g_db_mysql->GetMaxOffset(sql_select);
	//db.CloseDB();
}

int main(int argc,char*argv[])
{
	std::cout << "xxxx" << std::endl;
	el::Configurations conf("../conf/server_log.conf");
	el::Loggers::reconfigureAllLoggers(conf);
	g_conf.printArg();
	InitMaxOffset();
	InitKafka();
}
