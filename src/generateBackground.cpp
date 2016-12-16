///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#include <iostream>

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif

	#include <windows.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

std::string slash = "/"; 

template <typename It>
typename std::iterator_traits<It>::value_type Median(It begin, It end)
{
	auto size = std::distance(begin, end);
		std::nth_element(begin, begin + size / 2, end);
	return *std::next(begin, size / 2);
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		std::cerr << "You need to provide an output filename and a list of images in a txt file" << std::endl;
		return 0;
	}

	std::string filename_out = std::string(argv[1]);
	std::string filename = std::string(argv[2]);

	std::vector<cv::Mat> images;

	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		cv::Mat im = cv::imread(line);
		if (im.data == NULL)
		{
			std::cerr << "Could not load " << line << std::endl;
		}
		else{
			std::cerr << "Loaded " << line << std::endl;
			images.push_back(im);
		}
	}
	infile.close();
	cv::Mat im_out;
	images[0].copyTo(im_out);

	for (int y = 0; y < im_out.rows; y++){
		for (int x = 0; x < im_out.cols; x++){
			//std::cerr << (int)im_out.at<cv::Vec3b>(y, x)[0] << " " << (int)im_out.at<cv::Vec3b>(y, x)[1] << " " << (int)im_out.at<cv::Vec3b>(y, x)[2] << std::endl;

			std::vector<unsigned char> values;
			for (int i = 0; i < images.size(); i++){
				values.push_back(images[i].at<cv::Vec3b>(y, x)[0]);
			}
			unsigned char out_value = Median(values.begin(), values.end());
			im_out.at<cv::Vec3b>(y, x)[0] = out_value;
			im_out.at<cv::Vec3b>(y, x)[1] = out_value;
			im_out.at<cv::Vec3b>(y, x)[2] = out_value;
		}
		
	}
	
	cv::imwrite(filename_out, im_out);
	return 1;
}

