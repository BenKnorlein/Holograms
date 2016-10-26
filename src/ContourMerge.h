///\file ContourMerge.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOURMERGE_H
#define CONTOURMERGE_H

#include <string>
#include "Settings.h"
#include "Contour.h"

#include <opencv2/core/core.hpp>


class ContourMerge {
public:
	ContourMerge(Settings * settings);
	~ContourMerge();

	int mergeContours(std::vector<Contour*> &contours);

private:
	bool DoBoxesIntersect(int merge_threshold_dist, cv::Rect a, cv::Rect b);
	bool mergebounds(std::vector<Contour*> &contours);
	Settings* m_settings;
};

#endif //CONTOURMERGE_H