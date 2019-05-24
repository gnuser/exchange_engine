TARGET  := ws_bae.exe
INCS = -I ../network -I ../utils
LIBS = -L ../utils -lutils -L ../network -lnetwork -Wl,-Bstatic -lev -ljansson -lmpdec -lrdkafka -lz -lssl -lcrypto -lhiredis  -Wl,-Bdynamic -lcurl -lm -lpthread -ldl -llz4 -lmysqlclient
include ../makefile.inc
