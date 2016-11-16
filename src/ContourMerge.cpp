///\file ContourMerge.cpp
///\author Benjamin Knorlein
///\date 11/24/2016

#include "ContourMerge.h"
#include <iostream>


ContourMerge::ContourMerge(Settings * settings) : m_settings(settings)
{

}

ContourMerge::~ContourMerge()
{
	

}


int ContourMerge::mergeContours(std::vector<Contour*> &contours)
{
	int count = 0;
	while (mergebounds(contours))
	{
		count++;
	};
	return count;
}


bool ContourMerge::DoBoxesIntersect(int merge_threshold_dist, cv::Rect a, cv::Rect b) {
	if (a.x + a.width + merge_threshold_dist< b.x) return false; // a is left of b
	if (a.x > b.x + b.width + merge_threshold_dist) return false; // a is right of b
	if (a.y + a.height + merge_threshold_dist < b.y) return false; // a is above b
	if (a.y > b.y + b.height + merge_threshold_dist) return false; // a is below b
	return true; // boxes overlap
}

bool ContourMerge::mergebounds(std::vector<Contour*> &contours)
{
	std::cerr << "Test Intersect" << std::endl;

	bool merged = false;
	for (int c = 0; c < contours.size(); c++)
	{
		for (int k = c + 1; k < contours.size(); k++)
		{
			if (DoBoxesIntersect(m_settings->getMergeThresholdDist(), contours[c]->getBoundingBox(), contours[k]->getBoundingBox()) 
				&& abs(contours[c]->getDepth() - contours[k]->getDepth()) <= m_settings->getMergeThresholdDepth())
			{
				contours[c]->merge(contours[k], m_settings->getStepSize());
				std::cerr << "Intersect " << c << " " << k << std::endl;

				contours.erase(contours.begin() + k);

				k = k - 1;
				merged = true;
			}
		}
	}
	return merged;
}
