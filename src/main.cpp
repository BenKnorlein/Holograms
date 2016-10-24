///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#define NOMINMAX

#include "ImageCache.h"
#include "OctopusClient.h"
#include "OfflineReader.h"
#include "ReportWriter.h"
#include "Settings.h"
#include "ContourDetection.h"

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

#include <chrono>
#include <ctime>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "Contour.h"
#include "ContourDepthDetection.h"

using namespace cv;

std::string slash = "/"; 

double minAll = std::numeric_limits<double>::min();
double maxAll = std::numeric_limits<double>::max();

bool DoBoxesIntersect(int merge_threshold_dist, cv::Rect a, cv::Rect b) {
	if (a.x + a.width + merge_threshold_dist< b.x) return false; // a is left of b
	if (a.x > b.x + b.width + merge_threshold_dist) return false; // a is right of b
	if (a.y + a.height + merge_threshold_dist < b.y) return false; // a is above b
	if (a.y > b.y + b.height + merge_threshold_dist) return false; // a is below b
	return true; // boxes overlap
}

bool mergebounds(std::vector<cv::Rect> &bounds, std::vector<int> &depths_contour, std::vector<double> &vals_contour, int merge_threshold_depth, int merge_threshold_dist, int step_size)
{
	std::cerr << "Test Intersect" << std::endl;

	bool merged = false;
	for (int c = 0; c < bounds.size(); c++)
	{
		for (int k = c+1; k < bounds.size(); k++)
		{
			if (DoBoxesIntersect(merge_threshold_dist, bounds[c], bounds[k]) && abs(depths_contour[c] - depths_contour[k]) <= merge_threshold_depth)
			{
				std::cerr << "Intersect " << c << " " << k << std::endl;
				int x_max = max(bounds[c].x + bounds[c].width, bounds[k].x + bounds[k].width);
				int y_max = max(bounds[c].y + bounds[c].height, bounds[k].y + bounds[k].height);

				bounds[c].x = min(bounds[c].x, bounds[k].x);
				bounds[c].y = min(bounds[c].y, bounds[k].y);
				bounds[c].width = x_max - bounds[c].x;
				bounds[c].height = y_max - bounds[c].y;

				depths_contour[c] = ((depths_contour[c] + depths_contour[k]) * 0.5 / step_size + 0.5);
				depths_contour[c] *= step_size;
				vals_contour[c] = (vals_contour[c] + vals_contour[k]) * 0.5;
				bounds.erase(bounds.begin() + k);
				depths_contour.erase(depths_contour.begin() + k);
				vals_contour.erase(vals_contour.begin() + k);
				k = k - 1;
				merged = true;
			}
		}
	}
	return merged;
}

int main(int argc, char** argv)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	Settings * settings = new Settings(argv[2]);

	std::string outFile = settings->getOutputFolder() + slash + filename;

#ifdef _MSC_VER
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(outputFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	//setup ReportWriter
	ReportWriter * writer = new ReportWriter(settings->getOutputFolder(), filename);
	//setup ImageCache
	ImageCache * cache;
	if (settings->getOnline())
	{
		cache = new ImageCache(new OctopusClient(settings->getIp(), settings->getPort()), 500);
	} 
	else
	{
		cache = new ImageCache(new OfflineReader(), 500);

	}
	cache->getImageSource()->setSourceHologram(settings->getDatafolder(), filename);

////////Find contours
	ContourDetection * detector = new ContourDetection(cache, settings);
	std::vector<Contour *> contours;

	detector->generateMaxMap();
	writer->saveImage(*detector->getMaxImage(), "maximum.png", true);

	detector->findContours(contours);
	delete detector;

////////Find best depth for contour
	ContourDepthDetection * depthdetector = new ContourDepthDetection(cache, settings);
	for (int i = 0; i < contours.size(); i++)
		depthdetector->findBestDepth(contours[i], settings->getMinDepth(), settings->getMaxDepth(), settings->getStepSize());
	delete depthdetector;

//merge bounds
	/*if (settings->getDoMergebounds()){
		while (mergebounds(bounds, depths_contour, val_contour, settings->getMergeThresholdDepth(), settings->getMergeThresholdDist(), settings->getStepSize()));

		RNG rng(12345);
		Mat drawing = Mat::zeros(cv::Size(settings->getWidth(), settings->getHeight()), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, color, 2, 8, 0, 0, Point());
		}

		for (int i = 0; i< bounds.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
			putText(drawing, std::to_string(((long long)i)), bounds[i].br(),
				FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
		}
		
		writer->saveImage(drawing, "contours.png");

		if (settings->getShow()){
			imshow("Contours", drawing);
			cvWaitKey(1);
			destroyWindow("Contours");
		}
	}*/

///////Refine depths
	if (settings->getDoRefine() && settings->getOnline())
	{ 
		ContourDepthDetection * depthdetector = new ContourDepthDetection(cache, settings);
		for (int i = 0; i < contours.size(); i++)
			depthdetector->findBestDepth(contours[i], contours[i]->getDepth() - settings->getStepSize(), contours[i]->getDepth() + settings->getStepSize(), settings->getStepSize() / 10.0);
		delete depthdetector;
	}

////////Create Report
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	writer->writeXMLReport(contours, std::chrono::duration_cast<std::chrono::minutes>(end - begin).count());
	writer->saveROIImages(cache, contours);

////////Cleanup
	delete cache->getImageSource();
	delete cache;
	delete writer;
	return 0;
}

