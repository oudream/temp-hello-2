@echo off
title restart Tcp-Client100

set BASE_DIR=%~dp0
%BASE_DIR:~0,2%

:again

tasklist|findstr /i "tcpclient100.exe" > nul
if ERRORLEVEL 1 (
	echo TcpClient is off in %Date:~0,4%-%Date:~5,2%-%Date:~8,2% %Time:~0,2%:%Time:~3,2%
	start "" "tcpclient100.exe --remote-ip 192.168.91.221 --remote-port 7000 --client-count 100 --send-interval 1000"
)

cd /d %BASE_DIR%

choice /t 3 /d y /n > nul

goto again