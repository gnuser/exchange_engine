#include <iostream>
#include "init.h"
#include "producer.h"
#include "config.h"
#include <vector>

#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

std::string BTC = "BTC";
std::string ETH = "ETH";
std::string USDT = "USDT";
std::string CNY = "CNY";

std::string url_usd = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/listings/latest?limit=10";
std::string url_cny = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/listings/latest?limit=10&convert=CNY";
std::string key_cf = "X-CMC_PRO_API_KEY: f1d06e3c-5bea-4735-a1d4-d957fadba5d7";
std::string key_hs = "X-CMC_PRO_API_KEY: 2ea5aa4c-fbf3-445c-85e7-f01df39cc87a";

size_t WriteToString(void *ptr, size_t size, size_t count, void *stream)
{   
    ((std::string*)stream)->append((char*)ptr, 0, size* count);
    return size* count;
}

bool CurlGet(const std::string& url, const std::string& key, std::string& response)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    CURLcode res;

    if (curl)
    {
        headers = curl_slist_append(headers, key.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    return true;
}

bool RequestCoinRate(std::map<std::string, double> &map_rate)
{
    std::string response;
    if (!CurlGet(url_usd, key_hs, response))
        return false;
        
    json json_res = json::parse(response);
    if (json_res.find("data") == json_res.end())
        return false;
    
    json data = json_res["data"];
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::string symbol = data.at(i)["symbol"].get<std::string>();
        if (BTC == symbol || ETH == symbol || USDT == symbol)
        {
            json quote = data.at(i)["quote"];
            map_rate[symbol] = quote["USD"]["price"].get<double>();
            std::cout << symbol << " : " << std::to_string(map_rate[symbol]) << std::endl;
        }
    }

    response.clear();
    if (!CurlGet(url_cny, key_hs, response))
        return false;

    json_res = json::parse(response);
    if (json_res.find("data") == json_res.end())
        return false;

    data = json_res["data"];
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::string symbol = data.at(i)["symbol"].get<std::string>();
        if (USDT == symbol)
        {
            json quote = data.at(i)["quote"];
            map_rate[CNY] = map_rate[USDT] / quote["CNY"]["price"].get<double>();
        }
    }

    return true;   
}

bool PushToKafka(const std::map<std::string, double> &map_rate)
{
    json json_kafka;
    for (auto it = map_rate.begin(); it != map_rate.end(); ++it)
        json_kafka[it->first] = it->second;

    std::string buffer = json_kafka.dump();
    std::cout << "kafka: " << buffer << std::endl;

    if (PUSH_DATA_SUCCESS != g_kafka_producer->push_data_to_kafka(buffer.c_str(), buffer.size()))
    {
        std::cout << "push kafka message fail." << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char*argv[])
{
    if (!ParseCmd(argc, argv)) 
    {
        std::cout << "Parse cmd fail" << std::endl;
        return 0;
    }

    if (!InitKafkaProducer())
    {
        std::cout << "Init kafka producer fail" << std::endl;
        return 0;
    }

    std::map<std::string, double> map_rate;
    map_rate[BTC] = 0.0;
    map_rate[ETH] = 0.0;
    map_rate[USDT] = 0.0;
    map_rate[CNY] = 0.0;
    
    while (true)
    {
        if (RequestCoinRate(map_rate))
            PushToKafka(map_rate);
        sleep(15 * 60);  
    } 
    return 0;
}

