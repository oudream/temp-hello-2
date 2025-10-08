@echo off
title stop TCP-Client100

tasklist | findstr /i tcpclient100.exe && taskkill /f /im tcpclient100.exe

echo tcpclient100.exe is off in %Date:~0,4%-%Date:~5,2%-%Date:~8,2% %Time:~0,2%:%Time:~3,2%
