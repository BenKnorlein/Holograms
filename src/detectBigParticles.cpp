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

#include <iostream>
#include "tinyxml2.h"

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
		std::string slash = "\\";
	#include <windows.h>
#else
	#include <dirent.h>
	#include <fcntl.h>         // open
	#include <unistd.h>  // read, write, close
	#include <sys/sendfile.h>  // sendfile
	#include <sys/stat.h>
	#include <sys/types.h>
std::string slash = "/";
#endif

#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;




template <typename It>
typename std::iterator_traits<It>::value_type Median(It begin, It end)
{
	auto size = std::distance(begin, end);
		std::nth_element(begin, begin + size / 2, end);
	return *std::next(begin, size / 2);
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool has_suffix(const std::string &str, const std::string &suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int countFiles(std::string dir)
{
	int files = 0;
#ifdef _MSC_VER	
	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::string search = dir + "\\*.*";
	hFind = FindFirstFile(search.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			files++;
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
#else
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		return 0;
	}

	while ((dirp = readdir(dp)) != NULL){
		if(has_suffix(std::string(dirp->d_name), ".bmp")) {
			files++;
		}
	}									
	closedir(dp);
#endif
	return files;
}

std::vector <std::string> readTXT(std::string folder)
{
	std::vector<std::string> files;
#ifdef _MSC_VER	
	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::string search = folder + "\\*.txt";
	hFind = FindFirstFile(search.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (hasEnding(data.cFileName, ".txt"))
				files.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
#else
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(folder.c_str())) == NULL) {
		return files;
	}

	while ((dirp = readdir(dp)) != NULL){
		if (has_suffix(std::string(dirp->d_name), ".txt")) {
			files.push_back(std::string(dirp->d_name));
		}
	}
	closedir(dp);
#endif

	std::sort(files.begin(), files.end());
	return files;
}

std::vector <std::string> readBMP(std::string folder)
{
	std::vector<std::string> files;
#ifdef _MSC_VER	
	HANDLE hFind;
	WIN32_FIND_DATA data;	
	std::string search = folder + "\\*.bmp";
	hFind = FindFirstFile(search.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (hasEnding(data.cFileName, ".bmp"))
				files.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
#else
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(folder.c_str())) == NULL) {
		return files;
	}

	while ((dirp = readdir(dp)) != NULL){
		if(has_suffix(std::string(dirp->d_name), ".bmp")) {
			files.push_back(std::string(dirp->d_name));
		}
	}
	closedir(dp);
#endif

	std::sort(files.begin(), files.end());
	return files;
}

void copyFile(std::string in, std::string out)
{
#ifdef _MSC_VER	
	CopyFile(in.c_str(), out.c_str(), FALSE);
#else
	int source = open(in.c_str(), O_RDONLY, 0);
	int dest = open(out.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);

	// struct required, rationale: function stat() exists also
	struct stat stat_source;
	fstat(source, &stat_source);

	sendfile(dest, source, 0, stat_source.st_size);

	close(source);
	close(dest);
#endif
}

