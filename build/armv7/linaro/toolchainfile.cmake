set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# user
set(U_PROCESSOR armv7)
set(U_OS_NAME linaro)

# specify the cross compiler
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
SET(CMAKE_AR arm-linux-gnueabihf-gcc-ar CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB arm-linux-gnueabihf-gcc-ranlib)

# TOOL CHAIN DIR
set(TOOL_CHAIN_DIR /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf)
set(TOOL_CHAIN_NAME_PREFIX arm-linux-gnueabihf-)

set(CMAKE_C_COMPILER ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}g++)
set(CMAKE_AR ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}gcc-ar)
set(CMAKE_LINKER ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}ld)
set(CMAKE_RANLIB ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}gcc-ranlib)
set(CMAKE_NM ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}gcc-nm)
set(CMAKE_OBJDUMP ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}objdump)
set(CMAKE_OBJCOPY ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}objcopy)
set(CMAKE_STRIP ${TOOL_CHAIN_DIR}/bin/${TOOL_CHAIN_NAME_PREFIX}strip)

# where is the target environment
#set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf)
#set(CMAKE_SYSROOT /usr/arm-linux-gnueabihf)
set(CMAKE_SYSROOT ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/libc)

# include lib
set(TOOL_CHAIN_INCLUDE ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/include ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/libc/include ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/libc/usr/include)
set(TOOL_CHAIN_LIB ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/lib ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/libc/lib ${TOOL_CHAIN_DIR}/arm-linux-gnueabihf/libc/usr/lib)
#include_directories(${TOOL_CHAIN_INCLUDE})
set(CMAKE_INCLUDE_PATH ${TOOL_CHAIN_INCLUDE})
set(CMAKE_LIBRARY_PATH ${TOOL_CHAIN_LIB})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
