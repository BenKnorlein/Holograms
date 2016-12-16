///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#include "OctopusClient.h"
#include "Settings.h"
#include <iostream>

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif

	#include <windows.h>
	#define THREAD_SAVING 
	#include "ThreadedImageSaver.h"
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

std::string slash = "/"; 

void writeImages(std::string filename, Settings * settings, std::string background = "")
{
#ifdef _MSC_VER
	ThreadedImageSaver* saver = new ThreadedImageSaver(settings);
#endif

	int width = settings->getWidth();
	int height = settings->getHeight();
	std::string datafolder = settings->getDatafolder();
	int start = settings->getMinDepth();
	int stop = settings->getMaxDepth();
	int step_width = settings->getStepSize();

	std::string outFile = settings->getOutputFolder() + slash + filename;
#ifdef _MSC_VER
	CreateDirectory(settings->getOutputFolder().c_str(), NULL);
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(settings->getOutputFolder().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	OctopusClient * client = new OctopusClient(settings->getIp(), settings->getPort());

	//std::cout << "Loading " << datafolder + "/" + filename << std::endl;
	if (background != ""){
		//std::cout << "Use Background " << datafolder + "/" + background << std::endl;
	}

	if (!client->setSourceHologram(datafolder + "/", filename, background))exit;

	std::string name;
	for (int d = start; d <= stop; d += step_width){
		//std::cerr << "Save amplitude " << d << std::endl;
		float* data = new float[width * height];
		client->getAmplitudeImage(d, data);

		if (settings->getSavePngImages()){
			cv::Mat image(cv::Size(width, height), CV_32FC1, data);

			cv::Mat image_disp;
			cv::Mat B;
			normalize(image, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);

			imwrite(outFile + slash + "Amplitude_" + std::to_string(d) + ".png", B);

			image.release();
		}

		name = outFile + slash + "Amplitude_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
		saver->add(name, data);
#else
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);
		delete[] data;
#endif
		
	}

	if (settings->getSaveIntensity()){
		for (int d = start; d <= stop; d += step_width){
			//std::cerr << "Save Intensity " << d << std::endl;
			float* data = new float[width * height];
			client->getIntensityImage(d, data);
	
			if (settings->getSavePngImages()){
				cv::Mat image(cv::Size(width, height), CV_32FC1, data);

				cv::Mat image_disp;
				cv::Mat B;
				normalize(image, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);

				imwrite(outFile + slash + "Intensity_" + std::to_string(d) + ".png", B);

				image.release();
			}

			name = outFile + slash + "Intensity_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
			saver->add(name, data);
#else
			FILE* file = fopen(name.c_str(), "wb");
			fwrite(data, sizeof(float), width * height, file);
			fclose(file);
			delete[] data;
#endif
		}
	}

	//std::cout << "Loading " << datafolder + "/" + filename << std::endl;
	if (!client->setSourceHologram(datafolder + "/", filename))exit;

	for (int d = start; d <= stop; d += step_width){
		//std::cerr << "Save phase " << d << std::endl;
		float* data = new float[width * height];
		client->getPhaseImage(d, data);
		
		if (settings->getSavePngImages()){
			cv::Mat image(cv::Size(width, height), CV_32FC1, data);

			cv::Mat image_disp;
			cv::Mat B;
			normalize(image, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);

			imwrite(outFile + slash + "Phase_" + std::to_string(d) + ".png", B);

			image.release();
		}

		name = outFile + slash + "Phase_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
		saver->add(name, data);
#else
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);
		delete[] data;
#endif
	}

#ifdef _MSC_VER
	delete saver;
#endif
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	Settings * settings = new Settings(argv[2]);

	std::string background = "";

	if (argc >= 3) background = std::string(argv[3]);

	writeImages(filename, settings, background);

	delete settings;
	return 1;
}

