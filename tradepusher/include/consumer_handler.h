#ifndef CONSUMER_HANDLER_H
#define CONSUMER_HANDLER_H

#include <string>

class ConsumerHandler
{
public:
	ConsumerHandler();
	~ConsumerHandler();

public:
	void UpdateDB(std::string msg_update,int offset);	

};

#endif
