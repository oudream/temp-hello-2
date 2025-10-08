
if (MSVC)
    add_definitions(-D"_CRT_SECURE_NO_WARNINGS")
    add_definitions( "/wd4819" )
    add_compile_options(/EHsc)
endif ()

add_subdirectory(stl)

add_subdirectory(ccxx)
