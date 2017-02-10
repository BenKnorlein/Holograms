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


	strPattern = inputdir + "\\90\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = inputdir + "\\90\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string reportFile = strFilePath + "\\reportRaw_refined.xml";
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

void generateNbContoursPlot(std::ofstream &outfile, std::string inputdir, std::vector<std::string> reportnames, std::string threshold, std::string outputdata, bool MAC)
{
	std::vector<int> nbContours;
	for (int i = 0; i < reportnames.size(); i++)
	{
		std::string reportName = inputdir + "\\" + threshold + "\\" + reportnames[i] + "\\reportRaw_refined.xml";
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

void replaceAll(std::string &s, const std::string &search, const std::string &replace) {
	for (size_t pos = 0;; pos += replace.length()) {
		// Locate the substring to replace
		pos = s.find(search, pos);
		if (pos == std::string::npos) break;
		// Replace by erasing and inserting
		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}

void writeCellImage(std::ofstream &outfile, std::string inputdir, std::string reportname, std::string threshold, std::string outputdata, bool MAC)
{
	std::string outImageName = outputdata + "\\" + reportname + "_" + threshold + ".png";
	if(!MAC)copyFile(inputdir + "\\" + threshold + "\\" + reportname + "\\contoursSplat.png", outImageName);

	outfile << "<td >" << std::endl;
	if (!MAC){
		outfile << "<a href = \"" + inputdir + "\\" + threshold + "\\" + reportname + "\\report_refined.xml\">";
		outfile << "<img src = \"" << outImageName << "\" width = \"512\" height = \"512\">" << "</a>" << std::endl;
	}
	else
	{
		std::string MacInputdir = inputdir + "\\" + threshold + "\\" + reportname + "\\report_refined.xml";
		replaceAll(MacInputdir, "\\10.23.2.83", "Volumes");
		replaceAll(MacInputdir, "\\", "/");
		outfile << "<a href = \"" + MacInputdir + "\">";
		std::string MAcoutImageName = outImageName;
		replaceAll(MAcoutImageName, "\\10.23.2.83", "Volumes");
		replaceAll(MAcoutImageName, "\\", "/");
		outfile << "<img src = \"" << MAcoutImageName << "\" width = \"512\" height = \"512\">" << "</a>" << std::endl;
	}
	outfile << "</td>" << std::endl;
}


bool fileExists(std::string in)
{
#ifdef _MSC_VER	
	DWORD dwAttrib = GetFileAttributes(in.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
	return false;
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

void writeCellData(std::ofstream &outfile, std::string inputdir, std::string reportname, std::string threshold, std::string outputdata)
{
	outfile << "<td >" << std::endl;

	//std::vector<std::string> names;
	//std::vector<std::string> values;

	std::string reportName = inputdir + "\\" + threshold + "\\" + reportname + "\\reportRaw_refined.xml";
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
		tinyxml2::XMLElement* titleElement;
		titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");

		if (titleElement) {
			tinyxml2::XMLElement* el = titleElement->FirstChildElement("CONTOUR");
			if (el)
			{
				outfile << "<p>" << el->GetText() << "</p>" << std::endl;
			}

			outfile << "<p style=\"font-size:50%;\" > Threshold: 0." << threshold << "</p>" << std::endl;
			
			el = titleElement->FirstChildElement("NBCONTOURS");
			if (el)
			{
				outfile << "<p style=\"font-size:50%;\" > Nb Contours: " << el->GetText() << "</p>" << std::endl;
			}
		}
	}

	std::string dataFile = inputdir + "\\" + threshold + "\\" + reportname + "\\data\\" + reportname.substr(0, reportname.rfind(".")) + ".txt";
	if (fileExists(dataFile))
	{
		std::ifstream fin(dataFile);
		std::istringstream in;
		std::string line;
		while (!safeGetline(fin, line).eof())
		{
			outfile << "<p style=\"font-size:50%;\" >" << line << "</p>" << std::endl;
			line.clear();
		}
		fin.close();
	}

	outfile << "</td>" << std::endl;
}


void getDataFromTXT(std::string filename, std::vector<std::string> &names, std::vector<std::string> &values)
{
	if (fileExists(filename))
	{
		std::ifstream fin(filename);
		std::istringstream in;
		std::string line;
		std::string s;
		int count = 0;
		while (!safeGetline(fin, line).eof())
		{
			in.clear();
			in.str(line);
			std::vector<std::string> tmp;
			while (getline(in, s, ' ')) {
				tmp.push_back(s);
			}

			if (tmp.size() == 2)
			{
				names.push_back(tmp[0]);
				values.push_back(tmp[1]);
			}
			line.clear();
			tmp.clear();
			count++;
		}
		fin.close();
	}
}

void createCSV(std::string outputfolder, std::string inputdir, std::vector<std::string> &folders, std::string threshold)
{
	std::string csvname = outputfolder + "\\data" + threshold + ".csv";
	std::ofstream outfile(csvname, std::ofstream::out);
	for (int i = 0; i < folders.size(); i++)
	{
		std::string reportName = inputdir + "\\" + threshold + "\\" + folders[i] + "\\reportRaw_refined.xml";
		std::vector<std::string> values_name;
		std::vector<std::string> values;

		values_name.push_back("filename");
		values.push_back(folders[i]);
		values_name.push_back("time");
		values.push_back(folders[i].substr(folders[i].size() - 28, 24));
		values_name.push_back("Holofile#");
		values.push_back(std::to_string(i));

		std::string dataFile = inputdir + "\\" + threshold + "\\" + folders[i] + "\\data\\" + folders[i].substr(0, folders[i].rfind(".")) + ".txt";
		getDataFromTXT(dataFile, values_name, values);

		if (i == 0)
		{
			for (int v = 0; v < values.size(); v++)
			{
				outfile << values_name[v] << ",";
			}

			outfile << "ESD,ESV,X,Y,Z" << std::endl;
		}


		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(reportName.c_str()) == tinyxml2::XML_SUCCESS){
			tinyxml2::XMLElement* titleElement;
			titleElement = doc.FirstChildElement("doc")->FirstChildElement("DATA");

			if (titleElement) {
				
				for (tinyxml2::XMLElement* child = titleElement->FirstChildElement("ROI"); child != NULL; child = child->NextSiblingElement("ROI"))
				{
					for (int v = 0; v < values.size(); v++)
					{
						outfile << values[v] << ",";
					}
					if (child->FirstChildElement("ESD") && child->FirstChildElement("ESV") &&
						child->FirstChildElement("X") && child->FirstChildElement("Y") && child->FirstChildElement("DEPTH")){
						outfile << child->FirstChildElement("ESD")->GetText() << "," << child->FirstChildElement("ESV")->GetText() << ","
							<< child->FirstChildElement("X")->GetText() << "," << child->FirstChildElement("Y")->GetText() 
							<< "," << child->FirstChildElement("DEPTH")->GetText() << std::endl;
					}
					else
					{
						outfile << " " << "," << " " << std::endl;
					}
				}
			}
		}
	}
	outfile.close();
}

int main(int argc, char** argv)
{
	std::string inputdir = std::string(argv[1]);
	std::string outputfilename = std::string(argv[2]) + "\\report_overview.html";
	std::string outputfolder = std::string(argv[2]);
	std::string outputdata = outputfilename + "_data";

	CreateDirectory(outputfolder.c_str(), NULL);
	CreateDirectory(outputdata.c_str(), NULL);

	std::vector<std::string> folders = readReportFolders(inputdir);
	createCSV(outputfolder, inputdir,folders,"90");
	createCSV(outputfolder, inputdir, folders, "95");
	createCSV(outputfolder, inputdir, folders, "98");
	
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
		writeCellImage(outfile, inputdir, folders[i], "90", outputdata,false);
		writeCellImage(outfile, inputdir, folders[i], "95", outputdata, false);
		writeCellImage(outfile, inputdir, folders[i], "98", outputdata, false);
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

	outputfilename = std::string(argv[2]) + "\\report_overview_MAC.html";
	outfile = std::ofstream(outputfilename, std::ofstream::out);
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
		writeCellImage(outfile, inputdir, folders[i], "90", outputdata, true);
		writeCellImage(outfile, inputdir, folders[i], "95", outputdata, true);
		writeCellImage(outfile, inputdir, folders[i], "98", outputdata, true);
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

