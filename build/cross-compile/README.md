
### Google 开源项目风格指南——中文版
> https://github.com/zh-google-styleguide/zh-google-styleguide
- C++ 风格指南
> https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/
- Go 语言编码规范中文版 - Uber
> https://github.com/xxjwxc/uber_go_guide_cn


### ccxx git commit
- 1.step first sync with remote : first --remote
```shell
# work ... ... ... ...  editing
# work ... ... ... ...  editing
# work ... ... ... ...  editing
cd 3rd/ccxx
git add .
git commit -m "i3svg sqlite3template"
git push origin HEAD:master
#************************
git submodule update --remote --recursive
git submodule update --init --recursive
```


### cross compile aarch64 linaro
- aarch64 linaro
```shell
#
### full
#
rm -rf /opt/dev/hello_iec104/cmake-build-linaro
mkdir /opt/dev/hello_iec104/cmake-build-linaro
cd /opt/dev/hello_iec104/cmake-build-linaro
export STAGING_DIR=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/aarch64-linux-gnu
#cmake -DCMAKE_BUILD_TYPE=Debug -m32 -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/linaro/toolchainfile.cmake" ..
cmake -D U_HELLO=TRUE -D U_DEPLOY_PATH="/opt/dev/hello_iec104/build/deploy-linaro" -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/linaro/toolchainfile.cmake" ..
cmake -D U_DEPLOY_PATH="/opt/dev/hello_iec104/build/deploy-linaro" -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/linaro/toolchainfile.cmake" ..
make -j 8
#  
### update 
#  
cd /opt/dev/hello_iec104/cmake-build-linaro
export STAGING_DIR=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/aarch64-linux-gnu
make -j 4
#
### scp
#
cd /opt/dev/hello_iec104/build/deploy-linaro
scp tk_mqtt_xintong root@tk177:/userdata/tk_mqtt_xintong/
#
### tar
#
cd /opt/dev/hello_iec104/build
rm -rf tk_mqtt_jiexi.tar.gz
tar zcvf tk_mqtt_jiexi.tar.gz deploy-linaro
```

### deploy aarch64 linaro - tk_matt_tongke
- tar tk_matt_tongke 注意 gwdb 要是发布版的
```shell
cd /opt/dev/hello_iec104/build/aarch64/linaro
rm -rf tk_mqtt_tongke 
mkdir tk_mqtt_tongke
rm -rf tk_mqtt_tongke.tar
cp /opt/dev/hello_iec104/build/deploy-linaro/tk_mqtt_tongke ./tk_mqtt_tongke/
#cp -r /userdata/gwdb/ ./tk_mqtt_tongke/
cp /opt/dev/hello_iec104/projects/tk_mqtt_tongke/readme.md ./tk_mqtt_tongke/
cp /opt/dev/hello_iec104/projects/tk_mqtt_tongke/S99_start_tk_mqtt_tongke ./tk_mqtt_tongke/
tar cvf tk_mqtt_tongke.tar tk_mqtt_tongke
rm -rf tk_mqtt_tongke 
```
```shell
scp /opt/dev/hello_iec104/build/deploy-linaro/tk_mqtt_tongke root@tk177:/userdata/tk_mqtt_tongke/

```

### deploy aarch64 linaro - tk_mqtt_roma
```shell
cd /opt/dev/hello_iec104/build/aarch64/linaro
rm -rf tk_mqtt_roma 
mkdir tk_mqtt_roma
rm -rf tk_mqtt_roma.tar
cp /opt/dev/hello_iec104/build/deploy-linaro/tk_mqtt_roma tk_mqtt_roma/
cp /opt/dev/hello_iec104/build/deploy-linaro/tk_mqtt_roma_demo tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libcjson.so.1.7.11 tk_mqtt_roma/libcjson.so.1
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libHWMQTT.so tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libpaho-mqtt3as.so.1.0 tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libsecurec.so tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libcrypto.so.1.1 tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/lib/aarch64/linaro/libssl.so.1.1 tk_mqtt_roma/
cp /opt/dev/hello_iec104/projects/tk_mqtt_roma/S99_start_tk_mqtt_roma tk_mqtt_roma/
tar cvf tk_mqtt_roma.tar tk_mqtt_roma runsv
rm -rf tk_mqtt_roma
##
##
cd /userdata && tar xvf tk_mqtt_roma.tar
cp /userdata/tk_mqtt_roma/S99_start_tk_mqtt_roma /etc/init.d/
cat /etc/init.d/S99_start_tk_mqtt_roma

```


