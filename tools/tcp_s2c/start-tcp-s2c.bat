@echo off
title restart TCP-S2C

set BASE_DIR=%~dp0
%BASE_DIR:~0,2%

:again

tasklist|findstr /i "tcp_s2c.exe" > nul
if ERRORLEVEL 1 (
	echo TcpClient is off in %Date:~0,4%-%Date:~5,2%-%Date:~8,2% %Time:~0,2%:%Time:~3,2%
	start "" "tcp_s2c.exe --listen-port 7000 --remote-ip 192.168.91.171 --remote-port 10086"
)

cd /d %BASE_DIR%

choice /t 3 /d y /n > nul

goto again