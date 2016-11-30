///\file ContourDetection.cpp
///\author Benjamin Knorlein
///\date 11/24/2016

#include "ContourDetection.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

ContourDetection::ContourDetection(ImageCache * cache, Settings * settings) : m_cache(cache), m_settings(settings)
{
	m_image_maximum = cv::Mat(cv::Size(m_settings->getWidth(), m_settings->getHeight()), CV_32FC1, cvScalar(0));
}

ContourDetection::~ContourDetection()
{
}


void ContourDetection::generateMaxMap()
{
	m_image_maximum.setTo(0.0);
	if (m_settings->getShow())
	{
		cv::namedWindow("Maximum", cv::WINDOW_NORMAL);
		cv::resizeWindow("Maximum", 800, 800);

		cv::imshow("Maximum", 0);
	}

	for (int d = m_settings->getMinDepth(); d <= m_settings->getMaxDepth(); d += m_settings->getStepSize()){
		std::cout << "Maximum " << d << std::endl;

		cv::Mat image_tmp;
		cv::Mat kernel = cv::Mat(m_settings->getWindowsize(), m_settings->getWindowsize(), CV_32FC1, cv::Scalar::all(1.0 / (m_settings->getWindowsize()*m_settings->getWindowsize())));
		if (!m_settings->getUseSharpness()){
			//filter image
			cv::filter2D(*m_cache->getPhaseImage(d), image_tmp, CV_32F, kernel);
			if (m_settings->getUseAbs()) image_tmp = cv::abs(image_tmp);
		} else
		{
			cv::filter2D(*m_cache->getGradientImage(d), image_tmp, CV_32F, kernel);
		}
		float* max_ptr = (float *)m_image_maximum.data;
		float* image_ptr = (float *)image_tmp.data;

		for (int i = 0; i < m_settings->getWidth() * m_settings->getHeight(); i++, max_ptr++, image_ptr++)
		{
			if (*max_ptr < *image_ptr)
			{
				*max_ptr = *image_ptr;
			}
		}


		if (m_settings->getShow())
		{
			cv::Mat image_disp;
			cv::Mat B;
			normalize(m_image_maximum, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imshow("Maximum", B);
			cv::waitKey(1);
		}
	}

	if (m_settings->getShow())
	{
		cv::destroyWindow("Maximum");
	}
}
void ContourDetection::findContours(std::vector<Contour *> &contours )
{
	normalize(m_image_maximum, m_image_maximum, 0.0, 1.0, CV_MINMAX);

	//threshold	
	cv::threshold(m_image_maximum, m_image_maximum, 1.0 - m_settings->getMaxThreshold(), 1., CV_THRESH_BINARY);

	//convert to 8U
	cv::Mat bw;
	m_image_maximum.convertTo(bw, CV_8U);

	// Find total markers
	std::vector<std::vector<cv::Point> > contours_org;
	cv::findContours(bw, contours_org, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//filter by Area and position
	for (size_t i = 0; i < contours_org.size(); i++)
	{
		if (cv::contourArea(contours_org[i]) >= m_settings->getContourMinArea())
		{
			Contour * newCont = new Contour(contours_org[i]);
			contours.push_back(newCont);
		}
	}
	std::cout << " Contours found " << contours.size() << std::endl;

	/*RNG rng(12345);
	Mat drawing = Mat::zeros(bw.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, -1, 8, 0, 0, Point());
		rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
		putText(drawing, std::to_string(((long long)i)), bounds[i].br(),
			FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
	}

	if (show){
		imshow("Contours", drawing);
		cvWaitKey(1);
		destroyWindow("Contours");
	}*/

	//return drawing;
}


cv::Mat* ContourDetection::getMaxImage()
{
	return &m_image_maximum;
}
