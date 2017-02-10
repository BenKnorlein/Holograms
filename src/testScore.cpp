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
///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#include "OctopusClient.h"
#include "Settings.h"
#include <iostream>

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif

	#include <windows.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "Contour.h"
#include "ContourDetection.h"
#include "OfflineReader.h"

using namespace cv;

std::string slash = "/"; 
ImageCache * cache;
Settings * settings;

template <typename T>
cv::Mat plotGraph(std::vector<T>& vals, int YRange[2])
{

	auto it = minmax_element(vals.begin(), vals.end());
	float scale = 1. / ceil(*it.second - *it.first);
	float bias = *it.first;
	int rows = YRange[1] - YRange[0] + 1;
	cv::Mat image = cv::Mat(rows, vals.size(), CV_8UC1, cv::Scalar(255));
	for (int i = 0; i < (int)vals.size() - 1; i++)
	{
		cv::line(image, cv::Point(i, rows - 1 - (vals[i] - bias)*scale*YRange[1]), cv::Point(i + 1, rows - 1 - (vals[i + 1] - bias)*scale*YRange[1]), cv::Scalar(0, 0, 0), 1);
	}

	return image;
}

void on_mouse(int evt, int x, int y, int flags, void* param)
{
	if (evt == CV_EVENT_LBUTTONDOWN){
		std::cerr << x << " " << y << std::endl;
		std::vector<double> score;
		for (int d = settings->getMinDepth(); d <= settings->getMaxDepth(); d += settings->getStepSize()){
			cv::Mat image_tmp;
			cv::Mat kernel = cv::Mat(settings->getWindowsize(), settings->getWindowsize(), CV_32FC1, cv::Scalar::all(1.0 / (settings->getWindowsize()*settings->getWindowsize())));
			cv::Rect rect = cv::Rect(x - 0.5*(settings->getWindowsize() - 1), y - 0.5*(settings->getWindowsize() - 1), settings->getWindowsize(), settings->getWindowsize());
			if (!settings->getUseSharpness()){
				//filter image
				image_tmp = (*cache->getPhaseImage(d))(rect);
				if (settings->getUseAbs()) image_tmp = cv::abs(image_tmp);
			}
			else
			{
				image_tmp = (*cache->getGradientImage(d))(rect);
			}
			score.push_back(cv::mean(image_tmp)[0]);
		}
		int bounds[2] = { 0, 200 };
		cv::Mat graph = plotGraph(score, bounds);
		imshow("Graph", graph);
	}
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	settings = new Settings(argv[2]);

	std::string outFile = settings->getOutputFolder() + slash + filename;

#ifdef _MSC_VER
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(outFile.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	//setup ImageCache
	
	if (settings->getOnline())
	{
		cache = new ImageCache(new OctopusClient(settings->getIp(), settings->getPort()), settings, 500);
	}
	else
	{
		cache = new ImageCache(new OfflineReader(), settings, settings->getMaxImageCacheStorage());
	}
	cache->getImageSource()->setSourceHologram(settings->getDatafolder(), filename);

	////////Find contours
	ContourDetection * detector = new ContourDetection(cache, settings);
	detector->generateMaxMap();
	;

	cv::namedWindow("Maximum", cv::WINDOW_NORMAL);
	cv::resizeWindow("Maximum", 2048, 2048);
	cv::imshow("Maximum", 0);
	cv::setMouseCallback("Maximum", on_mouse, 0);

	int steps = (settings->getMaxDepth() - settings->getMinDepth()) / settings->getStepSize() + 1;

	cv::namedWindow("Graph", cv::WINDOW_NORMAL);
	cv::resizeWindow("Graph", steps, 200);
	cv::imshow("Graph", 0);
	
	cv::Mat image_disp;
	cv::Mat B;
	normalize(*detector->getMaxImage(), image_disp, 0, 255, CV_MINMAX);
	image_disp.convertTo(B, CV_8U);
	imshow("Maximum", B);
	
	while (true) {
		
		cv::waitKey(1000);

	}

	delete detector;
	return 1;
}

