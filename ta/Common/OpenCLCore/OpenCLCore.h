#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <string>
#include "CL\cl.h"





// Macros for OpenCL versions
#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f


class OpenCLCore {
	//utils
private:
	const static char* platform;
	cl_context       context;           // hold the context handler
	cl_device_id     device;            // hold the selected device handler
	cl_command_queue commandQueue;      // hold the commands-queue handler 
	cl_program       program;           // hold the program handler
	std::vector<cl_kernel>        kernel;            // hold the kernel handler
	float            platformVersion;   // hold the OpenCL platform version (default 1.2)
	float            deviceVersion;     // hold the OpenCL device version (default. 1.2)
	float            compilerVersion;   // hold the device OpenCL C version (default. 1.2)

	void LogError(const char* str, ...);
	void LogInfo(const char* str, ...);
	int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize);

protected:

	const char* TranslateOpenCLError(cl_int errorCode);
	bool CheckPreferredPlatformMatch(cl_platform_id platform, const char* preferredPlatform);
	cl_platform_id FindOpenCLPlatform(const char* preferredPlatform, cl_device_type deviceType);
	int GetPlatformAndDeviceVersion(cl_platform_id platformId);
	int SetupOpenCL(cl_device_type deviceType);
	int CreateAndBuildProgram(std::string src);


public:
	enum memoryFlags { READ_ONLY = CL_MEM_READ_ONLY, WRITE_ONLY = CL_MEM_WRITE_ONLY, READ_WRITE = CL_MEM_READ_WRITE };//USE_HOST_POINTER = CL_MEM_USE_HOST_PTR, ALLOC_HOST_POINTER = CL_MEM_ALLOC_HOST_PTR, COPY_HOST_PTR = CL_MEM_COPY_HOST_PTR 
	~OpenCLCore();
	OpenCLCore();
	void AddKernel(std::string progrmm, std::string kernel);
	cl_uint ExecuteKernel(int kerIndex, cl_uint dimention, size_t range[]);
	cl_mem CreateBuffer(cl_uint size, int flags);
	cl_uint SetKernelArguments(int kerIndex, int number, cl_mem *src);
	void ReadDataFromDevice(cl_mem srcMem, void* dst, cl_uint size);
	void WriteDataToDevice(cl_mem clDstMem, void* srcMem, cl_uint size);
	void ReleaseMemoryObject(cl_mem memory);
};
