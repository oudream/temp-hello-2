
title Get Pids

tasklist /v /fo csv | findstr /i "tcp_s2c.exe"

tasklist /v /fo csv | findstr /i "tcpclient100.exe"
