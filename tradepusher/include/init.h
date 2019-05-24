#ifndef INIT_H
#define INIT_H

#include <string>
#include "common.h"

bool ParseCmd(int argc, char*argv[]);
bool InitDB();
bool InitUsers();
bool InitKafkaProducer();
bool InitKafkaConsumer();

#endif
