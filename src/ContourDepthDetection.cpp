///\file ContourDepthDetection.cpp
///\author Benjamin Knorlein
///\date 11/24/2016

#include "ContourDepthDetection.h"
#include <iostream>

ContourDepthDetection::ContourDepthDetection(ImageCache * cache, Settings * settings) : m_cache(cache), m_settings(settings)
{
	
}

ContourDepthDetection::~ContourDepthDetection()
{


}

void ContourDepthDetection::findBestDepth(Contour* contour, int start, int stop, int step_width)
{
	int best_depth = 0;
	float maxVal = -100000;

	for (int d = start; d <= stop; d += step_width){
		cv::Mat image;
		if (!m_settings->getUseSharpness()){
			image = *m_cache->getPhaseImage(d);
		} 
		else
		{
			image = *m_cache->getGradientImage(d);
		}
		cv::Mat tmp;

		cv::Mat roi;
		((image)(contour->getBoundingBox())).copyTo(roi);

		if (m_settings->getUseAbs()) roi = cv::abs(roi);

		float val = cv::mean(roi, *contour->getMask())[0];
		if (val > maxVal)
		{
				maxVal = val;
				best_depth = d;
		}
	}
	contour->setDepth(best_depth);
	contour->setValue(maxVal);
	std::cout << "Best Depth " << best_depth << std::endl;
}
