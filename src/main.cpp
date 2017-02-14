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

#define NOMINMAX

#include "ImageCache.h"
#include "OctopusClient.h"
#include "OfflineReader.h"
#include "ReportWriter.h"
#include "Settings.h"
#include "ContourDetection.h"
#include "PhaseExperiments.h"

#include <iostream>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS

#ifndef WITH_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>         // open
#include <sys/sendfile.h>  // sendfile
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
#include "ContourMerge.h"

using namespace cv;

std::string slash = "/";

void copyFile(std::string in, std::string out)
{
#ifdef _MSC_VER	
	CopyFile(in.c_str(), out.c_str(), FALSE);
#else
	int source = open(in.c_str(), O_RDONLY, 0);
	if(source > 0){
		int dest = open(out.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);

		// struct required, rationale: function stat() exists also
		struct stat stat_source;
		fstat(source, &stat_source);

		sendfile(dest, source, 0, stat_source.st_size);

		close(source);
		close(dest);
	}
#endif
}

bool has_suffix(const std::string &str, const std::string &suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int DeleteDirectory(const std::string &refcstrRootDirectory)
{
#ifdef _MSC_VER	
	bool            bSubdirectory = false;       // Flag, indicating whether
	// subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Delete subdirectory
					int iRC = DeleteDirectory(strFilePath);
					if (iRC)
						return iRC;;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(refcstrRootDirectory.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}
#else
	DIR *dir;
	struct dirent *entry;
	char path[256];

	if (path == NULL) {
		return 0;
	}
	dir = opendir(refcstrRootDirectory.c_str());
	if (dir == NULL) {
		perror("Error opendir()");
		return 0;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			snprintf(path, (size_t)PATH_MAX, "%s/%s", refcstrRootDirectory.c_str(), entry->d_name);
			if (entry->d_type == DT_DIR) {
				DeleteDirectory(std::string(path));
			}

			unlink(path);
		}

	}
	closedir(dir);

	rmdir(refcstrRootDirectory.c_str());
#endif
	return 0;
}

int countXML(std::string dir)
{
	int files = 0;
#ifdef _MSC_VER	
	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::string search = dir + "\\*.xml";
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
		if (has_suffix(std::string(dirp->d_name), ".xml")) {
			std::cerr << std::string(dirp->d_name) << std::endl;
			files++;
		}
	}
	closedir(dp);
#endif
	return files;
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
	std::string outDirData = settings->getOutputFolder() + slash + filename + slash + "data";
#ifdef _MSC_VER
	CreateDirectory(outFile.c_str(), NULL);
	CreateDirectory(outDirData.c_str(), NULL);
#else
	mkdir(outFile.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(outDirData.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	std::string inDirData = settings->getDatafolder() + slash + filename + slash + "data";
	size_t lastindex = filename.find_last_of(".");
	std::string rawfilename = filename.substr(0, lastindex);
	copyFile(inDirData + slash + filename, outDirData + slash + filename);
	copyFile(inDirData + slash + rawfilename + ".xml", outDirData + slash + rawfilename + ".xml");
	copyFile(inDirData + slash + rawfilename + ".txt", outDirData + slash + rawfilename + ".txt");
	copyFile(inDirData + slash + rawfilename + "_diff.png", outDirData + slash + rawfilename + "_diff.png");
	copyFile(inDirData + slash + rawfilename + "_background.bmp", outDirData + slash + rawfilename + "_background.bmp");
	copyFile(inDirData + slash + "Settings_Writing.xml", outDirData + slash + "Settings_Writing.xml");

	//setup ReportWriter
	//ReportWriter * writer = new ReportWriter(settings->getOutputFolder(), filename);
	ReportWriter *writer = new ReportWriter(settings, filename);
	//setup ImageCache
	ImageCache * cache;
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
	std::vector<Contour *> contours;

	detector->generateMaxMap();
	writer->saveImage(*detector->getMaxImage(), "maximum.png", true);

	detector->findContours(contours);
	writer->saveImage(detector->getDepthImage(), "depthImage.png");

	delete detector;
	writer->saveContourImage(contours, settings);

	////////Find best depth for contour
	ContourDepthDetection * depthdetector = new ContourDepthDetection(cache, settings);
	for (int i = 0; i < contours.size(); i++)
		depthdetector->findBestDepth(contours[i], settings->getMinDepth(), settings->getMaxDepth(), settings->getStepSize());
	delete depthdetector;

	//merge bounds
	if (settings->getDoMergebounds()){
		int count = 1;
		while (count > 0){
			ContourMerge * merger = new ContourMerge(settings);
			count = merger->mergeContours(contours);
			delete merger;
			//find best depth for contours if a merge occured
			if (count > 0){
				ContourDepthDetection * depthdetector = new ContourDepthDetection(cache, settings);
				for (int i = 0; i < contours.size(); i++) {
					depthdetector->findBestDepth(contours[i], settings->getMinDepth(), settings->getMaxDepth(), settings->getStepSize());
				}
				delete depthdetector;
			}
		}
	}

	///////Refine depths
	if (settings->getDoRefine() && settings->getOnline())
	{
		ContourDepthDetection * depthdetector = new ContourDepthDetection(cache, settings);
		for (int i = 0; i < contours.size(); i++)
			depthdetector->findBestDepth(contours[i], contours[i]->getDepth() - settings->getStepSize(), contours[i]->getDepth() + settings->getStepSize(), settings->getStepSize() / 10.0);
		delete depthdetector;
	}

	////////Phase Experimetal Testing	
	//PhaseExperiments *pe = new PhaseExperiments(settings, cache, outFile);
	//pe->randomContourPixels(contours[1], 5, 10000, 12000, 100);
	//pe->randomContourPixels(contours[3], 10, 10000, 12000, 100);
	//delete pe;

	////////Create Report
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	writer->writeXMLReport(contours, std::chrono::duration_cast<std::chrono::minutes>(end - begin).count());
	writer->writeRawReport(contours, std::chrono::duration_cast<std::chrono::minutes>(end - begin).count());
	writer->saveROIImages(cache, contours);
	writer->saveContourImage(contours, settings);
	writer->writeSplatImage(contours, cache);

	///////remove files
	if (settings->getDeleteTemporary())
	{
		unlink(argv[2]);
		if (countXML(settings->getDatafolder() + slash + filename) == 0)
		{
			std::cerr << "Deleting" << std::endl;
			DeleteDirectory(settings->getDatafolder() + slash + filename);
		}
	}

	////////Cleanup
	delete settings;
	delete cache->getImageSource();
	delete cache;
	delete writer;
	return 0;
}