double computeStdDev(cv::Mat & matrix)
{
	double mean = 0;
	for (int y = 0; y < matrix.rows; y++){
		for (int x = 0; x < matrix.cols; x++){

			mean += matrix.at<float>(y, x);
		}
	}
	mean = mean / (matrix.rows *  matrix.cols);

	double stddev = 0;
	for (int y = 0; y < matrix.rows; y++){
		for (int x = 0; x < matrix.cols; x++){

			stddev += pow(matrix.at<float>(y, x) - mean, 2); 
		}
	}

	stddev = sqrt(stddev / (matrix.rows *  matrix.cols - 1));
	return stddev;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		std::cerr << "You need to provide: input directory, Images used for median, output directory " << std::endl;
		return 0;
	}

	std::string inputdir = std::string(argv[1]);
	int nbMedian = std::atoi(argv[2]);
	std::string outdir = std::string(argv[3]);
	double thresh = std::stof(argv[4]);

	bool useCTDData = false;
	if (argc > 5){
		useCTDData = std::atoi(argv[5]);
	}
	std::vector<std::string> files = readBMP(inputdir);

	std::cerr << std::endl;
	int current =(nbMedian - 1) / 2 + 5;
	int end_pos = files.size() - 1 - (nbMedian - 1) / 2;

	if (useCTDData){
		std::vector<std::string> files_csv = readTXT(inputdir);
		std::string start = files_csv[0];
		std::string end = files_csv[files_csv.size() - 1];
		start = start.substr(0, start.rfind(".")) + ".bmp";
		end = end.substr(0, end.rfind(".")) + ".bmp";

		int start_pos = std::find(files.begin(), files.end(), start) - files.begin();
		end_pos = std::find(files.begin(), files.end(), end) - files.begin();

		current = start_pos;
	}

	while (current <= end_pos){
		std::vector<cv::Mat> images;
		for (int i = 0; i < nbMedian; i++)
		{
			std::string filename_tmp = inputdir + slash + files[current + i - (nbMedian - 1) / 2];
			cv::Mat im = cv::imread(filename_tmp);
			if (im.data == NULL)
			{
				std::cerr << "Could not load " << filename_tmp << std::endl;
			}
			else{
				std::cerr << "Loaded " << filename_tmp << std::endl;
				images.push_back(im);
			}
		}

		cv::Mat im_out;
		images[0].copyTo(im_out);

		for (int y = 0; y < im_out.rows; y++){
			for (int x = 0; x < im_out.cols; x++){

				std::vector<unsigned char> values;
				for (int i = 0; i < images.size(); i++){
					values.push_back(images[i].at<cv::Vec3b>(y, x)[0]);
				}
				unsigned char out_value = Median(values.begin(), values.end());
				im_out.at<cv::Vec3b>(y, x)[0] = out_value;
				im_out.at<cv::Vec3b>(y, x)[1] = out_value;
				im_out.at<cv::Vec3b>(y, x)[2] = out_value;
			}

		}

		bool hasBigParticle = false;

		int middle = current;

		double stdDev = 0.0;
		cv::Mat difference(im_out.size(), CV_32FC1);
		if (1)
		{
			
			for (int y = 0; y < im_out.rows; y++){
				for (int x = 0; x < im_out.cols; x++){
					difference.at<float>(y, x) = ((float)images[(nbMedian - 1) / 2].at<cv::Vec3b>(y, x)[0]) - ((float) im_out.at<cv::Vec3b>(y, x)[0]);
				}
			}

			

			//std::cerr << cv::sum(difference)[0] << std::endl;
			cv::Mat blurred(im_out.size(), CV_32FC1);
			cv::GaussianBlur(difference, blurred, cv::Size(15, 15), 5);
			for (int y = 0; y < im_out.rows; y++){
				for (int x = 0; x < im_out.cols; x++){

					if(blurred.at<float>(y, x) < 0.0)
					{
						blurred.at<float>(y, x) = 0.0;
					}
					//else{
						//std::cerr << blurred.at<float>(y, x) << std::endl;
					//}
				}
			}
			
			stdDev = computeStdDev(blurred);
			double _min, _max;
			cv::minMaxLoc(blurred, &_min,& _max);
			std::cerr << _max << std::endl;
			std::cerr << cv::sum(blurred)[0] << std::endl;
			std::cerr << stdDev << std::endl;
			if (0){
				std::vector <std::vector <Vec3f>> circles_all;
				cv::Mat image_disp;
				cv::Mat B;
				image_disp = blurred * 25;
				//normalize(blurred, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);

				for (int c = 0; c < 500; c++)
				{
					std::vector <Vec3f> circles;

					HoughCircles(B, circles, CV_HOUGH_GRADIENT, 1, B.rows / 32, 180, 100, c, c);
					circles_all.push_back(circles);
				}
				cv::namedWindow("Maximum", cv::WINDOW_NORMAL);
				cv::resizeWindow("Maximum", 800, 800);

				imshow("Maximum", B);
				for (int c = 0; c < 500; c++)
				{
					for (size_t i = 0; i < circles_all[c].size(); i++)
					{
						Point center(cvRound(circles_all[c][i][0]), cvRound(circles_all[c][i][1]));
						int radius = cvRound(circles_all[c][i][2]) + 10;
						// circle center
						//circle(B, center, 3, Scalar(255, 255, 255), -1, 8, 0);
						// circle outline
						circle(blurred, center, radius, Scalar(0, 0, 0), -1, 8, 0);
					}
					//imshow("Maximum", B);
					//cv::waitKey(1);
				}

				stdDev = computeStdDev(blurred);
				std::cerr << stdDev << std::endl;
			}


			if (stdDev > thresh)
			{
				cv::namedWindow("Maximum", cv::WINDOW_NORMAL);
				cv::resizeWindow("Maximum", 800, 800);

				cv::imshow("Maximum", 0);
				cv::Mat image_disp;
				cv::Mat B;
				image_disp = blurred * 25;
				//normalize(blurred, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);
				cv::imwrite(outdir + slash + files[middle].substr(0, files[middle].rfind(".")) + "_diff.png", B);
				imshow("Maximum", B);
				cv::waitKey(1);
			}
			if (stdDev > thresh)  hasBigParticle = true;
		}

		if (hasBigParticle){

			std::cerr << "Copy File to " << outdir + slash + files[middle] << std::endl;
			//int outDirMin = std::numeric_limits<int>::max();


			copyFile(inputdir + slash + files[middle].substr(0, files[middle].rfind(".")) + ".txt", outdir + slash + files[middle].substr(0, files[middle].rfind(".")) + ".txt");
			if (useCTDData){
				copyFile(inputdir + slash + files[middle].substr(0, files[middle].rfind(".")) + ".txt", outdir + slash + files[middle].substr(0, files[middle].rfind(".")) + ".txt");
			}
			copyFile(inputdir + slash + files[middle], outdir + slash + files[middle]);
			std::string outputFilenameBackground = files[middle].substr(0, files[middle].length() - 4) + "_background.bmp";
			cv::imwrite(outdir + slash + outputFilenameBackground, im_out);

			std::string outputFilenameXML = outdir + slash + files[middle].substr(0, files[middle].length() - 4) + ".xml";
			FILE * xml_file = fopen(outputFilenameXML.c_str(), "w");
			tinyxml2::XMLPrinter printer(xml_file);
			printer.OpenElement("Processing");
			printer.OpenElement("Step1");
			printer.OpenElement("Image");
			printer.PushText(files[middle].c_str());
			printer.CloseElement();
			printer.OpenElement("Background");
			printer.PushText(outputFilenameBackground.c_str());
			printer.CloseElement();
			printer.OpenElement("StdDev");
			printer.PushText(std::to_string(stdDev).c_str());
			printer.CloseElement();
			printer.OpenElement("OriginalFolder");
			printer.PushText(inputdir.c_str());
			printer.CloseElement();
			for (int i = 0; i < nbMedian; i++)
			{
				std::string filename_tmp = files[current + i];
				printer.OpenElement("ImageUsedForBackground");
				printer.PushText(filename_tmp.c_str());
				printer.CloseElement();
			}

			printer.CloseElement();
			printer.CloseElement();
			fclose(xml_file);
		}
		current += 1;
		std::cerr << std::endl;
	}
	return 1;
}

