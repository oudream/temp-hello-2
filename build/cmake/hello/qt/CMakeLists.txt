
if (WIN32)
else()
    if(U_PROCESSOR MATCHES "armv7")
    else()
    endif ()
endif ()

# depend QT
add_subdirectory(tk_charge_testor)

add_subdirectory(tk_db_testor)
