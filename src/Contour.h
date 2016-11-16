///\file Contour.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOUR_H
#define CONTOUR_H

#include <string>
#include "ImageCache.h"

#include <opencv2/core/core.hpp>

class Contour {
public:
	Contour(std::vector<cv::Point> contourPoints);
	~Contour();

	void setValue(double value);
	double getValue();
	void setDepth(int depth);
	int getDepth();
	cv::Rect getBoundingBox();
	cv::Mat * getMask();
	std::vector <cv::Point>* getPoints();
	cv::Moments getMoments();
	cv::Point2d getPCACenter();
	cv::Point2d* getPCAAxis();

	void merge(Contour* contour, int stepsize);
	int getArea();

private:
	
	std::vector <cv::Point> m_pointsMask;
	cv::Mat m_mask;
	int m_depth;
	int m_value;
	cv::Rect m_boundingbox;
	cv::RotatedRect m_alignedBoundingbox; // major axis
	cv::Point2d m_centerOfGravity;
	cv::Moments m_moments;

	cv::Point2d m_pca_center;
	cv::Point2d m_pca_axis[2];


	void initMask(std::vector<cv::Point> contourPoints);
	void updateAll();
	void setPointsFromMask();
	void setMaskFromPoints();
	void updateBoundingBox();
	void updateAlignedBoundingBox();
	void updateMoments();
	void updatePrincipalAxis();
};

#endif //CONTOURDEPTHDETECTION_H
