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
	updateAll();
}


Contour::~Contour()
{

}

void Contour::setValue(double value)
{
	m_value = value;
}

double Contour::getValue()
{
	return m_value;
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

cv::Mat* Contour::getMask()
{
	return &m_mask;
}

void Contour::updateAll()
{
	updateMaskPoints();
	updateBoundingBox();
	updateAlignedBoundingBox();
	updateMoments();
}

void Contour::updateMaskPoints()
{
	m_pointsMask.clear();
	cv::findNonZero(m_mask, m_pointsMask);
	for (std::vector<cv::Point>::iterator pt = m_pointsMask.begin(); pt < m_pointsMask.end(); ++pt)
	{
		*pt += m_boundingbox.tl();
	}
}

void Contour::initMask(std::vector<cv::Point> contourPoints)
{
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
		double eigen_val = sqrt(pca_analysis.eigenvalues.at<double>(0, i));
		m_pca_axis[i] = cv::Point2d(eigen_vec.x * eigen_val, eigen_vec.y * eigen_val);
	}
}

