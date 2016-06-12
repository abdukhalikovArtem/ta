#include "SingleScaleFilter.h"

using namespace cv;
using namespace std;
using namespace fcs;

void SingleScaleFilter::CreateDeviceMemory(IInputProvider::VideoSettings& input) {
	clSigmaNumber = core->CreateBuffer(1, OpenCLCore::READ_ONLY);

	clImg = core->CreateBuffer(input.imageHeight*input.imageWidth, OpenCLCore::READ_ONLY);
	clResponse = core->CreateBuffer(input.imageHeight*input.imageWidth * 8, OpenCLCore::READ_WRITE);
	clBuffer = core->CreateBuffer(input.imageHeight*input.imageWidth * 8, OpenCLCore::READ_WRITE);
	clFilterSize = core->CreateBuffer(2, OpenCLCore::READ_ONLY);

	clRows = core->CreateBuffer(input.imageWidth * input.imageWidth * 8, OpenCLCore::READ_ONLY);
	clCols = core->CreateBuffer(input.imageWidth * input.imageWidth * 8, OpenCLCore::READ_ONLY);

	clLm = core->CreateBuffer(input.imageHeight*input.imageWidth, OpenCLCore::WRITE_ONLY);
}

void SingleScaleFilter::AssignKernelArguments() {
	//Assign kernel arguments
	core->SetKernelArguments(Buffer, 0, &clSigmaNumber);
	core->SetKernelArguments(Buffer, 1, &clImg);
	core->SetKernelArguments(Buffer, 2, &clCols);
	core->SetKernelArguments(Buffer, 3, &clFilterSize);
	core->SetKernelArguments(Buffer, 4, &clBuffer);

	core->SetKernelArguments(Response, 0, &clSigmaNumber);
	core->SetKernelArguments(Response, 1, &clBuffer);
	core->SetKernelArguments(Response, 2, &clRows);
	core->SetKernelArguments(Response, 3, &clFilterSize);
	core->SetKernelArguments(Response, 4, &clResponse);

	core->SetKernelArguments(LocMax, 0, &clResponse);
	core->SetKernelArguments(LocMax, 1, &clLm);
}

SingleScaleFilter::SingleScaleFilter(IInputProvider::VideoSettings& input)  {
	this->type = SingleScaleFilter::MAX;
	this->imgWidth = input.imageWidth;
	this->imgHeight = input.imageHeight;
	
	core = new OpenCLCore();
	core->AddKernel("Primary//BlobDetector//CORE//CreateBuffer.cl", "Core");
	core->AddKernel("Primary//BlobDetector//CORE//CreateResponse.cl", "Core");
	core->AddKernel("Primary//BlobDetector//CORE//FindLM.cl", "Core");

	CreateDeviceMemory(input);
	AssignKernelArguments();

	//AllocateMemory

	this->response = new double[input.imageWidth*input.imageHeight];
	this->buffer = new double[input.imageWidth*input.imageHeight];
	this->majority = new double[input.imageWidth*input.imageHeight];

	this->lm = new unsigned char[input.imageWidth*input.imageHeight];
	this->row = new double[input.imageWidth * input.imageWidth];
	this->col = new double[input.imageWidth * input.imageWidth];
	
	//init 
	nDim = 2;
	range[0] = input.imageHeight;
	range[1] = input.imageWidth;
	
	//Folder for dump
	/*string mainPath = std::getenv("DUMP_DATA");
	filterImg << mainPath << "\\filters\\";*/
	fileCounter = 0;
}

void SingleScaleFilter::SetFilter(double* col, double* row, size_t filterSize, int filterNUmber) {
	this->filterNumber = filterNUmber;
	assert(filterSize <= imgWidth);
	assert(filterSize <= imgWidth);

	core->WriteDataToDevice(clRows, (void*)row, filterSize * filterNUmber * 8);
	core->WriteDataToDevice(clCols, (void*)col, filterSize * filterNUmber * 8);
	
	short filter_size = filterSize;
	core->WriteDataToDevice(clFilterSize, &filter_size, 2);
}

