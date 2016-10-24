///\file ContourMerge.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOURMERGE_H
#define CONTOURMERGE_H

#include <string>
#include "ImageCache.h"
#include "Settings.h"
#include "Contour.h"

#include <opencv2/core/core.hpp>


class ContourMerge {
public:
	ContourMerge(ImageCache * cache, Settings * settings);
	~ContourMerge();


private:

};

#endif //CONTOURDETECTION_H