#include "../../fcs.h"

using namespace fcs;

IInputProvider::IInputProvider (std::string filename, VideoSettings settings){
	this->settings = settings;
	settings.imgType = CV_8U;
	
	video = new cv::VideoCapture(filename);
}

bool IInputProvider::GetNextFrame(cv::Mat& dst) {
	bool isNotEOF = video->read(frame);
	if (!isNotEOF)
	{
		return false;
	}

	if (settings.crop.area() > 0)
		frame = frame(settings.crop);

	if ((this->settings.imageWidth != 0) & (settings.imageHeight != 0))
		resize(frame, frame, cv::Size(this->settings.imageWidth, settings.imageHeight), 0, 0, cv::INTER_CUBIC);

	cv::cvtColor(frame, dst, CV_BGR2GRAY);
	
	return true;
}

void IInputProvider::SetVideoPos(double ms) {
	video->set(CV_CAP_PROP_POS_MSEC, ms);
}

double IInputProvider::GetVideoPos(void) {
	return video->get(CV_CAP_PROP_POS_MSEC);
}