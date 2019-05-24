#!/bin/bash

MYSQL_HOST="localhost"
MYSQL_USER="root"
MYSQL_PASS="a"
MYSQL_DB_HISTORY="trade_history"

for i in `seq 0 99`
do
    echo "alter table order_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB_HISTORY -e "ALTER TABLE order_history_$i MODIFY COLUMN price DECIMAL(30,9);" 
done

for i in `seq 0 99`
do
    echo "alter table order_detail_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB_HISTORY -e "ALTER TABLE order_detail_$i MODIFY COLUMN price DECIMAL(30,9);"
done

for i in `seq 0 99`
do
    echo "alter table deal_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB_HISTORY -e "ALTER TABLE deal_history_$i MODIFY COLUMN price DECIMAL(30,9);"
done

