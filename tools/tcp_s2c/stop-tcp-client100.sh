#!/bin/bash

PID=$(ps -ef | grep tcpclient100 | grep -v grep | awk '{ print $2 }')
if [ -z "$PID" ]
then
    echo tcpclient100 is already stopped
else
    echo kill tcpclient100!
    kill -9 $PID
fi

