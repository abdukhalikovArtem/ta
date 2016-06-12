#include "MogPixel/MogPixel.h"
#include "opencv2\opencv.hpp"

class MogDetector{
private:
	mogPixel* memory;
	int imgHeight, imgWidth;
public:
	MogDetector(int imgHeight, int imgWidth) : imgHeight(imgHeight), imgWidth(imgWidth) { memory = new mogPixel[imgWidth * imgHeight]; };
	~MogDetector() { delete[] memory; };
	void Process(cv::Mat& img, cv::Mat& mask){
		MogPixel MP;
		inData inBuffer;
		for (int i = 0; i < img.rows; i++)
			for (int j = 0; j < img.cols; j++){
				int address = i*img.cols + j;
				inBuffer.memory = memory[address];
				inBuffer.pixel = img.at<char>(i, j);
				outData outBuffer = MP.UpdatePixel(inBuffer);
				mask.at<char>(i, j) = (outBuffer.bg) ? 0 : 255;
				memory[address] = outBuffer.memory;
			}
	}
};