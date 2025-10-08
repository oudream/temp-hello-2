#!/bin/bash

PID=$(ps -ef | grep tcp_s2c | grep -v grep | awk '{ print $2 }')
if [ -z "$PID" ]
then
    echo tcp_s2c is already stopped
else
    echo kill tcp_s2c!
    kill -9 $PID
fi

