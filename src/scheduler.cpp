///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#include <iostream>
#include "tinyxml2.h"
#include <vector>
#include <algorithm>
#include <string>

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

std::string slash = "/"; 

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

std::vector <std::string> readXML(std::string folder)
{
	std::vector<std::string> files;
#ifdef _MSC_VER	
	HANDLE hFind;
	WIN32_FIND_DATA data;	
	std::string search = folder + "\\*.xml";
	hFind = FindFirstFile(search.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (hasEnding(data.cFileName, ".xml"))
				files.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	std::sort(files.begin(), files.end());
#endif
	return files;
}

void makeDirectory(std::string folder)
{
#ifdef _MSC_VER	
	CreateDirectory(folder.c_str(), NULL);
#else
	mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif#endif
}

void moveFile(std::string in, std::string out)
{
#ifdef _MSC_VER	
	MoveFile(in.c_str(), out.c_str());
#endif
}

void copyFile(std::string in, std::string out)
{
#ifdef _MSC_VER	
	CopyFile(in.c_str(), out.c_str(), FALSE);
#endif
}

void deleteFile(std::string in)
{
#ifdef _MSC_VER	
	DeleteFile(in.c_str());
#endif
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
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "You need to provide: input directory" << std::endl;
		return 0;
	}

	int serverA = 0;

	std::string inputdir = std::string(argv[1]);
	std::string settings = std::string(argv[2]);
	int nbServers = std::atoi(argv[3]);
	std::cerr << nbServers << " servers" << std::endl;
	while (1){
		std::vector<std::string> files = readXML(inputdir);
		if (files.size() > 0)
		{
			std::string filename;
			std::string backgroundFilename;
			std::string outdir;
			bool exists = false;
			std::string xml_file = inputdir + "//" + files[0];
			std::cerr << "Process " << xml_file.c_str() << std::endl;
			tinyxml2::XMLDocument doc;
			if (doc.LoadFile(xml_file.c_str()) == tinyxml2::XML_SUCCESS){
				tinyxml2::XMLElement* titleElement;

				titleElement = doc.FirstChildElement("Processing")->FirstChildElement("Step1")->FirstChildElement("Image");
				if (titleElement)
				{
					filename = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
				}

				titleElement = doc.FirstChildElement("Processing")->FirstChildElement("Step1")->FirstChildElement("Background");
				if (titleElement)
				{
					backgroundFilename = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
				}

				tinyxml2::XMLDocument doc2;
				if (doc2.LoadFile(settings.c_str()) == tinyxml2::XML_SUCCESS){
					tinyxml2::XMLElement* titleElement;

					titleElement = doc2.FirstChildElement("Settings")->FirstChildElement("outputFolder");
					if (titleElement) {
						outdir = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
					}
				}

				std::cerr << "Check if exists " << outdir + "//" + filename + "//" + "data" + "//" + files[0] << std::endl;
				exists = fileExists(outdir + "//" + filename + "//" + "data" + "//" + files[0]);

				if (!exists){
					std::string command;

					command = "writeImages.exe " + filename + " " + settings + " " + backgroundFilename + " " + std::to_string(serverA);
					std::cerr << "Running : " << command << std::endl;
					system(command.c_str());

					outdir = outdir + "//" + filename + "//" + "data";
					makeDirectory(outdir);

					copyFile(settings, outdir + "//Settings_Writing.xml");
					moveFile(inputdir + "//" + files[0], outdir + "//" + files[0]);
					moveFile(inputdir + "//" + filename, outdir + "//" + filename);
					moveFile(inputdir + "//" + filename.substr(0, filename.rfind(".")) + ".csv", outdir + "//" + filename.substr(0, filename.rfind(".")) + ".csv");
					moveFile(inputdir + "//" + backgroundFilename, outdir + "//" + backgroundFilename);

					serverA++;
					serverA = serverA % nbServers;
					std::cerr << "Done" << std::endl;
				}
				else
				{
					std::cerr << "File exists " << outdir + "//" + filename + "//" + "data" + "//" + files[0] << std::endl;
					std::cerr << "Delete data" << std::endl;
					deleteFile(inputdir + "//" + files[0]);
					deleteFile(inputdir + "//" + filename);
					deleteFile(inputdir + "//" + filename.substr(0, filename.rfind(".")) + ".csv");
					deleteFile(inputdir + "//" + backgroundFilename);
				}
			}
		}
		else{
			Sleep(1000);
		}
	}
	return 1;
}

