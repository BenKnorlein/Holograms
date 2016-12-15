///\file ReportWriter.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef REPORTWRITER_H
#define REPORTWRITER_H

#include <string>
#include "ImageCache.h"

#include <opencv2/core/core.hpp>
#include "Contour.h"

class Settings;

class ReportWriter {
public:
	//ReportWriter(std::string outdir, std::string filename, std::string templateFolder = "");	
	ReportWriter(Settings *settings, std::string filename);
	~ReportWriter();

	void writeXMLReport(std::vector<Contour*> contours, double time);
	void saveROIImages(ImageCache* cache, std::vector<Contour*> contours);
	void saveImage(cv::Mat image, std::string filename, bool normalizeImage = false);
	void saveContourImage(std::vector<Contour*> contours, Settings * settings);
	float reconPixelSize(int depth);

private:
	std::string m_outdir;
	std::string m_filename;
	std::string m_templateFolder;
	std::string m_QCfolder;

	int m_pixel_size;
	int m_screen_to_source;
	int m_width;
	int m_height;
	
	Settings *m_settings;

	template <typename T>
	cv::Mat plotGraph(std::vector<T>& vals, int YRange[2]);
};

#endif //REPORTWRITER_H
