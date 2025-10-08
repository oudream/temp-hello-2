#include <vtkSmartPointer.h>
#include <vtkITK/vtkITKArchetypeImageSeriesReader.h>
#include <vtkStringArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

struct Cmd {
    std::string pattern;   // e.g. D:\vol\%03d.bmp
    int z0=-1, z1=-1;      // inclusive
    std::string outVDI;    // out .vdi
    std::string outMHD;    // optional .mhd
    double spacing[3]{-1,-1,-1}; // if all <0, use image spacing or default 1
    std::string endianness = "little";
    int forceBits = 0;     // 0=auto, 8 or 16
};

static void die(const std::string& s){ std::cerr << "Error: " << s << std::endl; std::exit(1); }

static Cmd parseArgs(int argc, char** argv){
    Cmd c;
    for(int i=1;i<argc;i++){
        std::string a = argv[i];
        if(a=="-i" && i+1<argc) c.pattern = argv[++i];
        else if(a=="-z" && i+2<argc){ c.z0 = std::atoi(argv[++i]); c.z1 = std::atoi(argv[++i]); }
        else if(a=="-o" && i+1<argc) c.outVDI = argv[++i];
        else if(a=="--mhd" && i+1<argc) c.outMHD = argv[++i];
        else if(a=="--spacing" && i+3<argc){
            c.spacing[0]=std::atof(argv[++i]); c.spacing[1]=std::atof(argv[++i]); c.spacing[2]=std::atof(argv[++i]);
        }
        else if(a=="--little") c.endianness="little";
        else if(a=="--big")    c.endianness="big";
        else if(a=="--uint8")  c.forceBits=8;
        else if(a=="--uint16") c.forceBits=16;
        else if(a=="-h"||a=="--help"){
            std::cout <<
                      "Usage: bmpseries2vdi -i \"D:\\vol\\%03d.bmp\" -z 0 511 -o out.vdi [--spacing sx sy sz] [--mhd out.mhd] [--uint8|--uint16] [--little|--big]\n";
            std::exit(0);
        } else die("Unknown arg: "+a);
    }
    if(c.pattern.empty()) die("missing -i");
    if(c.z0<0 || c.z1<0 || c.z1<c.z0) die("invalid -z");
    if(c.outVDI.empty()) die("missing -o out.vdi");
    return c;
}

static std::string fmt(const std::string& pattern, int k){
    char buf[1024];
    std::snprintf(buf, sizeof(buf), pattern.c_str(), k);
    return std::string(buf);
}

static void swap16_inplace(uint16_t* p, size_t n){
    for(size_t i=0;i<n;i++){ uint16_t v=p[i]; p[i]=(uint16_t)((v>>8)|((v&0xFF)<<8)); }
}

static void writeVDI(const std::string& path, int W,int H,int D, int bits,
                     const double sp[3], const std::string& endian,
                     const void* voxels, size_t nbytes)
{
    std::ofstream ofs(path.c_str(), std::ios::binary);
    if(!ofs) die("cannot open output VDI: "+path);
    ofs << "VDI1\n";
    ofs << "width=" << W << "\n";
    ofs << "height=" << H << "\n";
    ofs << "depth=" << D << "\n";
    ofs << "component=1\n";
    ofs << "pixel_type=" << (bits==8?"uint8":"uint16") << "\n";
    ofs << "spacing=" << sp[0] << " " << sp[1] << " " << sp[2] << "\n";
    ofs << "endianness=" << endian << "\n";
    ofs << "DATA\n";
    ofs.write((const char*)voxels, std::streamsize(nbytes));
    if(!ofs) die("failed writing VDI payload");
}

static void writeMHD(const std::string& mhdPath, const std::string& rawPath,
                     int W,int H,int D, int bits, const double sp[3], const std::string& endian)
{
    std::ofstream ofs(mhdPath.c_str(), std::ios::binary);
    if(!ofs) die("cannot open output MHD: "+mhdPath);
    ofs << "ObjectType = Image\n";
    ofs << "NDims = 3\n";
    ofs << "DimSize = " << W << " " << H << " " << D << "\n";
    ofs << "ElementSpacing = " << sp[0] << " " << sp[1] << " " << sp[2] << "\n";
    ofs << "ElementType = " << (bits==8 ? "MET_UCHAR" : "MET_USHORT") << "\n";
    ofs << "ElementByteOrderMSB = " << (endian=="big"?"True":"False") << "\n";
    ofs << "BinaryData = True\n";
    ofs << "BinaryDataByteOrderMSB = " << (endian=="big"?"True":"False") << "\n";
    ofs << "CompressedData = False\n";
    ofs << "ElementDataFile = " << rawPath.substr(rawPath.find_last_of("/\\")+1) << "\n";
}

