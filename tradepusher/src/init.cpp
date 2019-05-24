#include "init.h"
#include "common.h"
#include "easylogging++.h"
#include "db_mysql.h"
#include "consumer.h"
#include <boost/program_options.hpp>

static ConfManager g_conf;
std::vector<uint64_t> users;

INITIALIZE_EASYLOGGINGPP; 

bool ParseCmd(int argc,char*argv[])
{
    using namespace boost::program_options;
    std::string path_configure ; 
    std::string path_log;

    boost::program_options::options_description opts_desc("All options");
    opts_desc.add_options()
        ("help,h", "help info")
        ("log_path,l", value<std::string>(&path_log)->default_value("../conf/server_log.conf"), "log configure ")
        ("configure_path,c", value<std::string>(&path_configure)->default_value("../conf/server_main.conf"), "path configure ");

    variables_map cmd_param_map;
    try
    {
        store(parse_command_line(argc, argv, opts_desc), cmd_param_map);
    }
    catch(boost::program_options::error_with_no_option_name &ex)
    {
        std::cout << ex.what() << std::endl;
    }
    notify(cmd_param_map); 
    
    if (cmd_param_map.count("help"))
    {
        std::cout << opts_desc << std::endl;
        return false;
    }

    el::Configurations conf(path_log);
    el::Loggers::reconfigureAllLoggers(conf);

    g_conf.setConfPath(path_configure);
    g_conf.readConfigFile();
    g_conf.printArg();
    return true; 
}

bool InitDB()
{
    DBMysql::MysqlConnect* connect = new DBMysql::MysqlConnect();
    connect->url = g_conf.getArgs("mysqlserver", "127.0.0.1");
    connect->port = std::atoi(g_conf.getArgs("mysqlport", "3306").c_str());
    connect->user_name = g_conf.getArgs("mysqluser", "root");
    connect->user_pass = g_conf.getArgs("mysqlpass", "root890*()");
    connect->use_db = g_conf.getArgs("mysqldb", "tradepusher");
    
    g_db_mysql->SetConnect(connect);
    return g_db_mysql->OpenDB();
}

bool InitUsers()
{
    std::string user_ids = g_conf.getArgs("users", "[]");
    json js = json::parse(user_ids);
    for (int i = 0; i < js.size(); ++i)
        users.push_back(js.at(i));	
}

bool InitKafkaProducer()
{
    std::string kafka_to = g_conf.getArgs("kafka_to", "127.0.0.1:9092");
    std::string topic_to = g_conf.getArgs("topic_to", "trades");
    if (PRODUCER_INIT_SUCCESS != g_kafka_producer->init_kafka(0, kafka_to.c_str(), topic_to.c_str()))  
        return false;
    
    return true; 
}

bool InitKafkaConsumer()
{
    std::string kafka_from = g_conf.getArgs("kafka_from", "127.0.0.1:9092");
    std::string topic_from = g_conf.getArgs("topic_from", "deals");
    std::string group = g_conf.getArgs("groups", "1");

    std::string select_sql = "select offset from kafka_offset order by offset desc limit 1";

    DBMysql::JsonDataFormat json_format;
    json_format.column_size = 1;
    json_format.map_column_type[0] = DBMysql::INT;

    json json_data;
    bool ret = g_db_mysql->GetDataAsJson(select_sql, &json_format, json_data);

    if (!ret)
    {
        LOG(ERROR) << "Init kafka consumer fail.";
        return false;
    }
   
    uint64_t offset = 0;
    if (json_data.is_null())
    {
        offset = 0;
    }
    else 
    {   
        offset = json_data.at(0).at(0).get<int>() + 1;
    }

    std::cout << "start_offset: " << offset<< std::endl;

    std::shared_ptr<CConsumer> kafka_consumer_client_ = std::make_shared<CConsumer>(kafka_from, topic_from, group, offset);
    if (!kafka_consumer_client_->Init())
    {
        std::cout << "init kafka consumer failed " << std::endl;
        return false;
    } 
    else 
    {
        std::cout << "start kafka consumer.\n";
        kafka_consumer_client_->Consume(1000);
    }
    return true;
}

