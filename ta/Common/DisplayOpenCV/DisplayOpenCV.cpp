#include "DisplayOpenCV.h"

using namespace std;
using namespace cv;
using namespace fcs;

Control* DisplayCtrl::ctrl;
const int ROISize = 50;
int DisplayCtrl::imgWidth;
int DisplayCtrl::imgHeight;

void DisplayCtrl::SetCtrlStatus(fcs::Control::channel::mode mode) {
	for (int i = 0; i < ctrl->ch.size(); i++)
		ctrl->ch[i].status = mode;
}

void DisplayCtrl::CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		for (int i = 0; i < ctrl->ch.size(); i++)
			ctrl->ch[i].status = Control::channel::Detection;
	}
	if (event == EVENT_LBUTTONUP)
	{
		for (int i = 0; i < ctrl->ch.size(); i++)
			ctrl->ch[i].status = Control::channel::Tracking;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		for (int i = 0; i < ctrl->ch.size(); i++)
			ctrl->ch[i].status = Control::channel::AutoDetection;
	}
	else if (event == EVENT_RBUTTONUP)
	{
		for (int i = 0; i < ctrl->ch.size(); i++)
			ctrl->ch[i].status = Control::channel::Tracking;
	}

	
	// ROI
	x = (x < 0) ? 0 :
		(x > imgWidth - 1) ? x - 1 : x;

	y = (y < 0) ? 0 :
		(y >= imgHeight) ? imgHeight - 1 : y;
	
	ctrl->ch[0].roi = cv::Rect(x - ROISize/2, y - ROISize/2, ROISize, ROISize);
}


void DisplayCtrl::Display(Mat& img) {
	namedWindow("RAW Video", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("RAW Video", img);                   // Show our image inside it.
	waitKey(1);
}

void DisplayCtrl::DumpObject(fcs::object objects, cv::Mat& img) {
	Mat object = img(objects.bbox);
	resize(object, object, cv::Size(32, 32), 0, 0, cv::INTER_CUBIC);
	namedWindow("Object", WINDOW_AUTOSIZE);// Create a window for display.
	waitKey(1);

	std::stringstream tmp;
	string mainPath = std::getenv("DUMP_DATA");
	tmp << mainPath << "\\objects\\";
	tmp << "object_" << objectCounter << ".png";
	std::string s = tmp.str();
	imwrite(s, object);

	objectCounter++;
}

void DisplayCtrl::Display(std::vector<fcs::object>& objects, cv::Mat& img, cv::VideoWriter& writer) {
	Mat draw;
	img.copyTo(draw);

	TResp maxMaj;
	auto maxMajPointer = std::max_element(objects.begin(), objects.end(), [](fcs::object ob1, fcs::object ob2) -> bool { return (ob1.mainFeat.majority < ob2.mainFeat.majority); });
	if (maxMajPointer != objects.end())
		maxMaj = maxMajPointer->mainFeat.majority;

	//Найдем максимальную контрастность для нормировки
	for (auto& a : objects) {
		float norm = a.mainFeat.majority / maxMaj;
		if (0) {// (a.capability & fcs::object::Segment) {
			for (auto& p : a.segment)
				draw.at<Vec3b>(p.y, p.x) = Vec3b(125, 125, 0);
		}

		if (a.capability & fcs::object::Bbox) 
			cv::rectangle(draw, a.bbox, Scalar(0, 0, 255 * norm));
		

		if (a.capability & fcs::object::Feature) {
			TFeature feat = a.mainFeat;
			if (feat.type == TFeature::Type::LoG) {
				circle(draw, Point(feat.x, feat.y), (float)feat.size, Scalar(0, 0, 255 * norm, 0.5), 1, 16);
				if (1) {
					stringstream ss;
					ss << "[Majority] : " << feat.majority;
					putText(draw, ss.str(), Point(feat.x + 10, feat.y + 10), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(0, 255 * norm, 0, 30));
					ss = stringstream();
					ss << "[Response] : " << feat.response;
					putText(draw, ss.str(), Point(feat.x + 10, feat.y + 25), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(0, 255 * norm, 0, 30));
					ss = stringstream();
					ss << "[Size] : " << feat.size;
					putText(draw, ss.str(), Point(feat.x + 10, feat.y + 40), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(0, 255 * norm, 0, 30));
					ss = stringstream();
					ss << "[channelId] : " << a.channelId;
					putText(draw, ss.str(), Point(feat.x + 10, feat.y + 55), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(0, 255 * norm, 0, 30));
				}
			}
		}
		
	}
	
	DumpObject(objects[0], img);

	string str;
	
	if (ctrl->ch[0].status == Control::channel::Detection)
		str = "Manual detection";	
	else if (ctrl->ch[0].status == Control::channel::Tracking)
		str = "Autotracking";
	else if (ctrl->ch[0].status == Control::channel::AutoDetection)
		str = "Autodetection";

	putText(draw, str, Point(10, 10), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(255, 255, 255));
	
	
	namedWindow("Main", WINDOW_AUTOSIZE);// Create a window for display.
	setMouseCallback("Main", CallBackFunc, NULL);
	imshow("Main", draw);                   // Show our image inside it.
	waitKey(1);
	writer << draw;
}

void DisplayCtrl::Display(std::vector<fcs::TFeature>& points, cv::Mat& img, int maxObjects){
	Mat draw;
	img.copyTo(draw);

	TResp maxMaj;
	auto maxMajPointer = std::max_element(points.begin(), points.end(), [](fcs::TFeature p1, fcs::TFeature p2) -> bool { return (p1.majority < p2.majority); });
	if (maxMajPointer != points.end())
		maxMaj = maxMajPointer->majority;

	//Найдем максимальную контрастность для нормировки
	for (auto& a : points) {
		float norm = a.majority / maxMaj;
		circle(draw, Point(a.x, a.y), (float)a.size, Scalar(0, 0, 255 * norm, 0.5), 1, 16);
	}

	namedWindow("Main", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Main", draw);
	waitKey(1);

	std::stringstream tmp, tmp1;
	string mainPath = std::getenv("DUMP_DATA");
	tmp << mainPath << "\\filters\\";
	tmp << "frame_" << frameCounter << ".png";
	tmp1 << mainPath << "\\filters\\" << "img_" << frameCounter << ".png";
	std::string s = tmp.str();
	std::string s2 = tmp1.str();
	imwrite(s, draw);
	imwrite(s2, img);
	frameCounter++;
}