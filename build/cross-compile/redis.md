
### build
```shell
git clone https://github.com/redis/redis.git
# 6.2.7 oranagra released this 27 Apr 2022
git checkout e6f6709
### build amd64
make
# make 32bit
#
mkdir -p /opt/deploy/amd64/redis/
cp src/redis-server /opt/deploy/amd64/redis/
cp src/redis-cli /opt/deploy/amd64/redis/
cp src/redis-check-aof /opt/deploy/amd64/redis/
cp src/redis-check-rdb /opt/deploy/amd64/redis/
cp src/redis-sentinel /opt/deploy/amd64/redis/
cp src/redis-benchmark /opt/deploy/amd64/redis/

# run
cd src
./redis-server
./redis-server /path/to/redis.conf

### build arm64
export PATH=$PATH:/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export RAINLIB=aarch64-linux-gnu-rainlib
export AR=aarch64-linux-gnu-ar
export LINK=aarch64-linux-gnu-g++
make MALLOC=libc
#
mkdir -p /opt/deploy/arm64/redis/
cp src/redis-server /opt/deploy/arm64/redis/
cp src/redis-cli /opt/deploy/arm64/redis/
cp src/redis-check-aof /opt/deploy/arm64/redis/
cp src/redis-check-rdb /opt/deploy/arm64/redis/
cp src/redis-sentinel /opt/deploy/arm64/redis/
cp src/redis-benchmark /opt/deploy/arm64/redis/

### build arm32
export PATH=$PATH:/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/
export CC=arm-linux-gnueabihf-gcc
export CXX=arm-linux-gnueabihf-g++
export LD=arm-linux-gnueabihf-ld
export RAINLIB=arm-linux-gnueabihf-rainlib
export AR=arm-linux-gnueabihf-ar
export LINK=arm-linux-gnueabihf-g++
make MALLOC=libc
#
mkdir -p /opt/deploy/arm32/redis/
cp src/redis-server /opt/deploy/arm32/redis/
cp src/redis-cli /opt/deploy/arm32/redis/
cp src/redis-check-aof /opt/deploy/arm32/redis/
cp src/redis-check-rdb /opt/deploy/arm32/redis/
cp src/redis-sentinel /opt/deploy/arm32/redis/
cp src/redis-benchmark /opt/deploy/arm32/redis/

```
