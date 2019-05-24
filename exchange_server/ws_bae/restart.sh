#!/bin/bash

killall -s SIGQUIT ws_bae.exe
sleep 1
./ws_bae.exe config.json
