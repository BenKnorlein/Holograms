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

struct contour {
	cv::Mat mask;
	cv::Mat image;
	double area;
};

void segmentObject(std::string filename, std::vector <contour> &contours)
{
	cv::Mat img_rgb = cv::imread(filename);
	cv::Mat image = cv::Mat(img_rgb.rows, img_rgb.cols, CV_8UC1);
	cv::Mat image_mask = cv::Mat(img_rgb.rows, img_rgb.cols, CV_8UC1);

	for (int i = 0; i < img_rgb.rows; i++)
	{
		for (int j = 0; j < img_rgb.cols; j++)
		{
			uchar val = img_rgb.at<cv::Vec3b>(i, j)[0];
			image.at<unsigned char>(i, j) = val;
			if (img_rgb.at<cv::Vec3b>(i, j)[2] != img_rgb.at<cv::Vec3b>(i, j)[0])
			{
				image_mask.at<unsigned char>(i, j) = 0;
			}
			else
			{
				image_mask.at<unsigned char>(i, j)= 255;
			}
		}
	}
	
	
	cv::Mat image_fg = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat image_pfg = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat image_pbg = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat image_bg = cv::Mat(image.rows, image.cols,CV_8UC1,cv::Scalar(0));
	cv::Mat mask = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));

	threshold(image, image_fg, 190, 255, CV_THRESH_BINARY);
	threshold(image, image_pfg, 75, 255, CV_THRESH_BINARY);
	cv::rectangle(image_bg, cv::Point(0, 0), cv::Point(image_bg.cols - 1, image_bg.rows - 1), 255);

	//imshow("Background", image_bg);
	//imshow("image_pfg", image_pfg);
	//imshow("image_fg", image_fg);
	//imshow("image_pbg", image_pbg);

	for (int x = 0; x < image.cols; x++)
	{
		for (int y = 0; y < image.rows; y++)
		{
			if (image_fg.at<unsigned char>(y, x) > 100)
			{
				mask.at<unsigned char>(y, x) = GC_FGD;
			}
			else if (image_pfg.at<unsigned char>(y, x) > 100)
			{
				mask.at<unsigned char>(y, x) = GC_PR_FGD;
			}
			else if (image_bg.at<unsigned char>(y, x) > 100)
			{
				mask.at<unsigned char>(y, x) = GC_BGD;
			}
			else
			{
				mask.at<unsigned char>(y, x) = GC_PR_BGD;
			}
		}
	}
	cv::Mat bgdModel, fgdModel;
	cv::Rect rect = cv::Rect(0, 0, image.cols, image.rows);
	//imshow("Mask" , mask * 50);
	cv::Mat image_rgb = cv::Mat(image.rows, image.cols, CV_8UC3);
	cv::cvtColor(image, image_rgb, CV_GRAY2RGB);

	grabCut(image_rgb, mask, rect, bgdModel, fgdModel, 10, GC_INIT_WITH_MASK);
	
	cv::Mat bw = (mask & 1);
	std::vector<std::vector<cv::Point> > contours_org;
	cv::findContours(bw, contours_org, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//filter by Area and position
	float maxSize = 0;
	cv::Mat mask_cont = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));;
	for (size_t i = 0; i < contours_org.size(); i++)
	{
		if (cv::contourArea(contours_org[i]) >= maxSize)
		{
			mask_cont = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(0));;
			cv::drawContours(mask_cont, contours_org, i, cv::Scalar(255), CV_FILLED, 8, 0, 0);
			maxSize = cv::contourArea(contours_org[i]);
		}
	}

	cv::Mat image_masked = cv::Mat(image.rows, image.cols, CV_8UC1, cv::Scalar(255));;
	image.copyTo(image_masked, mask_cont);
	//std::cerr << maxSize << " " << cv::sum(mask_cont) / 255 << std::endl;

	contour c;
	c.mask = mask_cont;
	c.image = image;
	c.area = maxSize;
	contours.push_back(c);
	
	//imshow("masked Image", image_masked);
	//imshow("original", image);
	//imshow("mask_original", image_mask);
	//imshow("mask Grabcut", (mask & 1) * 255);
	//imshow("cont_mask", mask_cont);

	//cv::waitKey(0);
}

