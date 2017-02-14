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
#include <fstream>

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif

	#include <windows.h>
	#define THREAD_SAVING 
	#include "ThreadedImageSaver.h"
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

std::string slash = "/"; 

void copyFile(std::string in, std::string out)
{
#ifdef _MSC_VER	
	CopyFile(in.c_str(), out.c_str(), FALSE);
#endif
}

void writeImages(std::string filename, Settings * settings, std::string background = "")
{
#ifdef _MSC_VER
	ThreadedImageSaver* saver = new ThreadedImageSaver(settings);
#endif

	int width = settings->getWidth();
	int height = settings->getHeight();
	std::string datafolder = settings->getDatafolder();
	int start = settings->getMinDepth();
	int stop = settings->getMaxDepth();
	int step_width = settings->getStepSize();

	std::string outFile = settings->getOutputFolder() + slash + filename;
#ifdef _MSC_VER
	CreateDirectory(settings->getOutputFolder().c_str(), NULL);
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(settings->getOutputFolder().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	OctopusClient * client = new OctopusClient(settings->getIp(), settings->getPort());

	std::cout << "Loading " << datafolder + "/" + filename << std::endl;
	if (background != ""){
		std::cout << "Use Background " << datafolder + "/" + background << std::endl;
	}
	
	Sleep(2000);
	if (!client->setSourceHologram(datafolder + "/", filename, background))exit;
	Sleep(2000);
	std::string name;
	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save amplitude " << d << std::endl;
		float* data = new float[width * height];
		client->getAmplitudeImage(d, data);

		if (settings->getSavePngImages()){
			cv::Mat image(cv::Size(width, height), CV_32FC1, data);

			cv::Mat image_disp;
			cv::Mat B;
			normalize(image, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);

			imwrite(outFile + slash + "Amplitude_" + std::to_string(d) + ".png", B);

			image.release();
		}

		name = outFile + slash + "Amplitude_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
		saver->add(name, data);
#else
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);
		delete[] data;
#endif
		
	}

	if (settings->getSaveIntensity()){
		for (int d = start; d <= stop; d += step_width){
			//std::cerr << "Save Intensity " << d << std::endl;
			float* data = new float[width * height];
			client->getIntensityImage(d, data);
	
			if (settings->getSavePngImages()){
				cv::Mat image(cv::Size(width, height), CV_32FC1, data);

				cv::Mat image_disp;
				cv::Mat B;
				normalize(image, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);

				imwrite(outFile + slash + "Intensity_" + std::to_string(d) + ".png", B);

				image.release();
			}

			name = outFile + slash + "Intensity_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
			saver->add(name, data);
#else
			FILE* file = fopen(name.c_str(), "wb");
			fwrite(data, sizeof(float), width * height, file);
			fclose(file);
			delete[] data;
#endif
		}
	}

	if (settings->getSavePhase()){
		//std::cout << "Loading " << datafolder + "/" + filename << std::endl;
		if (!client->setSourceHologram(datafolder + "/", filename))exit;

		for (int d = start; d <= stop; d += step_width){
			//std::cerr << "Save phase " << d << std::endl;
			float* data = new float[width * height];
			client->getPhaseImage(d, data);

			if (settings->getSavePngImages()){
				cv::Mat image(cv::Size(width, height), CV_32FC1, data);

				cv::Mat image_disp;
				cv::Mat B;
				normalize(image, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);

				imwrite(outFile + slash + "Phase_" + std::to_string(d) + ".png", B);

				image.release();
			}

			name = outFile + slash + "Phase_" + std::to_string(d) + ".ext";
#ifdef THREAD_SAVING
			saver->add(name, data);
#else
			FILE* file = fopen(name.c_str(), "wb");
			fwrite(data, sizeof(float), width * height, file);
			fclose(file);
			delete[] data;
#endif
		}
	}

#ifdef _MSC_VER
	delete saver;
#endif
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	Settings * settings = new Settings(argv[2]);

	std::string background = "";
	int serverA = 0;
	if (argc >= 4) background = std::string(argv[3]);
	if (argc >= 5) serverA = std::atoi(argv[4]);

	
	writeImages(filename, settings, background);

	copyFile("Z://FK170124//holograms//Settings//Settings_90.xml", settings->getOutputFolder() + slash + filename + slash + "//Settings_90.xml");
	copyFile("Z://FK170124//holograms//Settings//Settings_95.xml", settings->getOutputFolder() + slash + filename + slash + "//Settings_95.xml");
	copyFile("Z://FK170124//holograms//Settings//Settings_98.xml", settings->getOutputFolder() + slash + filename + slash + "//Settings_98.xml");

	if (settings->getStartRemote()){	
		std::string command;
		if (serverA == 0){
			command = "plink SOI1 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_90_direct.xml &> /dev/null &\"";
			system(command.c_str());
			command = "plink SOI2 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_95_direct.xml &> /dev/null &\"";
			system(command.c_str());
			command = "plink SOI3 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_98_direct.xml &> /dev/null &\"";
			system(command.c_str());
		}
		else
		{
			command = "plink SOI4 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_90_direct.xml &> /dev/null &\"";
			system(command.c_str());
			command = "plink SOI5 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_95_direct.xml &> /dev/null &\"";
			system(command.c_str());
			command = "plink SOI6 \"sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_98_direct.xml &> /dev/null &\"";
			system(command.c_str());
		}
	}
	else
	{
		std::string outbatchFile = settings->getOutputFolder() + slash + "batch" + std::to_string(serverA);
		std::cerr << "Write to " << outbatchFile << std::endl;
		std::ofstream outfile(outbatchFile, std::ofstream::out | std::ofstream::app);
		outfile << "sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_90.xml" << std::endl;
		outfile << "sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_95.xml" << std::endl;
		outfile << "sudo /export/users/ubuntu/holograms/Holograms/build/bin/Holograms " + filename + " /data2/FK170124/holograms/temporaryData/" + filename + "/Settings_98.xml" << std::endl;
		outfile.close();
	}

	delete settings;
	return 1;
}

