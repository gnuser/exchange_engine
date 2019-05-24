#include "init.h"
#include "config.h"
#include <boost/program_options.hpp>

static ConfManager g_conf;

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
    }
    notify(cmd_param_map); 
    
    if (cmd_param_map.count("help"))
    {
        return false;
    }

    g_conf.setConfPath(path_configure);
    g_conf.readConfigFile();
    g_conf.printArg();
    return true; 
}

bool InitKafkaProducer()
{
    std::string kafka_url = g_conf.getArgs("kafka_url", "47.99.82.55:9092");
    std::string producer_topic = g_conf.getArgs("kafka_topic", "rate");
    if (PRODUCER_INIT_SUCCESS != g_kafka_producer->init_kafka(0, kafka_url.c_str(), producer_topic.c_str()))  
        return false;
    
    return true; 
}