void readImages(std::string inputdir, std::vector<std::string> & images)
{
	std::string reportName = inputdir + "\\reportRaw.xml";
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
		tinyxml2::XMLElement* titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");

		if (titleElement) {
			for (tinyxml2::XMLElement* child = titleElement->FirstChildElement("ROI"); child != NULL; child = child->NextSiblingElement("ROI"))
			{
				if (child->FirstChildElement("IMAGE")){
					images.push_back(child->FirstChildElement("IMAGE")->GetText());
				}
			}
		}
	}
}

void writeRefinedReport(std::string inputdir, std::vector <contour> &contours)
{
	std::string reportName = inputdir + "\\reportRaw.xml";
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
		tinyxml2::XMLElement* titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");
		int count = 0;
		if (titleElement) {
			for (tinyxml2::XMLElement* child = titleElement->FirstChildElement("ROI"); child != NULL; child = child->NextSiblingElement("ROI"))
			{
				if (child->FirstChildElement("IMAGE")){
					std::string name = child->FirstChildElement("IMAGE")->GetText();
					name = name.substr(0, name.rfind(".")) + "_refined.png";
					child->FirstChildElement("IMAGE")->SetText(name.c_str());
					
					cv::Mat m = cv::Mat::zeros(contours[count].image.rows, contours[count].image.cols, CV_8UC3);
					for (int row = 0; row < m.rows; row++) {
						for (int col = 0; col < m.cols; col++) {
							unsigned char d = contours[count].image.at<unsigned char>(row, col);

							cv::Vec3b pixel;
							pixel[0] = d;
							pixel[1] = d;
							pixel[2] = d;

							unsigned char d2 = contours[count].mask.at<unsigned char>(row, col);
							if (d2 == 0) {
								pixel[2] = 255 * 0.3 + 0.7 * pixel[2];
							}

							m.at<cv::Vec3b>(row, col) = pixel;
						}
					}



					cv::imwrite(inputdir + "//" + name, m);
					
				}
				child->FirstChildElement("AREA_PIXEL")->SetText(contours[count].area);
				double screen_to_source = 100000;
				double pixel_size = 7.4;
				double depth = std::stof(child->FirstChildElement("DEPTH")->GetText());
				double conversion = (float) depth / screen_to_source * pixel_size;
				double area = contours[count].area * conversion * conversion;
				child->FirstChildElement("AREA")->SetText(area);
				float esd = sqrt(area * 6 / CV_PI);
				float esv = CV_PI * pow(esd, 3) / 6;
				child->FirstChildElement("ESD")->SetText(esd);
				child->FirstChildElement("ESV")->SetText(esv);
				count++;
			}
		}
		std::string outxml = inputdir + "\\reportRaw_refined.xml";
		doc.SaveFile(outxml.c_str());
	}
}


std::istream& safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;)
	{
		int c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty())
			{
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::vector<std::string> readReportFolders(std::string inputdir)
{
	std::vector<std::string> reportDirectories;

	HANDLE          hFile;                       // Handle to directory
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = inputdir + "\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = inputdir + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string reportFile = strFilePath + "\\reportRaw.xml";
					if (FileExists(reportFile.c_str()))
					{
						reportDirectories.push_back(FileInformation.cFileName);
					}
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return reportDirectories;
}

int main(int argc, char** argv)
{
	std::string inputdirMain = std::string(argv[1]);

	std::vector<std::string> folders = readReportFolders(inputdirMain);

	for (int f = 0; f < folders.size(); f++){
		std::vector <std::string> images;
		std::string inputdir = inputdirMain + "\\" + folders[f];
		std::cerr << inputdir << std::endl;
		readImages(inputdir, images);

		std::vector <contour> contours;
		for (int i = 0; i < images.size(); i++)
		{
			std::cerr << images[i] << std::endl;
			segmentObject(inputdir + "\\" + images[i], contours);
		}

		writeRefinedReport(inputdir, contours);

		std::ofstream outfile(inputdir + "//" + "report_refined.xml", std::ofstream::out);
		
		std::ifstream fin("template.xml");
		std::string line;
		safeGetline(fin, line);
		while (!safeGetline(fin, line).eof())
		{
			outfile << line << std::endl;
			line.clear();
		}
		fin.close();


		outfile << std::endl;

		std::ifstream fin2(inputdir + "//" + "reportRaw_refined.xml");
		safeGetline(fin2, line);
		while (!safeGetline(fin2, line).eof())
		{
			outfile << line << std::endl;
			line.clear();
		}
		fin.close();
		outfile.close();
	}
return 1;
}

