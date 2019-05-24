#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <map>
#include <vector>
#include "json.hpp"
#include "producer.h"

using json = nlohmann::json;

class ConfManager
{
public:
    ConfManager(std::string path = "../conf/server_main.conf"):path_(path)
    {
    }
    ~ConfManager(){}
    std::string getArgs(const std::string& strArg, const std::string& strDefault);
    bool isArgSet(const std::string& strArg);
    void setConfPath(std::string path)
    {
        path_ = path;
    }
    void printArg();
    void readConfigFile();

private:
    void readConfJsonObj(const json js);
private:
    std::string path_;
    json conf_;
    std::map<std::string,std::string> mapArgs_;
};

extern std::unique_ptr<ConfManager> confptr;
extern std::vector<uint64_t> users;
extern ProducerKafka* g_kafka_producer;

#endif
