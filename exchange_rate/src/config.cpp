#include <iostream>
#include <string>
#include <fstream>
#include "config.h"

std::unique_ptr<ConfManager> confptr(new ConfManager);
ProducerKafka* g_kafka_producer = new ProducerKafka;

std::string ConfManager::getArgs(const std::string& strArg, const std::string& strDefault)
{
    if(mapArgs_.count(strArg))
    {
        return mapArgs_[strArg];
    }
    return strDefault;
}

void ConfManager::readConfigFile()
{
    std::ifstream jfile(path_);
    if (!jfile)
	return;

    jfile >> conf_;
    if(!conf_.is_object())
    {
        conf_.clear();
        return ;
    }
    readConfJsonObj(conf_);
    conf_.clear();
}

void ConfManager::readConfJsonObj(const json js)
{
    try
    {
        auto it = js.begin();
        auto end = js.end();
        for (it; it !=end; ++it)
        {
            std::string key = it.key();
            auto val = it.value();
            if(val.is_object())
            {
                readConfJsonObj(val);
            }
            else if(val.is_string())
            {
                mapArgs_[it.key()] = val;
            }
        }
    }
    catch(...)
    {
        printArg();
        conf_.clear();
    }

}

void ConfManager::printArg()
{
    for(auto &it : mapArgs_)
        std::cout << it.first << "  :  " << it.second << std::endl;
}

