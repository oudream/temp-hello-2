
### MCT平台
- CT软件平台分两个系列软件：ECT、MCT。ECT系列(C#)：校准、运控、采图、2D检测、存结果、报告、在线对接；MCT(C++)：重建、看图、切面、试检、分割、配准、转换

### 0、预先要备好
- Cuda 版本应该没要求，但使用这版编译过：\\192.168.130.32\cyg\14-长园创新研究院\软件\NVIDIA-cuda-cudnn\cuda_12.6.3_561.17_windows.exe
- VisualStudio：\\192.168.130.32\cyg\14-长园创新研究院\工控机必装-全复制到D盘\安装软件-安装才用\VisualStudioSetup.exe
- Clion，要配置好编译工具链：\\192.168.130.32\cyg\14-长园创新研究院\软件\日常软件\Jetbrains-Clion-Idea\CLion-2023.3.4.exe
![README-Clion-编译工具链.png](README-Clion-%B1%E0%D2%EB%B9%A4%BE%DF%C1%B4.png)



- Developer Command Prompt for VS 2022
```shell
cd /d D:/ct

git clone https://github.com/microsoft/vcpkg.git

cd vcpkg && git checkout 2025.08.27

bootstrap-vcpkg.bat

set "VCPKG_ROOT=D:\ct\vcpkg"
set PATH=%VCPKG_ROOT%;%PATH%

# not cuda
vcpkg install qtbase:x64-windows qtdeclarative:x64-windows qttools:x64-windows qtsvg:x64-windows qtimageformats:x64-windows qtserialport:x64-windows qtserialbus:x64-windows qtnetworkauth:x64-windows qtmqtt:x64-windows qtmultimedia[ffmpeg,widgets]:x64-windows qt5compat:x64-windows qtcharts:x64-windows qt3d:x64-windows opencv[world,dnn]:x64-windows vtk[qt,opengl]:x64-windows itk[rtk,tools,vtk,opencv]:x64-windows gtest:x64-windows benchmark:x64-windows yaml-cpp:x64-windows nlohmann-json:x64-windows cpp-httplib[openssl,zlib]:x64-windows spdlog:x64-windows sqlite3:x64-windows mysql-connector-cpp:x64-windows

# has cuda
vcpkg install --no-binarycaching qtbase:x64-windows qtdeclarative:x64-windows qttools:x64-windows qtsvg:x64-windows qtimageformats:x64-windows qtserialport:x64-windows qtserialbus:x64-windows qtnetworkauth:x64-windows qtmqtt:x64-windows qtmultimedia[ffmpeg,widgets]:x64-windows qt5compat:x64-windows qtcharts:x64-windows qt3d:x64-windows opencv[world,dnn,cuda,tbb]:x64-windows vtk[qt,opengl,cuda,tbb]:x64-windows itk[rtk,tools,vtk,opencv,cuda,tbb]:x64-windows gtest:x64-windows benchmark:x64-windows yaml-cpp:x64-windows nlohmann-json:x64-windows cpp-httplib[openssl,zlib]:x64-windows spdlog:x64-windows sqlite3:x64-windows mysql-connector-cpp:x64-windows

qt-online-installer-windows-x64-4.10.0.exe --mirror https://mirrors.ustc.edu.cn/qtproject
```

- Windows环境变量
```shell
PATH
D:\ct\vcpkg\installed\x64-windows\debug\bin

QML_IMPORT_PATH
D:\ct\vcpkg\installed\x64-windows\debug\Qt6\qml

QT_PLUGIN_PATH
D:\ct\vcpkg\installed\x64-windows\debug\Qt6\plugins

QT_QPA_PLATFORM_PLUGIN_PATH
D:\ct\vcpkg\installed\x64-windows\debug\Qt6\plugins\platforms
```

- 下载
```shell
\\192.168.130.32\cyg\14-长园创新研究院\CT\vcpkg-工控机2.zip
```
### 3、下载本代码
```shell
git clone http://iagit.cygia.com/CYGIA/Research-Institute/Software/cx-ct-x2.git
```


### 4、配置 CMakePresets.json 中相关路径
-DCMAKE_TOOLCHAIN_FILE=D:/ct/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_APPLOCAL_DEPS=OFF

### Google 开源项目风格指南——中文版
> https://github.com/zh-google-styleguide/zh-google-styleguide
- C++ 风格指南
> https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/
- Go 语言编码规范中文版 - Uber
> https://github.com/xxjwxc/uber_go_guide_cn


### VTK build with qt
- https://github.com/Kitware/VTK/blob/master/Documentation/docs/build_instructions/build.md
```shell
#生成项目
cmake -S D:/ct/VTK -B D:/ct/VTK/build-qt ^
 -G "Visual Studio 17 2022" -A x64 -T v143 ^
 -DVTK_BUILD_TESTING=OFF -DVTK_BUILD_EXAMPLES=OFF ^
 -DVTK_GROUP_ENABLE_Qt=YES ^
 -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES ^
 -DVTK_MODULE_ENABLE_VTK_ViewsQt=YES ^
 -DCMAKE_PREFIX_PATH="D:/ct/vcpkg/installed/x64-windows/share/Qt6"

#编译并安装 Debug
cmake --build D:/ct/VTK/build-qt --config Debug --target ALL_BUILD

cmake --install D:/ct/VTK/build-qt --config Debug --prefix D:/ct/VTK/install-qt


certutil -hashfile v2.7.0.tar.gz SHA512
```


