### build
```shell
git clone https://github.com/eclipse/mosquitto.git
git clone https://github.com/troydhanson/uthash.git
cp uthash/src/* mosquitto/include
cd mosquitto
# v2.0.14: 17 Nov 2021 
git checkout fd0e398
# build amd64
mkdir cmake-build-local && cd cmake-build-local
cmake -DWITH_CJSON=no -DWITH_TLS=no -DWITH_BUNDLED_DEPS=no -DWITH_DOCS=no -DDOCUMENTATION=OFF ..
# build arm64
mkdir cmake-build-linaro64 &&cd cmake-build-linaro64
cmake -DWITH_CJSON=no -DWITH_TLS=no -DWITH_BUNDLED_DEPS=no -DWITH_DOCS=no -DDOCUMENTATION=OFF -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/aarch64/linaro/toolchainfile.cmake" ..
# build arm32
mkdir cmake-build-linaro32 &&cd cmake-build-linaro32
cmake -DWITH_CJSON=no -DWITH_TLS=no -DWITH_BUNDLED_DEPS=no -DWITH_DOCS=no -DDOCUMENTATION=OFF -D CMAKE_TOOLCHAIN_FILE="/opt/dev/hello_iec104/build/armv7/linaro/toolchainfile.cmake" ..
#
make -j 8
#
mkdir -p /opt/deploy/amd64/mosquitto/
cp src/mosquitto /opt/deploy/amd64/mosquitto/
cp client/mosquitto_pub /opt/deploy/amd64/mosquitto/
cp client/mosquitto_rr /opt/deploy/amd64/mosquitto/
cp client/mosquitto_sub /opt/deploy/amd64/mosquitto/
cp lib/libmosquitto.so.1 /opt/deploy/amd64/mosquitto/
```
