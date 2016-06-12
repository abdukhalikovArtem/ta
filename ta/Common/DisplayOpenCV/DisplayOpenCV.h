#pragma once
#include "../../fcs.h"
#include "opencv2\opencv.hpp"
//using namespace trackfx;
//using namespace cv;
class DisplayCtrl {
private:
	static fcs::Control* ctrl;
	static int imgWidth;
	static int imgHeight;
	static void CallBackFunc(int event, int x, int y, int flags, void* userdata);
	int frameCounter;
	int objectCounter;
	void SetCtrlStatus(fcs::Control::channel::mode mode);

	void DumpObject(fcs::object objects, cv::Mat& img);
public:
	DisplayCtrl(fcs::Control* ctrl, fcs::IInputProvider::VideoSettings& settings) { objectCounter = 0; frameCounter = 0;  this->ctrl = ctrl; imgHeight = settings.imageHeight; imgWidth = settings.imageWidth; };
	void Display(std::vector<fcs::object>& points, cv::Mat& frame, cv::VideoWriter& writer);

	void Display(std::vector<fcs::TFeature>& points, cv::Mat& frame, int maxObjects = -1);
	void Display(cv::Mat& frame);
	
};
