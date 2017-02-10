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

	void writeRawReport(std::vector<Contour*> contours, double time);
	void writeXMLReport(std::vector<Contour*> contours, double time);
	void writeSplatImage(std::vector<Contour*> contours, ImageCache* cache);
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
