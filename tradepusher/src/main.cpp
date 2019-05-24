#include <iostream>
#include "init.h"

int main(int argc,char*argv[])
{
    if (!ParseCmd(argc, argv)) 
    {
        std::cout << "Parse cmd failed !" << std::endl;
        return 0;
    }

    if (!InitDB())
    {
        std::cout << "Init DB failed !" << std::endl;
        return 0;
    }

    InitUsers();

    if (!InitKafkaProducer())
    {
        std::cout << "Init kafka producer failed !" << std::endl;
        return 0;
    }

    if (!InitKafkaConsumer())
    {
        std::cout << "Init kafka consumer failed !" << std::endl;
        return 0;
    }

    return 0;
}
