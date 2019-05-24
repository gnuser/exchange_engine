#ifndef CONSUMER_HANDLER_H
#define CONSUMER_HANDLER_H
#include<string>
#include<vector>

class ConsumerHandler
{
public:
	ConsumerHandler();
	~ConsumerHandler();

public:
	void UpdateDB(std::string msg_update,int offset);	


private:
    std::vector<std::string> vect_sql_;
    uint64_t last_time_step_;
};



#endif