int main(int argc, char** argv){
    Cmd cmd = parseArgs(argc, argv);

    // 1) 组织文件名列表（Archetype = 第一张片）
    vtkNew<vtkStringArray> files;
    for(int z=cmd.z0; z<=cmd.z1; ++z){
        files->InsertNextValue(fmt(cmd.pattern, z));
    }
    if(files->GetNumberOfValues()==0) die("no input files");

    std::string archetype = files->GetValue(0);

    // 2) 配置 vtkITKArchetypeImageSeriesReader
    auto reader = vtkSmartPointer<vtkITKArchetypeImageSeriesReader>::New();
    reader->SetArchetype(archetype.c_str());
//    reader->SetFileNames(files);
    reader->SetUseNativeOriginOn();       // 保留原始 origin/spacing（若有）
    reader->SetOutputScalarTypeToNative(); // 按原图类型
    reader->SetSingleFile(0);             // 明确：多文件序列
    // 可选：reader->SetDesiredCoordinateOrientationToIdentity();

    try{
        reader->Update();
    }catch(...){
        die("reader->Update() failed (check files and ITK bridge build)");
    }

    vtkImageData* img = reader->GetOutput();
    if(!img) die("reader output is null");

    int ext[6];
    img->GetExtent(ext); // [xmin,xmax, ymin,ymax, zmin,zmax]
    const int W = ext[1]-ext[0]+1;
    const int H = ext[3]-ext[2]+1;
    const int D = ext[5]-ext[4]+1;

    double sp_img[3]; img->GetSpacing(sp_img);
    double spacing[3] = {1,1,1};
    if(cmd.spacing[0]>0 && cmd.spacing[1]>0 && cmd.spacing[2]>0){
        spacing[0]=cmd.spacing[0]; spacing[1]=cmd.spacing[1]; spacing[2]=cmd.spacing[2];
    }else{
        // 若 reader 有合适 spacing 就用之，否则 fallback 1
        if(sp_img[0]>0 && sp_img[1]>0 && sp_img[2]>0) {
            spacing[0]=sp_img[0]; spacing[1]=sp_img[1]; spacing[2]=sp_img[2];
        }
    }

    vtkDataArray* scalars = img->GetPointData()->GetScalars();
    if(!scalars) die("no scalars in image");

    int vtkType = scalars->GetDataType();
    int nComp   = scalars->GetNumberOfComponents();
    if(nComp!=1){
        die("only single-component images supported; please convert to grayscale first");
    }

    // 3) 把数据拷到连续缓冲（VTK 可能内部带对齐/ghost，不直接假设连续）
    size_t voxCount = static_cast<size_t>(W)*H*D;
    int outBits = cmd.forceBits ? cmd.forceBits : ((vtkType==VTK_UNSIGNED_SHORT||vtkType==VTK_SHORT)?16:8);

    std::vector<uint8_t>  vol8;
    std::vector<uint16_t> vol16;

    // 用 GetScalarPointer(i,j,k) 逐切片/逐行拷贝，保证顺序：Z 从小到大，行从上到下，列从左到右
    for(int k=ext[4]; k<=ext[5]; ++k){
        for(int j=ext[2]; j<=ext[3]; ++j){
            void* rowPtr = img->GetScalarPointer(ext[0], j, k);
            if(!rowPtr) die("GetScalarPointer returned null");

            if(outBits==8){
                if(vol8.empty()) vol8.resize(voxCount);
                if(vtkType==VTK_UNSIGNED_CHAR){
                    std::memcpy(&vol8[(size_t)(k-ext[4])*W*H + (size_t)(j-ext[2])*W],
                                rowPtr, (size_t)W * sizeof(uint8_t));
                }else if(vtkType==VTK_UNSIGNED_SHORT){
                    // 简单缩放 16->8（可替换为窗宽窗位或线性缩放）
                    auto* p16 = static_cast<uint16_t*>(rowPtr);
                    uint8_t* dst = &vol8[(size_t)(k-ext[4])*W*H + (size_t)(j-ext[2])*W];
                    for(int i=0;i<W;i++) dst[i] = static_cast<uint8_t>(p16[i] >> 8); // 取高字节
                }else{
                    die("unsupported VTK scalar type for --uint8 output");
                }
            }else{ // outBits == 16
                if(vol16.empty()) vol16.resize(voxCount);
                if(vtkType==VTK_UNSIGNED_SHORT){
                    std::memcpy(&vol16[(size_t)(k-ext[4])*W*H + (size_t)(j-ext[2])*W],
                                rowPtr, (size_t)W * sizeof(uint16_t));
                }else if(vtkType==VTK_UNSIGNED_CHAR){
                    // 8->16 复制并放大（0..255 -> 0..65535）
                    auto* p8 = static_cast<uint8_t*>(rowPtr);
                    uint16_t* dst = &vol16[(size_t)(k-ext[4])*W*H + (size_t)(j-ext[2])*W];
                    for(int i=0;i<W;i++) dst[i] = uint16_t(p8[i]) * 257;
                }else{
                    die("unsupported VTK scalar type for --uint16 output");
                }
            }
        }
    }

    // 4) 端序（仅 16 位关心）
    if(outBits==16 && cmd.endianness=="big"){
        swap16_inplace(vol16.data(), vol16.size());
    }

    // 5) 写 VDI
    if(outBits==8){
        writeVDI(cmd.outVDI, W,H,D, 8, spacing, cmd.endianness, vol8.data(), vol8.size());
    }else{
        writeVDI(cmd.outVDI, W,H,D, 16, spacing, cmd.endianness, vol16.data(), vol16.size()*sizeof(uint16_t));
    }
    std::cerr << "VDI saved: " << cmd.outVDI << "\n";

    // 6) 可选写 MHD+RAW
    if(!cmd.outMHD.empty()){
        std::string raw = cmd.outMHD;
        size_t p = raw.find_last_of('.');
        if(p==std::string::npos) raw += ".raw"; else { raw.erase(p); raw += ".raw"; }

        writeMHD(cmd.outMHD, raw, W,H,D, outBits, spacing, cmd.endianness);
        std::ofstream r(raw.c_str(), std::ios::binary);
        if(!r) die("cannot open raw: "+raw);
        if(outBits==8) r.write((const char*)vol8.data(), std::streamsize(vol8.size()));
        else           r.write((const char*)vol16.data(), std::streamsize(vol16.size()*sizeof(uint16_t)));
        if(!r) die("failed write raw");
        std::cerr << "MHD+RAW saved: " << cmd.outMHD << " , " << raw << "\n";
    }

    std::cerr << "Done. Volume = " << W << "x" << H << "x" << D << " (" << outBits << "b)\n";
    return 0;
}
