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
///\file Contour.cpp
///\author Benjamin Knorlein
///\date 11/24/2016

#include "Contour.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>


Contour::Contour(std::vector<cv::Point> contourPoints)
{
	initMask(contourPoints);
	setPointsFromMask();
	updateAll();
	m_values.clear();
}


Contour::~Contour()
{

}

void Contour::setMaxValue(double value)
{
	m_max_value = value;
}

double Contour::getMaxValue()
{
	return m_max_value;
}

std::vector<double>* Contour::getValues()
{
	return &m_values;
}

void Contour::setDepth(int depth)
{
	m_depth = depth;
}

int Contour::getDepth()
{
	return m_depth;
}

cv::Rect Contour::getBoundingBox()
{
	return m_boundingbox;
}

std::vector<cv::Point>* Contour::getPoints()
{
	return &m_pointsMask;
}

cv::Moments Contour::getMoments()
{
	return m_moments;
}

cv::Point2d Contour::getPCACenter()
{
	return m_pca_center;
}

cv::Point2d* Contour::getPCAAxis()
{
	return &m_pca_axis[0];
}

cv::Mat* Contour::getMask()
{
	return &m_mask;
}

void Contour::merge(Contour* contour, int stepsize)
{
	for (std::vector<cv::Point>::iterator pt = contour->getPoints()->begin(); pt < contour->getPoints()->end(); ++pt)
		m_pointsMask.push_back(*pt);

	setMaskFromPoints();
	updateAll();

	m_depth = ((m_depth + getDepth()) * 0.5 / stepsize + 0.5);
	m_depth *= stepsize;
	m_max_value = (m_max_value * m_moments.m00 + contour->getMaxValue() * getMoments().m00) / (m_moments.m00 + getMoments().m00);
}

void Contour::updateAll()
{
	updateBoundingBox();
	updateAlignedBoundingBox();
	updateMoments();
	updatePrincipalAxis();
}

void Contour::setPointsFromMask()
{
	m_pointsMask.clear();
	cv::findNonZero(m_mask, m_pointsMask);
	for (std::vector<cv::Point>::iterator pt = m_pointsMask.begin(); pt < m_pointsMask.end(); ++pt)
	{
		*pt += m_boundingbox.tl();
	}
}

void Contour::setMaskFromPoints()
{
	updateBoundingBox();
	m_mask = cv::Mat::zeros(m_boundingbox.size(), CV_8UC1);
	for (std::vector<cv::Point>::iterator it = m_pointsMask.begin(); it < m_pointsMask.end(); ++it)
	{
		m_mask.at<uchar>(it->y - m_boundingbox.y, it->x - m_boundingbox.x) = 255;
	}
}

void Contour::initMask(std::vector<cv::Point> contourPoints)
{
	std::vector<std::vector<cv::Point> > m_contourPoints;
	std::vector<cv::Point> m_contourPoints_tmp;
	m_contourPoints_tmp.swap(contourPoints);
	m_contourPoints.push_back(m_contourPoints_tmp);
	m_boundingbox = cv::boundingRect(m_contourPoints[0]);
	m_mask = cv::Mat::zeros(m_boundingbox.size(), CV_8UC1);
	cv::drawContours(m_mask, m_contourPoints, 0, cv::Scalar(255), CV_FILLED, 8, 0, 0, cv::Point(-m_boundingbox.x, -m_boundingbox.y));
}

void Contour::updateAlignedBoundingBox()
{
	m_alignedBoundingbox = cv::fitEllipse(m_pointsMask);

	//m_alignedBoundingbox = cv::fitEllipse(cv::Mat(m_contourPoints[0]));
	//std::cerr << m_alignedBoundingbox.size << " - " << m_alignedBoundingbox2.size << std::endl;
	//std::cerr << m_alignedBoundingbox.angle << " - " << m_alignedBoundingbox2.angle << std::endl;
	//std::cerr << m_alignedBoundingbox.center << " - " << m_alignedBoundingbox2.center << std::endl;
}

void Contour::updateMoments()
{
	m_moments = cv::moments(m_mask, true);

	//m_moments = cv::moments(m_contourPoints[0]);
	//std::cerr << m_moments.m00 << " - " << m_moments2.m00 << std::endl;
	//std::cerr << m_moments.m01 << " - " << m_moments2.m01 << std::endl;
	//std::cerr << m_moments.m02 << " - " << m_moments2.m02 << std::endl;
	//std::cerr << m_moments.m03 << " - " << m_moments2.m03 << std::endl;
	//std::cerr << m_moments.m10 << " - " << m_moments2.m10 << std::endl;
	//std::cerr << m_moments.m11 << " - " << m_moments2.m11 << std::endl;
	//std::cerr << m_moments.m12 << " - " << m_moments2.m12 << std::endl;
	//std::cerr << m_moments.m20 << " - " << m_moments2.m20 << std::endl;
	//std::cerr << m_moments.m21 << " - " << m_moments2.m21 << std::endl;
	//std::cerr << m_moments.mu02 << " - " << m_moments2.mu02 << std::endl;
	//std::cerr << m_moments.mu03 << " - " << m_moments2.mu03 << std::endl;
	//std::cerr << m_moments.mu11 << " - " << m_moments2.mu11 << std::endl;
	//std::cerr << m_moments.mu12 << " - " << m_moments2.mu12 << std::endl;
	//std::cerr << m_moments.mu20 << " - " << m_moments2.mu20 << std::endl;
	//std::cerr << m_moments.mu21 << " - " << m_moments2.mu21 << std::endl;
	//std::cerr << m_moments.mu30 << " - " << m_moments2.mu30<< std::endl;
}

void Contour::updateBoundingBox()
{
	m_boundingbox = cv::boundingRect(m_pointsMask);

	//m_boundingbox = cv::boundingRect(m_contourPoints[0]);
	//std::cerr << m_boundingbox.width << " - " << m_alignedBoundingbox2.width << std::endl;
	//std::cerr << m_boundingbox.height << " - " << m_alignedBoundingbox2.height << std::endl;
	//std::cerr << m_boundingbox.x << " - " << m_alignedBoundingbox2.x << std::endl;
	//std::cerr << m_boundingbox.y << " - " << m_alignedBoundingbox2.y << std::endl;
}

void Contour::updatePrincipalAxis()
{
	int sz = m_pointsMask.size();
	cv::Mat data_pts = cv::Mat(sz, 2, CV_64FC1);
	for (int i = 0; i < data_pts.rows; ++i)
	{
		data_pts.at<double>(i, 0) = m_pointsMask[i].x;
		data_pts.at<double>(i, 1) = m_pointsMask[i].y;
	}
	//Perform PCA analysis
	cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);

	//Store the center of the object
	m_pca_center = cv::Point(pca_analysis.mean.at<double>(0, 0),pca_analysis.mean.at<double>(0, 1));
	
	//Store the axis
	std::vector<cv::Point2d> eigen_vecs(2);
	std::vector<double> eigen_val(2);
	for (int i = 0; i < 2; ++i)
	{
		cv::Point2d eigen_vec = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0), pca_analysis.eigenvectors.at<double>(i, 1));
		double eigen_val = sqrt(pca_analysis.eigenvalues.at<double>(i, 0));
		m_pca_axis[i] = cv::Point2d(eigen_vec.x * eigen_val*eigen_val, eigen_vec.y * eigen_val*eigen_val);
	}
}

int Contour::getArea() {
	return m_pointsMask.size();
}


