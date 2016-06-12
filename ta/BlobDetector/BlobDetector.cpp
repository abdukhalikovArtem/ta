#include "BlobDetector.h"


#include <windows.h> 

using namespace fcs;
using namespace cv;

void BlobDetector::GenerateFilters() {
	
	for (int i = 0; i < sigma.size(); i++) {
		LoGSettings settings{ sigma[i], filterSize[i] };
		filter f = GenerateLogDilter(settings);
		for (int k = 0; k < 2; k++)
			for (int j = 0; j < filterSize[i]; j++) {
				rows[i][k * filterSize[i] + j] = f.drow[k][j];
				cols[i][k * filterSize[i] + j] = f.dcol[k][j];
			}
	}
}

BlobDetector::BlobDetector(IInputProvider::VideoSettings& settings, std::vector<float> sigma, float filterSizeMul) : SingleScaleFilter(settings) {
	this->filterSizeMul = filterSizeMul;

	this->imgHeight = settings.imageHeight;
	this->imgWidth = settings.imageWidth;

	this->sigma = sigma;
	for (auto& a : sigma) {
		int size = a * filterSizeMul;
		size = (size % 2 == 0) ? size + 1 : size;
		filterSize.push_back(size);
	}
	
	rows = new double*[filterSize.size()];
	cols = new double*[filterSize.size()];

	for (int i = 0; i < filterSize.size(); i++) {
		rows[i] = new double[filterSize[i] * 2];
		cols[i] = new double[filterSize[i] * 2];
	}	

	for (int i = 0; i < sigma.size(); i++) {
		double* buffer = new double[imgHeight * imgWidth];
		char* binaryBuf = new char[imgHeight * imgWidth];
		responses.push_back(buffer);
		binResp.push_back(binaryBuf);
	}

	GenerateFilters();

	dumpCounter = 0;
}

BlobDetector::~BlobDetector() {
	
   	for (int i = 0; i < filterSize.size(); i++) {
		delete[] rows[i];
		delete[] cols[i];
	}
	delete[] rows;
	delete[] cols;

	for (auto& a : responses) 
		delete[] a;	

	for (auto& a : binResp)
		delete[] a;
}

void BlobDetector::DisplayMajority() {

	for (int i = 0; i < filterSize.size(); i++) {

		Mat Resp = Mat(imgHeight, imgWidth, CV_64F, responses[i]);
		Mat nom;
		double min, max;
		cv::minMaxLoc(Resp, &min, &max);
		std::cout << " Min " << min << " max " << max << std::endl;

		Mat draw = Mat(imgHeight, imgWidth, CV_8U);
		for (int i = 0; i < imgHeight; i++)
			for (int j = 0; j < imgWidth; j++) {
				double pixel = (Resp.at<double>(i, j) - min) * 255 / (max - min);
				draw.at<uchar>(i, j) = pixel;
			}

		std::stringstream ss;
		ss << "Response_" << i;

		namedWindow(ss.str(), WINDOW_AUTOSIZE);// Create a window for display.
		imshow(ss.str(), draw);                   // Show our image inside it.
		waitKey(1);

		std::stringstream tmp;
		string mainPath = std::getenv("DUMP_DATA");
		tmp << mainPath << "\\filters\\";
		tmp << "response_" << i << "_counter_" << dumpCounter << ".png";
		std::string s = tmp.str();
		imwrite(s, draw);
		
	}
	dumpCounter++;
};

void BlobDetector::CreateSegments() {
	
	for (int k = 0; k < responses.size(); k++) {
		Mat Resp(imgHeight, imgWidth, CV_64F, responses[k]);
		
		Mat nom;
		double min, max;
		cv::minMaxLoc(Resp, &min, &max);

		Mat binary(imgHeight, imgWidth, CV_8U, binResp[k]);

		for (int i = 0; i < imgHeight; i++)
			for (int j = 0; j < imgWidth; j++)
				binary.at<uchar>(i, j) = (Resp.at<double>(i, j) > 0.4 * max) ? 255 : 0;

		//cv::threshold(draw, binary, 0, 1, CV_THRESH_BINARY | CV_THRESH_OTSU);
		//cv::threshold(draw, binary, 0.3 * 255, 1, CV_THRESH_BINARY);		
	}

	return;
}

fcs::TPrimary BlobDetector::Process(const cv::Mat& input) {
	
	std::vector<TFeature> points;
	
	for (int i = 0; i < filterSize.size(); i++) {
		SetFilter(cols[i], rows[i], filterSize[i], 2);
		
		auto ssPoints = FindPoints(input);

		double norm = sigma[i] * sigma[i];
		 
 
		memcpy(responses[i], majority, imgHeight*imgWidth * 8);
		std::for_each(ssPoints.begin(), ssPoints.end(), [=](fcs::TFeature& p) {p.type = p.LoG;  p.size = norm; p.response *= norm; p.majority *= norm; p.aluID = i; p.size = sigma[i]; p.type = p.LoG; });
		points.insert(points.end(), ssPoints.begin(), ssPoints.end());
	}

	CreateSegments();
	//DisplayMajority();
	
	fcs::TPrimary result;
	result.feat = points;

	for (int i = 0; i < filterSize.size(); i++) {
		TBin buffer;
		buffer.filterSize = sigma[i];
		buffer.bin = binResp[i];
		result.segm.push_back(buffer);
	}

	return result;
}

