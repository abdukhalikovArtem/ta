#pragma once 
#include "../../../fcs.h"

#include "SingleScaleFilter\SingleScaleFilter.h"
#include "SolvedFilterGenerator\SolvedFilterGenerator.h"

class BlobDetector : private SingleScaleFilter, private SolvedFilterGenerator{

private:
	float a;

	double** rows;
	double** cols;
	
	int imgWidth;
	int imgHeight;
	int filterSizeMul;

	std::vector<float>   sigma;
	std::vector<double*> responses;
	std::vector<char*>	 binResp;
	std::vector<int>     filterSize;

	void GenerateFilters();

	void DisplayMajority();

	void CreateSegments();

	int dumpCounter;
	
public:
	BlobDetector(fcs::IInputProvider::VideoSettings& settings, std::vector<float> sigma, float filterSizeMul);

	~BlobDetector();

	fcs::TPrimary Process(const cv::Mat& input);
};
