@echo off
title stop TCP-S2C

tasklist | findstr /i tcp_s2c.exe && taskkill /f /im tcp_s2c.exe

echo tcp_s2c.exe is off in %Date:~0,4%-%Date:~5,2%-%Date:~8,2% %Time:~0,2%:%Time:~3,2%
