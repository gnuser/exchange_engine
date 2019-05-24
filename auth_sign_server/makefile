SRC=./src/server.cpp ./src/main.cpp  ./src/common.cpp  ./src/cdbparam.cpp
INCLUDE= -I./include  
LIB=-L./lib  -levent -lc -lrt -lcurl -lpthread 
APP= auth_sign_server
CFLAG=-std=c++11
DEBUG=-g
auth_sign_server:
	g++ $(CFLAG) $(DEBUG) $(SRC) $(INCLUDE) -o $(APP) $(LIB)  

clean:
	rm -rf $(APP)
