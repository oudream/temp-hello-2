#!/bin/bash


echo "--- --- --- --- --- --- --- --- --- --- ---"
echo "tcp_s2c PIDS:"

PID=$(ps -ef | grep "tcp_s2c" | grep -v grep | awk '{ print $2 }')
echo "${PID}"


echo "--- --- --- --- --- --- --- --- --- --- ---"
echo "tcpclient100 PIDS:"

PID=$(ps -ef | grep "tcpclient100" | grep -v grep | awk '{ print $2 }')
echo "${PID}"

