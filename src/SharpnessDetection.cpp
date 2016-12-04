///\file SharpnessDetection.cpp
///\author Benjamin Knorlein
///\date 11/22/2016

#include "SharpnessDetection.h"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include "ImageCache.h"

SharpnessDetection::SharpnessDetection(ImageCache * cache, Settings * settings) : m_cache(cache), m_settings(settings)
{


}

SharpnessDetection::~SharpnessDetection()
{


}

cv::Mat SharpnessDetection::getSharpnessImage(int size, int depth, int method)
{
	cv::Mat* image = m_cache->getAmplitudeImage(depth);
	cv::Mat out = cv::Mat(image->size(), CV_32FC1, cv::Scalar::all(0.0));

	cv::Mat image_val;

	switch (method)
	{
		case 0: 
			image_val = gradientSquared(*image);
			return (size == 0) ? image_val : smartSum(size, image_val);
			break;
		case 1:
			image_val = brennerGradient(*image);
			return (size == 0) ? image_val : smartSum(size, image_val);
			break;
		default:
		case 2:
			image_val = tennenbaumGradient(*image);
			return (size == 0) ? image_val : smartSum(size, image_val);
			break;
	}

	
}

cv::Mat SharpnessDetection::smartSum(int size, cv::Mat & in)
{
	cv::Mat sum_x_mat = cv::Mat(in.size(), CV_32FC1, cv::Scalar::all(0.0));

	for (int y = 0; y < in.rows; y++)
	{
		cv::Rect rect = cv::Rect(0, y, 2 * size + 1, 1);
		cv::Mat sub = in(rect);
		float val = cv::sum(sub)[0];
		sum_x_mat.at<float>(size, y) = val;
		for (int x = size + 1; x < in.cols - size; x++)
		{
			val = val + in.at<float>(y, x + size) - in.at<float>(y, x - size - 1);
			sum_x_mat.at<float>(y, x) = val;
		}
	}

	cv::Mat out = cv::Mat(in.size(), CV_32FC1, cv::Scalar::all(0.0));

	for (int x = 0; x < in.cols; x++)
	{
		cv::Rect rect = cv::Rect(x, 0, 1, 2 * size + 1);
		cv::Mat sub = sum_x_mat(rect);
		double val = cv::sum(sub)[0];
		out.at<float>(x, size) = val;

		for (int y = size + 1; y < in.rows - size ; y++)
		{
			val = val - sum_x_mat.at<float>(y - size - 1, x) + sum_x_mat.at<float>(y + size,x);
			out.at<float>(y, x) = val;
		}
	}

	cv::divide(out, cv::Scalar(size*size), out);

	return sum_x_mat;
}

cv::Mat SharpnessDetection::gradientSquared(cv::Mat &image)
{
	cv::Mat kernel = cv::Mat(1,2, CV_32FC1);
	kernel.at<float>(0, 0) = -1;
	kernel.at<float>(0, 1) = 1;
	cv::Mat image_tmp;
	cv::filter2D(image, image_tmp, CV_32F, kernel, cv::Point(0,0));
	cv::multiply(image_tmp, image_tmp, image_tmp);
	return image_tmp;
}
cv::Mat SharpnessDetection::brennerGradient(cv::Mat &image)
{
	cv::Mat kernel = cv::Mat(1, 3, CV_32FC1);
	kernel.at<float>(0, 0) = -1;
	kernel.at<float>(0, 1) = 0;
	kernel.at<float>(0, 2) = 1;
	cv::Mat image_tmp;
	cv::filter2D(image, image_tmp, CV_32F, kernel, cv::Point(0, 0));
	cv::multiply(image_tmp, image_tmp, image_tmp);
	return image_tmp;
}
cv::Mat SharpnessDetection::tennenbaumGradient(cv::Mat &image)
{
	cv::Mat kernelX = cv::Mat(3, 3, CV_32FC1);
	kernelX.at<float>(0, 0) = -1;
	kernelX.at<float>(0, 1) = 0;
	kernelX.at<float>(0, 2) = 1;
	kernelX.at<float>(1, 0) = -2;
	kernelX.at<float>(1, 1) = 0;
	kernelX.at<float>(1, 2) = 2;
	kernelX.at<float>(2, 0) = -1;
	kernelX.at<float>(2, 1) = 0;
	kernelX.at<float>(2, 2) = 1;
	cv::Mat gradX;
	cv::filter2D(image, gradX, CV_32F, kernelX);
	cv::multiply(gradX, gradX, gradX);

	cv::Mat kernelY = cv::Mat(3, 3, CV_32FC1);
	kernelY.at<float>(0, 0) = -1;
	kernelY.at<float>(0, 1) = -2;
	kernelY.at<float>(0, 2) = -1;
	kernelY.at<float>(1, 0) = 0;
	kernelY.at<float>(1, 1) = 0;
	kernelY.at<float>(1, 2) = 0;
	kernelY.at<float>(2, 0) = 1;
	kernelY.at<float>(2, 1) = 2;
	kernelY.at<float>(2, 2) = 1;
	cv::Mat gradY;
	cv::filter2D(image, gradY, CV_32F, kernelY);
	cv::multiply(gradY, gradY, gradY);

	cv::add(gradX, gradY, gradX);
	return gradX;
}