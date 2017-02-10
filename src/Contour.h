//  ----------------------------------
//  Holograms -- Copyright © 2016, Brown University, Providence, RI.
//  
//  All Rights Reserved
//   
//  Use of the software is provided under the terms of the GNU General Public License version 3 
//  as published by the Free Software Foundation at http://www.gnu.org/licenses/gpl-3.0.html, provided 
//  that this copyright notice appear in all copies and that the name of Brown University not be used in 
//  advertising or publicity pertaining to the use or distribution of the software without specific written 
//  prior permission from Brown University.
//  
//  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE WHICH IS 
//  PROVIDED “AS IS”, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
//  FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY 
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR FOR ANY DAMAGES WHATSOEVER RESULTING 
//  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
//  OTHER TORTIOUS ACTION, OR ANY OTHER LEGAL THEORY, ARISING OUT OF OR IN CONNECTION 
//  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
//  ----------------------------------
//  
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

	void setMaxValue(double value);
	double getMaxValue();

	std::vector<double>* getValues();

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
	double m_max_value;
	std::vector<double> m_values;
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
