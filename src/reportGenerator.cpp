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


	strPattern = inputdir + "\\98\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = inputdir + "\\98\\" + FileInformation.cFileName;

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

void copyFile(std::string in, std::string out)
{
	CopyFile(in.c_str(), out.c_str(), FALSE);
}

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

void generateNbContoursPlot(std::ofstream &outfile, std::string inputdir, std::vector<std::string> reportnames, std::string threshold, std::string outputdata)
{
	std::vector<int> nbContours;
	for (int i = 0; i < reportnames.size(); i++)
	{
		std::string reportName = inputdir + "\\" + threshold + "\\" + reportnames[i] + "\\reportRaw.xml";
		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
			tinyxml2::XMLElement* titleElement;
			titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");

			if (titleElement) {
				tinyxml2::XMLElement* el = titleElement->FirstChildElement("NBCONTOURS");
				if (el)
				{
					nbContours.push_back(std::atoi(el->GetText()));
				}
			}
		}
	}

	int bounds[2] = { 0, 300 };
	cv::Mat plot = plotGraph(nbContours, bounds);
	cv::imwrite(outputdata + "/" + "contoursPerImage_" + threshold + ".png", plot);
	outfile << "<td >" << std::endl;
	outfile << "<img src = \"" << outputdata + "/" + "contoursPerImage_" + threshold + ".png" << "\" width = \"512\" height = \"512\">" << "</a>" << std::endl;
	outfile << "</td>" << std::endl;
}


void writeCellImage(std::ofstream &outfile, std::string inputdir, std::string reportname, std::string threshold, std::string outputdata)
{
	std::string outImageName = outputdata + "\\" + reportname + "_" + threshold + ".png";
	copyFile(inputdir + "\\" + threshold + "\\" + reportname + "\\contoursSplat.png", outImageName);

	outfile << "<td >" << std::endl;
	outfile << "<a href = \"" + inputdir + "\\" + threshold + "\\" + reportname + "\\report.xml\">";
	outfile << "<img src = \"" << outImageName << "\" width = \"512\" height = \"512\">" << "</a>" << std::endl;
	outfile << "</td>" << std::endl;
}

void writeCellData(std::ofstream &outfile, std::string inputdir, std::string reportname, std::string threshold, std::string outputdata)
{
	outfile << "<td >" << std::endl;
	std::string reportName = inputdir + "\\" + threshold + "\\" + reportname + "\\reportRaw.xml";
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
		tinyxml2::XMLElement* titleElement;
		titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");

		if (titleElement) {
			tinyxml2::XMLElement* el = titleElement->FirstChildElement("FILENAME");
			if (el)
			{
				outfile << "<p> File: " << el->GetText() << "</p>" << std::endl;
			}

			outfile << "<p> Threshold: 0." << threshold << "</p>" << std::endl;
			
			el = titleElement->FirstChildElement("NBCONTOURS");
			if (el)
			{
				outfile << "<p> Nb Contours: " << el->GetText() << "</p>" << std::endl;
			}
		}
	}
	outfile << "</td>" << std::endl;
}

int main(int argc, char** argv)
{
	std::string inputdir = std::string(argv[1]);
	std::string outputfilename = std::string(argv[2]);
	std::string outputdata = outputfilename + "_data";
	CreateDirectory(outputdata.c_str(), NULL);

	std::vector<std::string> folders = readReportFolders(inputdir);

	std::ofstream outfile(outputfilename, std::ofstream::out);
	outfile << "<!DOCTYPE html>" << std::endl;
	outfile << "<html>" << std::endl;
	outfile << "<body>" << std::endl;
	outfile << "<table>" << std::endl;

	outfile << "<tr>" << std::endl;
	outfile << "<td>" << "<p> Number Reports: " << folders.size() << "</p>" << "</td>" << std::endl;
	outfile << "</tr>" << std::endl;

	//outfile << "<tr>" << std::endl;
	//generateNbContoursPlot(outfile, inputdir, folders, "90", outputdata);
	//generateNbContoursPlot(outfile, inputdir, folders, "95", outputdata);
	//generateNbContoursPlot(outfile, inputdir, folders, "98", outputdata);
	//outfile << "</tr>" << std::endl;

	for (int i = 0; i < folders.size(); i++)
	{
		std::cerr << "Processing " << folders[i] << std::endl;

		outfile << "<tr>" << std::endl;
		writeCellImage(outfile, inputdir, folders[i], "90", outputdata);
		writeCellImage(outfile, inputdir, folders[i], "95", outputdata);
		writeCellImage(outfile, inputdir, folders[i], "98", outputdata);
		outfile << "</tr>" << std::endl;
		outfile << "<tr>" << std::endl;
		writeCellData(outfile, inputdir, folders[i], "90", outputdata);
		writeCellData(outfile, inputdir, folders[i], "95", outputdata);
		writeCellData(outfile, inputdir, folders[i], "98", outputdata);
		outfile << "</tr>" << std::endl;
	}
	outfile << "</table>" << std::endl;
	outfile << "</html>" << std::endl;
	outfile << "</body>" << std::endl;
	outfile.close();
	return 1;
}

