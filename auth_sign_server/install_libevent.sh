 #!/bin/bash
 git clone https://github.com/nmathewson/Libevent.git
 cd ./Libevent/
./autogen.sh
./configure
make
cd ../
cp -rf  ./Libevent/include/* ./include
cp -rf  ./Libevent/.libs/* ./lib
