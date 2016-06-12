#pragma once
#include "../../../fcs.h"

#include "opencv2\opencv.hpp"
#include "../../../common/OpenCLCore/OpenCLCore.h"

class SingleScaleFilter {
public:
	enum lmTypes { MAX, MIN, ALL };

private:
	int imgWidth;
	int imgHeight;

	int filterNumber; 

	double* row;
	double* col;
	
	enum Kernel { Buffer = 0, Response = 1, LocMax = 2 };
	cl_mem clSigmaNumber, clImg, clResponse, clBuffer, clLm, clRows, clCols, clFilterSize;
	
	double* buffer;
	
	unsigned char* lm;

	int nDim;
	int dimSize;
	int numThreads;
	size_t range[2];

	OpenCLCore* core;

	
	void CreateDeviceMemory(fcs::IInputProvider::VideoSettings& settings);
	void AssignKernelArguments();

	void WriteResponses();

	std::vector<fcs::TFeature> GeneratePoints(const cv::Mat& input);
	lmTypes type;

	void WriteImage(const cv::Mat& input);

	double GetMeanResponse(void);
	double GetMaxMajority(void);
	

	std::stringstream filterImg;
	int fileCounter;

protected:
	double* response;
	double* majority;
	double GetStdResp(void);
public:


	SingleScaleFilter(fcs::IInputProvider::VideoSettings& settings);
	~SingleScaleFilter();

	void SetFilter(double* col, double* row, size_t filterSize, int filterNumber);
	std::vector<fcs::TFeature
	> FindPoints(const cv::Mat& input);
};
