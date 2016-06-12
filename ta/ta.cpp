// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "Primary/primary.h"
#include "opencv2\opencv.hpp"

using namespace std;
using namespace cv;

int main(int, char**)
{
	std::string arg = "D:\\mog_test.mp4";
	int imgHeight = 720, imgWidth = 1280;
	
	VideoCapture capture(arg);

	if (!capture.isOpened()) {
		cerr << "Failed to open video file!\n" << endl;
		system("Pause");
		return 1;
	}

	Mat img;
	Mat mask = Mat::zeros(imgHeight, imgWidth, CV_8U);
	MogDetector detector(imgHeight, imgWidth);

	fcs::IInputProvider::VideoSettings settings;
	settings.imageHeight = imgHeight; settings.imageWidth = imgWidth;

	std::vector<float> sigma{4,7,10,15,20};
	//BlobDetector kpDetector(settings, sigma, 5);

	for (int i = 0; i < 200; i++){
		Mat matOriginal;
	
		capture >> matOriginal;
		cv::cvtColor(matOriginal, img, CV_BGR2GRAY);
		cv::resize(img, img, cv::Size(imgWidth, imgHeight));

		//int size = img.total() * img.elemSize();
		cout << i << endl;

		detector.Process(img, mask);
		//auto primary = kpDetector.Process(img);

		namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
		imshow("Display window", mask);                   // Show our image inside it.
		waitKey(1);

		/*stringstream tmp;
		tmp = stringstream();
		tmp << "D:\\temp\\data\\Background_" << i << "_frame.jpg";
		imwrite(tmp.str(), mask);*/
	}

	//getchar();
}