### cross compile openwrt
- openwrt
```shell
# full
rm -rf /opt/dev/hello_iec104/cmake-build-openwrt
mkdir /opt/dev/hello_iec104/cmake-build-openwrt
#
cd /opt/dev/hello_iec104/cmake-build-openwrt
export STAGING_DIR=/opt/openwrt-gcc-8.3.0
#cmake -m32 -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/openwrt/toolchainfile.cmake" ..
cmake -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/openwrt/toolchainfile.cmake" ..
make -j 4
# 
cd /opt/dev/hello_iec104/cmake-build-openwrt
export STAGING_DIR=/opt/openwrt-gcc-8.3.0
rm -rf /opt/dev/hello_iec104/build/deploy-linaro/*
make -j 4
#
cd ../build
rm -rf tk_mqtt_jiexi.tar.gz
tar zcvf tk_mqtt_jiexi.tar.gz deploy-linaro
```


### cross compile armv7
- armv7
```shell
# full
rm -rf /opt/dev/hello_iec104/cmake-build-armv7
mkdir /opt/dev/hello_iec104/cmake-build-armv7
#
cd /opt/dev/hello_iec104/cmake-build-armv7
export STAGING_DIR=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf
cmake -m32 -D U_HELLO=TRUE -D U_DEPLOY_PATH="/opt/dev/hello_iec104/build/deploy-armv7" -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/armv7/linaro/toolchainfile.cmake" ..
make -j 4
# 
cd /opt/dev/hello_iec104/cmake-build-armv7
export STAGING_DIR=/opt/arm_linux_4.8
rm -rf /opt/dev/hello_iec104/build/deploy-armv7/*
make -j 4
```


### cross compile nuvoton
- nuvoton
```shell
# full
rm -rf /opt/dev/hello_iec104/cmake-build-nuvoton
mkdir /opt/dev/hello_iec104/cmake-build-nuvoton
#
cd /opt/dev/hello_iec104/cmake-build-nuvoton
export STAGING_DIR=/opt/arm_linux_4.8
cmake -m32 -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/nuvoton/toolchainfile.cmake" ..
make -j 4
# 
cd /opt/dev/hello_iec104/cmake-build-nuvoton
export STAGING_DIR=/opt/arm_linux_4.8
rm -rf /opt/dev/hello_iec104/build/deploy-nuvoton/*
make -j 4
```


### deploy
- tar tk5web
```shell
cd /opt/dev/hello_iec104/build/aarch64/linaro
rm -rf tk5web 
mkdir tk5web
rm -rf tk5web.tar
cp -r /opt/dev/hello_iec104/assets/ tk5web/
cp -r /opt/dev/hello_iec104/go/deploy/conf/ tk5web/
cp -r /opt/dev/hello_iec104/go/deploy/tk5web tk5web/
cp -r /opt/dev/hello_iec104/go/deploy/S99_start_tk5web tk5web/
tar cvf tk5web.tar tk5web runsv
rm -rf tk5web
##
##
cd /userdata && tar xvf tk5web.tar
cp /userdata/tk5web/S99_start_tk5web /etc/init.d/
cat /etc/init.d/S99_start_tk5web
```



### clion setting for linaro 
- CMake Options
```shell
-DU_PROCESSOR=aarch64 -DU_OS_NAME=linaro
-DU_PROCESSOR=armv7 -DU_OS_NAME=linaro
```
- environment
```shell
STAGING_DIR=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/aarch64-linux-gnu
```

### clion setting for openwrt
- CMake Options
```shell
-DU_PROCESSOR=aarch64 -DU_OS_NAME=openwrt
-DU_PROCESSOR=armv7 -DU_OS_NAME=openwrt
```
- environment
```shell
STAGING_DIR=/opt/openwrt-gcc-8.3.0
```

### clion setting for nuvoton
- CMake Options
```shell
-DU_PROCESSOR=aarch64 -DU_OS_NAME=nuvoton
-DU_PROCESSOR=armv7 -DU_OS_NAME=nuvoton
```
- environment
```shell
STAGING_DIR=/opt/arm_linux_4.8
```

### clion setting for QT
```text
-DU_QT=TRUE -DU_HELLO=TRUE
-DU_QT=TRUE -DU_HELLO=TRUE -DU_CPP_SEVENTEEN=TRUE
```


### 
- 蒲公英访问端
```text
用户名：39623294:003
密码是：syy20210417
网关是：10.168.1.170
```


