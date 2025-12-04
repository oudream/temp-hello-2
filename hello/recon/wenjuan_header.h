#pragma once
#ifndef header_h_
#define header_h_


#include <iostream>
#include <omp.h>
//#include <sys/time.h>
#include <string.h>
#include <vector>
using namespace std;

struct CLDimensions3D {
	unsigned int iVolX;//重建体素X
	unsigned int iVolY;//重建体素Y
	unsigned int iVolZ;//重建体素Z
	unsigned int iProjAngles;//采图张数
	unsigned int iProjU; // 探测器长（列）（像素）
	unsigned int iProjV; //探测器宽（行）（像素）
	float piexlsize = 0.0495*2;//单个像素尺寸0.0495
	float voxel_size = 0.1;//重建体素尺寸
	float circleRs = 46.735;//光源轨迹半径
	float circleRd = 184.602;//探测器轨道半径
	float sid = 68.755;//光源到物体的垂直距离
	float sdd = 206.145;//光源到探测器的垂直距离

	//迭代重建需要增加的参数
	int iter = 100;//迭代次数
	float MaxConstraint = 1.0;//迭代参数
	float MinConstraint = 0.0;

	//矫正参数
	int offsetU = 0;//探测器列方向偏移--像素为单位
	int offsetV = 0;////探测器行方向偏移--像素为单位
	float tiltU = 0;//探测器绕U方向倾斜--角度为单位
	float tiltV = 0;//探测器绕V方向倾斜--角度为单位
	float tiltZ = 0;//探测器平面内旋转--角度为单位

	int type = 1;//高级平面CT，0-倾斜圆锥束
	int saveslice = 1;//1保存，0不保存
	int otherpara = 0;//是否加载其他参数


};
struct inputdata
{
	vector<uint16_t*> VecCpuIndata;
	vector<int>vecNum;//每个内存的图片数量
	int W;
};

#define IN
#define OUT
//小于4G的数据申请1个内存
__declspec(dllexport) bool maincuCGLS(IN uint16_t* cpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);
__declspec(dllexport) bool maincusirt(IN uint16_t* cpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);
__declspec(dllexport) bool maincufdk(IN uint16_t* cpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);


//超过4G的数据申请多个内存
__declspec(dllexport) bool maincuCGLS(IN inputdata veccpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);
__declspec(dllexport) bool maincusirt(IN inputdata veccpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);
__declspec(dllexport) bool maincufdk(IN inputdata veccpuIndata, OUT float** cupOutdata, IN CLDimensions3D CLdims, IN int gpuindx);



#endif