cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(cx-ct-x2 VERSION 1.0.1 LANGUAGES CXX C)

#cmake . -G "Visual Studio 10 2010" --build "C:\ddd\ccpp\gcl3\build\cmake" -B"C:\ddd\ccpp\gcl3\build\cmake\ccpro-vs2010"
#cmake . -G "Visual Studio 12 2013" --build "C:\ddd\ccpp\gcl3\build\cmake" -B"C:\ddd\ccpp\gcl3\build\cmake\ccpro-vs2013"
#cmake . -G "Visual Studio 14 2015" --build "C:\ddd\ccpp\gcl3\build\cmake" -B"C:\ddd\ccpp\gcl3\build\cmake\ccpro-vs2015"
#-DQT_QMAKE_EXECUTABLE:FILEPATH=F:/qt5.5.1mingw4.9/5.5/mingw492_32/bin/qmake.exe
#-DCMAKE_PREFIX_PATH:FILEPATH=F:/qt5.5.1mingw4.9/5.5/mingw492_32

include(./build/cmake/output.cmake)

add_subdirectory(./build/cmake)
