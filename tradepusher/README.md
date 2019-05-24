# Compile and Install

Requirements

curl 
mysqlclient
boost
event
gmp         https://gmplib.org/download/gmp/gmp-6.1.2.tar.lz  ./configure --enable-cxx
librdkafka  https://github.com/edenhill/librdkafka
cppkafka    https://github.com/mfontanini/cppkafka

First, init you database, the sql is in "doc/asset.sql", the database must be mysql.

Second, congfigure your basic setting in "conf/server_log.conf" and "conf/server_main.conf".

cmake & make 
