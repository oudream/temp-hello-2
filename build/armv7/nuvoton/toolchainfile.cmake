set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# user
set(U_PROCESSOR armv7)
set(U_OS_NAME nuvoton)
set(U_TOOL_ROOT_PATH /opt/arm_linux_4.8)
set(U_TOOL_PREFIX arm-nuvoton-linux-uclibceabi-)

# specify the cross compiler

set(CMAKE_C_COMPILER ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}g++)
set(CMAKE_AR ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}ar CACHE FILEPATH "Archiver")
set(CMAKE_LINKER ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}ld)
set(CMAKE_RANLIB ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}ranlib)
set(CMAKE_NM ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}nm)
set(CMAKE_OBJDUMP ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}objdump)
set(CMAKE_OBJCOPY ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}objcopy)
set(CMAKE_STRIP ${U_TOOL_ROOT_PATH}/bin/${U_TOOL_PREFIX}strip)

# where is the target environment
set(CMAKE_FIND_ROOT_PATH ${U_TOOL_ROOT_PATH})
set(CMAKE_SYSROOT ${U_TOOL_ROOT_PATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# automatically use the cross-wrapper for pkg-config
# set(PKG_CONFIG_EXECUTABLE "/somewhere/bin/aarch64-unknown-linux-gnueabi-pkg-config" CACHE FILEPATH "pkg-config executable")

# Set the {C,CXX}FLAGS appended by CMake depending on the build type
# defined by Buildroot. CMake defaults these variables with -g and/or
# -O options, and they are appended at the end of the argument list,
# so the Buildroot options are overridden. Therefore these variables
# have to be cleared, so that the options passed in CMAKE_C_FLAGS do
# apply.
#
# Note:
#   if the project forces some of these flag variables, Buildroot is
#   screwed up and there is nothing Buildroot can do about that :(
set(CMAKE_C_FLAGS_DEBUG "" CACHE STRING "Debug CFLAGS")
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "Debug CXXFLAGS")
set(CMAKE_C_FLAGS_RELEASE " -DNDEBUG" CACHE STRING "Release CFLAGS")
set(CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG" CACHE STRING "Release CXXFLAGS")

# Build type from the Buildroot configuration
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Buildroot build configuration")
#set(CMAKE_BUILD_TYPE Release CACHE STRING "Buildroot build configuration")

# Buildroot defaults flags.
# If you are using this toolchainfile.cmake file outside of Buildroot and
# want to customize the compiler/linker flags, then:
# * set them all on the cmake command line, e.g.:
#     cmake -DCMAKE_C_FLAGS="-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Os -Dsome_custom_flag" ...
# * and make sure the project's CMake code extends them like this if needed:
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dsome_definitions")
### big file
#set(CMAKE_C_FLAGS "-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Os" CACHE STRING "Buildroot CFLAGS")
#set(CMAKE_CXX_FLAGS "-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Os" CACHE STRING "Buildroot CXXFLAGS")
#set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "Buildroot LDFLAGS for executables")
#
#set(CMAKE_INSTALL_SO_NO_EXE 0)
include_directories(${U_TOOL_ROOT_PATH}/include)
