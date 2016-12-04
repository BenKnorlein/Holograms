///\file SharpnessDetection.h
///\author Benjamin Knorlein
///\date 11/22/2016

#ifndef SHARPNESSDETECTIONDETECTION_H
#define SHARPNESSDETECTIONDETECTION_H

#include <string>
#include "Settings.h"

#include <opencv2/core/core.hpp>
class ImageCache;

class SharpnessDetection {
public:
	SharpnessDetection(ImageCache * cache, Settings * settings);
	~SharpnessDetection();

	cv::Mat getSharpnessImage(int size, int depth, int method = 2);

	cv::Mat gradientSquared(cv::Mat &image);
	cv::Mat brennerGradient(cv::Mat &image);
	cv::Mat tennenbaumGradient(cv::Mat &image);

private:
	Settings * m_settings;
	ImageCache * m_cache;

	
	cv::Mat smartSum(int size, cv::Mat & in);

};

#endif //SHARPNESSDETECTIONDETECTION_H