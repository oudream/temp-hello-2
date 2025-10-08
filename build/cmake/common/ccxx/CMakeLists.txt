
project(ccxx LANGUAGES CXX)

set(_HEADERS
        cxglobal.h
        cxassert.h
        cxcontainer.h
        cxstring.h
        cxstringc.h
        datetime.h
        cxthread.h
        cxsocket.h
        cxsharememory.h
        cxprocess.h
        cxprocmutex.h
        cxbuffer.h
        cxcrc.h
        cxmath.h
        cxpacket.h
        cxtinyxml.h
        cxarguments.h
        cxfilesystem.h
        configuration.h
)

set(_SRCS
        cxassert.cpp
        cxstring.cpp
        cxstringc.cpp
        datetime.cpp
        cxthread.cpp
        cxsocket.cpp
        cxsharememory.cpp
        cxprocess.cpp
        cxprocmutex.cpp
        cxbuffer.cpp
        cxcrc.cpp
        cxmath.cpp
        cxtinyxml.cpp
        tinyxml/tinyxml.cpp
        tinyxml/tinyxmlerror.cpp
        tinyxml/tinyxmlparser.cpp
        cxarguments.cpp
        cxfilesystem.cpp
)

#cxqtutil.cpp
#cxqtutil.h
#cxsqlite3template.hpp
#cxuuid.cpp
#cxuuid.h
#cxwinreg.cpp
#cxwinreg.h

mc_merge_file_path(${gs_project_ccxx_path} "${_HEADERS}" PROJECT_HEADERS)
mc_merge_file_path(${gs_project_ccxx_path} "${_SRCS}" PROJECT_SRCS)

# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC
        ${PROJECT_SRCS}
        #${PROJECT_HEADERS}
)

## ------------------------------------------------------------------------------
## Link libraries
## ------------------------------------------------------------------------------
#target_link_libraries(${PROJECT_NAME}
#        PUBLIC
#        ${VTK_LIBRARIES}
#        ${ITK_LIBRARIES}
#)

# 用法：print_unicode_state(<target>)
function(print_unicode_state tgt)
    get_target_property(t_defs  ${tgt} COMPILE_DEFINITIONS)           # 目标显式定义
    get_target_property(t_opts  ${tgt} COMPILE_OPTIONS)               # 目标编译选项
    get_directory_property(d_defs COMPILE_DEFINITIONS)                # 目录层面的定义

    message(STATUS "[${tgt}] COMPILE_DEFINITIONS: ${t_defs}")
    message(STATUS "[${tgt}] COMPILE_OPTIONS:    ${t_opts}")
    message(STATUS "[DIR ] COMPILE_DEFINITIONS: ${d_defs}")

    foreach(sym IN ITEMS UNICODE _UNICODE)
        set(found "NO")
        if(t_defs)
            list(FIND t_defs ${sym} idx1)
            if(NOT idx1 EQUAL -1)
                set(found "YES (target def)")
            endif()
        endif()
        if(found STREQUAL "NO" AND d_defs)
            list(FIND d_defs ${sym} idx2)
            if(NOT idx2 EQUAL -1)
                set(found "YES (dir def)")
            endif()
        endif()
        if(found STREQUAL "NO" AND t_opts)
            string(REGEX MATCH "(^|;) */D${sym}(;|$)" m "${t_opts}")      # MSVC
            if(m)
                set(found "YES (/D in options)")
            endif()
            string(REGEX MATCH "(^|;) *-D${sym}(;|$)" m2 "${t_opts}")     # GCC/Clang
            if(m2)
                set(found "YES (-D in options)")
            endif()
        endif()
        message(STATUS "[${tgt}] ${sym} defined? ${found}")
    endforeach()

    # （VS 生成器特有）如果你用 Visual Studio，可以顺便看“字符集”属性
    get_target_property(cs ${tgt} VS_GLOBAL_CharacterSet)
    if(cs)
        message(STATUS "[${tgt}] VS CharacterSet: ${cs}") # 一般为 Unicode 或 MultiByte
    endif()
endfunction()

# 示例：
# add_executable(app main.cpp)
# print_unicode_state(${PROJECT_NAME})