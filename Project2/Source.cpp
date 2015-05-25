#include <iostream>
#include <opencv2\opencv.hpp>
#include <windows.h>
#include "timer.h"

#define MJPG			1196444237
#define SizeX 64
#define SizeY 96
#define HiddenLayers 13
#define XML 1

//#define WRITE_RESULT_VIDEO

void networkInit(CvANN_MLP *network);

int main(int argc, char* argv[])
{
	CvANN_MLP network;
	networkInit(&network);
	cv::Mat classes(1, 2, CV_32F);

	std::string fName("../cascade_classifier_03.05.2015/cascade.xml");

	cv::CascadeClassifier cascade(fName);
	if (cascade.empty())
	{
		std::cout << "Error: can't open file " << fName << "!\n";
		return 0;
	}

	fName.assign("../testing_videos/nao_cam0.avi");
	cv::VideoCapture cap(fName);
	if (!cap.isOpened())
	{
		std::cout << "Error: can't open file " << fName << "!\n";
		return 0;
	}

#ifdef WRITE_RESULT_VIDEO
	cv::VideoWriter writer;
#endif	
	int c = -1;
	int nFrames = 0;
	int counter = 0;
	int counter_perc = 0;
	float meanTime = 0.0f;
	float tim = 0.0f;
	while (1)
	{
		cv::Mat frColor_;
		cap >> frColor_;

		if (frColor_.empty())
			break;

#ifdef WRITE_RESULT_VIDEO
		if (nFrames == 0)
			writer.open("result4.avi", MJPG, 15, frColor_.size());
#endif

		cv::Mat frColor;
		cv::resize(frColor_, frColor, cv::Size(320, 240));
		cv::Mat frGray;
		cv::cvtColor(frColor, frGray, cv::COLOR_RGB2GRAY);

		std::vector<cv::Rect> objects; 
		std::vector<cv::Rect> netObjects;

		cascade.detectMultiScale(frGray, objects, 1.1, 1, 0, cv::Size(32, 48), cv::Size(158, 237));
		for (int i = 0; i < objects.size(); i++)
		{
			cv::Mat mRoi = frGray(objects[i]);
			cv::resize(mRoi, mRoi, cv::Size(SizeX, SizeY));
			mRoi.convertTo(mRoi, CV_32F);
			network.predict(mRoi.reshape(1, 1), classes);
			if (classes.at<float>(0, 0) > classes.at<float>(0, 1))
				netObjects.push_back(objects[i]);
			cv::rectangle(frColor, objects[i], cv::Scalar(255, 255, 255, 0), 3);
		}
		
		for (int i = 0; i < netObjects.size(); i++) {
			cv::rectangle(frColor, netObjects[i], cv::Scalar(255, 0, 255, 0), 3);
			cv::rectangle(frGray, netObjects[i], cv::Scalar(255, 0, 255, 0), 3);
		}

		cv::imshow("frames", frColor);
		cv::imshow("percetron only", frGray);
		

		cv::waitKey(100);

#ifdef WRITE_RESULT_VIDEO	
		writer.write(frColor);
#endif

		nFrames++;
	}

	cv::waitKey();
	cv::destroyAllWindows();

#ifdef WRITE_RESULT_VIDEO	
	writer.release();
#endif

	return 0;
}


void networkInit(CvANN_MLP *network)
{
	cv::Mat testPics, ans;
	cv::Mat layers(3, 1, CV_32S);
	cv::Mat clas(1, 2, CV_32F);
	char buff[128];

	//positive pics
	clas.at< float >(0, 0) = 1.0f;
	clas.at< float >(0, 1) = 0.0f;

	for (int i = 1; i <= 28; i++) {
		sprintf_s<128>(buff, "C:/nao/nao_detect/learning_images/positive/pos_image_%.2d.jpg", i);
		cv::Mat tmp = cv::imread(buff, CV_LOAD_IMAGE_GRAYSCALE);
		if (tmp.empty()) continue;
		testPics.push_back(tmp.reshape(1, 1));
		ans.push_back(clas);
	}

	//negative pics
	clas.at< float >(0, 0) = 0.0f;
	clas.at< float >(0, 1) = 1.0f;

	for (int i = 29; i <= 3 * 28; i++) {
		sprintf_s<128>(buff, "C:/nao/nao_detect/learning_images/negative/neg_%.2d.jpg", i);
		cv::Mat tmp = cv::imread(buff, CV_LOAD_IMAGE_GRAYSCALE);
		if (tmp.empty()) continue;
		testPics.push_back(tmp.reshape(1, 1));
		ans.push_back(clas);
	}

	layers.at< int >(0, 0) = SizeX*SizeY; //size of pic
	layers.at< int >(1, 0) = HiddenLayers; // 
	layers.at< int >(2, 0) = 2; //robot not robot

	testPics.convertTo(testPics, CV_32F);

	if (!XML) {
		network->create(layers);
		cv::TermCriteria c(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 100000, 0.0001);
 		CvANN_MLP_TrainParams p(c, CvANN_MLP_TrainParams::BACKPROP, 0.01);
 		std::cout << "Starting train...\n";
		network->train(testPics, ans, cv::Mat(), cv::Mat(), p);
		network->write(*(cv::FileStorage("perceptorn.xml", cv::FileStorage::WRITE)), "robot");
	} else 
		network->load("perceptorn.xml", "robot");
	std::cout << "Network ready!\n";
}