std::vector<TFeature> SingleScaleFilter::FindPoints(const Mat& input) {
	//TODO
	//ASsert when input is not grayscale

	core->WriteDataToDevice(clImg, input.data, input.rows*input.cols);
	for (char sigmaNumber = 0; sigmaNumber < filterNumber; sigmaNumber++) {
		core->WriteDataToDevice(clSigmaNumber, &sigmaNumber, 1);
		core->ExecuteKernel(Buffer, nDim, range);
		core->ExecuteKernel(Response, nDim, range);
	}
	
	core->ReadDataFromDevice(clResponse, response, input.rows*input.cols * 8);
	auto mean = this->GetMeanResponse();
	for (int i = 0; i < imgHeight * imgWidth; i++)
		majority[i] = fabs(response[i] - mean);

	core->WriteDataToDevice(clResponse, majority, input.rows*input.cols * 8);
	
	core->ExecuteKernel(LocMax, nDim, range);
	
	core->ReadDataFromDevice(clLm, lm, input.rows*input.cols);

	auto points = GeneratePoints(input);
	vector<TFeature> result;

	auto max = GetMaxMajority();
	for (auto a : points) {
		if ( (a.majority > 0.1 * max)  )
			result.push_back(a);
	}
	//DBG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//WriteImage(input);
	//WriteResponses();
	
	return result;
}

std::vector<TFeature> SingleScaleFilter::GeneratePoints(const Mat& input) {
	std::vector<TFeature> points;
	
	for (int i = 0; i < input.rows; i++) {
		for (int j = 0; j < input.cols; j++) {
			bool pass;
			pass = (this->type == SingleScaleFilter::MAX) ? ((int)lm[i * input.cols + j] == 255) :
				(this->type == SingleScaleFilter::MIN) ? ((int)lm[i * input.cols + j] == 0) : ((int)lm[i * input.cols + j] != 127);
			
			if (pass) {
				TFeature point;
				point.x = j;
				point.y = i;
				point.response = (response[i * input.cols + j]);
				point.majority = majority[i * input.cols + j];
				points.push_back(point);
			}
		}
	}

	return points;
}

double SingleScaleFilter::GetMeanResponse(void) {
	double mean = 0;
	for (int i = 0; i < imgWidth * imgHeight; i++) {
		mean += response[i];
	}
	mean /= imgWidth * imgHeight;
	return mean;
};

double SingleScaleFilter::GetMaxMajority(void) {
	double max = majority[0];
	for (int i = 1; i < imgWidth * imgHeight; i++) 
		max = (max < majority[i])? majority[i] : max;
	return max;
}

double SingleScaleFilter::GetStdResp(void) {
	double mean = GetMeanResponse();
	double sum = 0;

	for (int i = 0; i < imgWidth * imgHeight; i++)
		sum += (response[i] - mean) * (response[i] - mean);
	return sum / (imgWidth * imgHeight);
}

void SingleScaleFilter::WriteImage(const Mat& input) {

	stringstream tmp;
	tmp << filterImg.str();
	tmp << "img" << "_counter_" << fileCounter << ".png";
	std::string s = tmp.str();
	imwrite(s, input);

}

void SingleScaleFilter::WriteResponses() {
	//WriteImage(input);

	core->ReadDataFromDevice(clResponse, response, imgWidth * imgHeight * 8);
	Mat Resp = Mat(imgHeight, imgWidth, CV_64F, response);
	
	double min, max;
	cv::minMaxLoc(Resp, &min, &max);
	cout << "[Response] min " << min << " max " << max << endl;;

	Mat draw = Mat(imgHeight, imgWidth, CV_8U);

	for (int i = 0; i < imgHeight; i++)
		for (int j = 0; j < imgWidth; j++)
			draw.at<uchar>(i, j) = (Resp.at<double>(i, j) - min) * 255 / fabs(max);

	stringstream tmp;
	tmp << filterImg.str();
	tmp << "response" << "_counter_" << fileCounter << ".png";
	std::string s = tmp.str();
	imwrite(s, draw);
	
	namedWindow("Response", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Response", draw);                   // Show our image inside it.
	waitKey(1);
	fileCounter++;
};

SingleScaleFilter::~SingleScaleFilter() {
	delete[] row;
	delete[] col;
	delete[] response;
	delete[] lm;
	delete[] buffer;
	delete[] majority;

	core->ReleaseMemoryObject(clImg);
	core->ReleaseMemoryObject(clSigmaNumber);
	core->ReleaseMemoryObject(clRows);
	core->ReleaseMemoryObject(clCols);
	core->ReleaseMemoryObject(clFilterSize);
	core->ReleaseMemoryObject(clResponse);
	core->ReleaseMemoryObject(clLm);
}

