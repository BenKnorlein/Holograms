///\file ContourDetection.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOURDETECTION_H
#define CONTOURDETECTION_H

#include <string>
#include "ImageCache.h"
#include "Settings.h"
#include "Contour.h"

#include <opencv2/core/core.hpp>


class ContourDetection {
public:
	ContourDetection(ImageCache * cache, Settings * settings);
	~ContourDetection();

	void generateMaxMap();
	cv::Mat *getMaxImage();

	void findContours(std::vector<Contour *> &contours);

private:
	ImageCache * m_cache;
	Settings * m_settings;

	cv::Mat m_image_maximum;
};

#endif //CONTOURDETECTION_H