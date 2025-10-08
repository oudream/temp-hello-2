#!/bin/bash

I3DS_BIN_PATH=${PWD}

I3DS_BIN_EXE_NAME=tcp_s2c

export LD_LIBRARY_PATH=${I3DS_BIN_PATH}:$LD_LIBRARY_PATH
export PATH=${I3DS_BIN_PATH}:${PATH}

nohup ${I3DS_BIN_PATH}/${I3DS_BIN_EXE_NAME} --listen-port 7000 --remote-ip 192.168.91.171 --remote-port 10086 1>/dev/null 2>&1 &
