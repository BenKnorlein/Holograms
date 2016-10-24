///\file ContourDepthDetection.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOURDEPTHDETECTION_H
#define CONTOURDEPTHDETECTION_H

#include <string>
#include "ImageCache.h"
#include "Settings.h"
#include "Contour.h"

#include <opencv2/core/core.hpp>

class ContourDepthDetection {
public:
	ContourDepthDetection(ImageCache * cache, Settings * settings);
	~ContourDepthDetection();

	void findBestDepth(Contour * contour, int start, int stop, int step_width);

private:
	ImageCache * m_cache;
	Settings * m_settings;
};

#endif //CONTOURDEPTHDETECTION_H