### debug mqtt jiexi
- tk_mqtt_jiexi
```shell
--mqtt-ip "192.168.91.221" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_jiexi_b812345b --mqtt-app-name Tk_mqtt_jiexi --log-path "/var/log/tk_mqtt_jiexi.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_mqtt_jiexi --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_jiexi_b812345b --mqtt-app-name Tk_mqtt_jiexi --log-path "/userdata/dgri/tk/tk_mqtt_jiexi.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_mqtt_jiexi.log

# windows
--mqtt-ip "192.168.1.170" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_jiexi_b812345b --mqtt-app-name Tk_mqtt_jiexi --log-path "C:/dev/hello_iec104/build/deploy/tk_mqtt_jiexi.log" --db-path-format "C:/Users/Administrator/Desktop/tmp/gwdb/%s.db"
```
- tk_tool_jiexi_db
```shell
--mqtt-ip "192.168.91.221" --mqtt-port 1883 --mqtt-client-id Tk_tool_jiexi_db_b812345b --mqtt-app-name Tk_tool_jiexi_db --log-path "/var/log/tk_tool_jiexi_db.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_tool_jiexi_db --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_jiexi_b812345b --mqtt-app-name Tk_mqtt_jiexi --log-path "/userdata/dgri/tk/tk_tool_jiexi_db.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_tool_jiexi_db.log
```
- tk_test_gateway_client
```shell
--mqtt-ip "192.168.91.221" --mqtt-port 1883 --mqtt-client-id Tk_test_gateway_client_b812345b --mqtt-app-name Tk_test_gateway_client --log-path "/var/log/tk_test_gateway_client.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_tool_jiexi_db --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_jiexi_b812345b --mqtt-app-name Tk_mqtt_jiexi --log-path "/userdata/dgri/tk/tk_tool_jiexi_db.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_tool_jiexi_db.log
```


### debug mqtt tongke
- tk_mqtt_tongke
```shell
--mqtt-ip "8.129.8.199" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_tongke_b812345b --mqtt-app-name Tk_mqtt_tongke --log-path "/var/log/tk_mqtt_tongke.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_mqtt_tongke --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_tongke_b812345b --mqtt-app-name Tk_mqtt_tongke --log-path "/userdata/dgri/tk/tk_mqtt_tongke.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_mqtt_tongke.log

# windows
--mqtt-ip "192.168.1.170" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_tongke_b812345b --mqtt-app-name Tk_mqtt_tongke --log-path "C:/dev/hello_iec104/build/deploy/tk_mqtt_tongke.log" --db-path-format "C:/Users/Administrator/Desktop/tmp/gwdb/%s.db"
```
- tk_tool_tongke_db
```shell
--mqtt-ip "192.168.91.221" --mqtt-port 1883 --mqtt-client-id Tk_tool_tongke_db_b812345b --mqtt-app-name Tk_tool_tongke_db --log-path "/var/log/tk_tool_tongke_db.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_tool_tongke_db --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_tongke_b812345b --mqtt-app-name Tk_mqtt_tongke --log-path "/userdata/dgri/tk/tk_tool_tongke_db.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_tool_tongke_db.log
```
- tk_test_gateway_client
```shell
--mqtt-ip "192.168.91.221" --mqtt-port 1883 --mqtt-client-id Tk_test_gateway_client_b812345b --mqtt-app-name Tk_test_gateway_client --log-path "/var/log/tk_test_gateway_client.log" --db-path-format "/userdata/gwdb/%s.db"

# 192.168.1.170
cd /userdata/dgri/tk/deploy
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./tk_tool_tongke_db --mqtt-ip "127.0.0.1" --mqtt-port 1883 --mqtt-client-id Tk_mqtt_tongke_b812345b --mqtt-app-name Tk_mqtt_tongke --log-path "/userdata/dgri/tk/tk_tool_tongke_db.log" --db-path-format "/userdata/dgri/tk/gwdb/%s.db"
tail -f /userdata/dgri/tk/tk_tool_tongke_db.log
```


### bjht
```shell
# windows
-d C:/dev/hello_iec104/build/deploy --mysql-ip 192.168.91.221 --mysql-user-name root --mysql-password Aa.123456
-d C:/dev/hello_iec104/build/deploy --mysql-ip 127.0.0.1 --mysql-user-name root --mysql-password "" --mysql-port 3308

# linux
-d /opt/dev/hello_iec104/build/deploy --mysql-ip 192.168.91.221 --mysql-user-name root --mysql-password Aa.123456

```


### compile
```shell
mkdir cmake-build-centos && cd  cmake-build-centos 
cmake -D U_QT=TRUE ..
make -j 4

-DU_PROCESSOR=aarch64 -DU_OS_NAME=linaro -D U_HELLO=TRUE -D U_DEPLOY_PATH="/opt/dev/hello_iec104/build/deploy-linaro"
```


### Clion 如何让Jetbrains系列IDE代码自动排版功能的大括号换行？
- https://www.zhihu.com/question/59161105


### sqlite db
- myapp web video ai db
```text
#define DB_AI_FILEPATH "/userdata/gwdb/myapp.video.db"
```

- 清远 mqtt (golang) db
```text
	Mqtt31DatabaseName               = "gw_mqtt31"
```

- tongke ( jiexi ) mqtt (c++) db
```text
#define DB_NAME_GW_IOT "gw_iot"
```
