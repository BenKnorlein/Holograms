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

	while ((dirp = readdir(dp)) != NULL && has_suffix(std::string(dirp->d_name), ".bmp") ) {
		files++;
	}									
	closedir(dp);
#endif
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

	while ((dirp = readdir(dp)) != NULL && has_suffix(std::string(dirp->d_name), ".bmp")) {
		files.push_back(std::string(dirp->d_name));
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

int main(int argc, char** argv)
{
	if (argc < 5)
	{
		std::cerr << "You need to provide: input directory, Images used for median, step size and at least 1 output directory " << std::endl;
		return 0;
	}

	std::string inputdir = std::string(argv[1]);
	int nbMedian = std::atoi(argv[2]);
	int step = std::atoi(argv[3]);
	std::vector<std::string> outputs;
	for (int i = 0; i < argc - 4; i++)
	{
		outputs.push_back( std::string(argv[i+4]));
		std::cerr << "Save to " << outputs[i] << std::endl;
	}

	std::cerr << std::endl;

	std::vector<std::string> files = readBMP(inputdir);
	
	int current = 0;
	while (current + nbMedian <= files.size()){
		std::vector<cv::Mat> images;
		for (int i = 0; i < nbMedian; i++)
		{
			std::string filename_tmp = inputdir + slash + files[current + i];
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
				//std::cerr << (int)im_out.at<cv::Vec3b>(y, x)[0] << " " << (int)im_out.at<cv::Vec3b>(y, x)[1] << " " << (int)im_out.at<cv::Vec3b>(y, x)[2] << std::endl;

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

		int middle = current + nbMedian / 2;

		std::string outdir;
		int outDirMin = std::numeric_limits<int>::max();

		for (int i = 0; i < outputs.size(); i++)
		{
			int count = countFiles(outputs[i]);
			if (count < outDirMin)
			{
				outDirMin = count;
				outdir = outputs[i];
			}
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
		current += step;
		std::cerr << std::endl;
	}
	return 1;
}

