#!/bin/bash

I3DS_BIN_PATH=${PWD}

I3DS_BIN_EXE_NAME=tcpclient100

export LD_LIBRARY_PATH=${I3DS_BIN_PATH}:$LD_LIBRARY_PATH
export PATH=${I3DS_BIN_PATH}:${PATH}

nohup ${I3DS_BIN_PATH}/${I3DS_BIN_EXE_NAME} --remote-ip 192.168.91.221 --remote-port 7000 --client-count 100 --send-interval 1000 1>/dev/null 2>&1 &
