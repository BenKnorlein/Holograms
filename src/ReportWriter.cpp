///\file ReportWriter.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "ReportWriter.h"
#include "Settings.h"
#include <iostream>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo.hpp>
#include "tinyxml2.h"

template <typename T>
cv::Mat ReportWriter::plotGraph(std::vector<T>& vals, int YRange[2])
{

	auto it = minmax_element(vals.begin(), vals.end());
	float scale = 1. / ceil(*it.second - *it.first);
	float bias = *it.first;
	int rows = YRange[1] - YRange[0] + 1;
	cv::Mat image = cv::Mat(rows, vals.size(), CV_8UC1,cv::Scalar(255));
	for (int i = 0; i < (int)vals.size() - 1; i++)
	{
		cv::line(image, cv::Point(i, rows - 1 - (vals[i] - bias)*scale*YRange[1]), cv::Point(i + 1, rows - 1 - (vals[i + 1] - bias)*scale*YRange[1]), cv::Scalar(0, 0, 0), 1);
	}

	return image;
}

ReportWriter::ReportWriter(Settings *settings, std::string filename)
{
	m_outdir = settings->getOutputFolder() + "/" + filename;
	m_filename = filename;
	m_templateFolder = settings->getTemplateFolder();
	m_QCfolder = settings->getQCfolder();
	m_pixel_size = settings->getPixelSize();
	m_screen_to_source = settings->getScreenToSource();
	m_width = settings->getWidth();
	m_height = settings->getHeight();
	m_settings = settings;
}

ReportWriter::~ReportWriter()
{
	
}

void ReportWriter::writeXMLReport(std::vector<Contour*> contours, double time)
{
	std::ifstream infile;
	std::string templateFile = "template.xml";
	if (!m_templateFolder.empty()) templateFile = m_templateFolder + "/" + templateFile;

	infile.open(templateFile, std::ifstream::in);

	char * buffer;
	long size;

	// get size of file
	infile.seekg(0);
	std::streampos fsize = 0;
	infile.seekg(0, infile.end);
	size = infile.tellg() - fsize;
	infile.seekg(0);

	// allocate memory for file content
	buffer = new char[size];

	// read content of infile
	infile.read(buffer, size);

	infile.close();

	std::ofstream outfile(m_outdir + "/" + "report.xml", std::ofstream::out);
	// write to outfile
	std::string buffer_st(buffer);
	outfile.write(buffer, size);
	delete[] buffer;

	std::ofstream qcfile();
	tinyxml2::XMLDocument doc;
	std::string QCfilename = m_QCfolder + "/" + "holoyurt-qc-data.xml";

	if (doc.LoadFile(QCfilename.c_str()) == tinyxml2::XML_SUCCESS){
		outfile << "<QCDATA>" << std::endl;

		tinyxml2::XMLElement* root;
		root = doc.FirstChildElement("QCCONTAINER");
		for (tinyxml2::XMLElement* e = root->FirstChildElement("HOLOGRAM"); e != NULL; e = e->NextSiblingElement("HOLOGRAM"))
		{
			if (std::string(e->FirstChildElement("FILENAME")->GetText()) == m_filename)
			{
				tinyxml2::XMLElement* roi = e->FirstChildElement("ROI");
				outfile << "<ROI>" << std::endl;

				outfile << "<X>" << roi->FirstChildElement("X")->GetText() << "</X>" << std::endl;
				outfile << "<Y>" << roi->FirstChildElement("Y")->GetText() << "</Y>" << std::endl;

				outfile << "<MAJORAXIS>" << roi->FirstChildElement("MAJORAXIS")->GetText() << "</MAJORAXIS>" << std::endl;
				outfile << "<MINORAXIS>" << roi->FirstChildElement("MINORAXIS")->GetText() << "</MINORAXIS>" << std::endl;
				if (roi->FirstChildElement("TYPE")->GetText() != NULL){
					outfile << "<TYPE>" << roi->FirstChildElement("TYPE")->GetText() << "</TYPE>" << std::endl;
				} else
				{
					outfile << "<TYPE>" << "</TYPE>" << std::endl;
				}

				outfile << "<DEPTH>" << roi->FirstChildElement("DEPTH")->GetText() << "</DEPTH>" << std::endl;

				outfile << "<IMAGE>" << roi->FirstChildElement("IMAGE")->GetText() << "</IMAGE>" << std::endl;

				outfile << "</ROI>" << std::endl;

				std::ifstream  src(m_QCfolder + "/" + roi->FirstChildElement("IMAGE")->GetText(), std::ios::binary);
				std::ofstream  dst(m_outdir + "/" + roi->FirstChildElement("IMAGE")->GetText(), std::ios::binary);

				dst << src.rdbuf();
				src.close();
				dst.close();
			}
		}

		outfile << "</QCDATA>" << std::endl;
	}


	outfile << "<DATA>" << std::endl;
	outfile << "<FILENAME>" << m_filename << "</FILENAME>" << std::endl;
	outfile << "<CONTOURIMAGE>contours.png</CONTOURIMAGE>" << std::endl;
	outfile << "<MAXIMAGE>maximum.png</MAXIMAGE>" << std::endl;
	outfile << "<DEPTHIMAGE>depthImage.png</DEPTHIMAGE>" << std::endl;
	outfile << "<TIME>" << time << "</TIME>" << std::endl;

	for (size_t c = 0; c < contours.size(); c++)
	{
		float x_pixel = (contours[c]->getBoundingBox().tl() + contours[c]->getBoundingBox().br()).x * 0.5;
		float y_pixel = (contours[c]->getBoundingBox().tl() + contours[c]->getBoundingBox().br()).y * 0.5;
		int width_pixel = contours[c]->getBoundingBox().width;
		int height_pixel = contours[c]->getBoundingBox().height;
		int area_pixel = contours[c]->getArea();
		int depth = contours[c]->getDepth();
		float scalar = reconPixelSize(depth);
		float x = (x_pixel - m_width * 0.5) * scalar;
		float y = (y_pixel - m_width * 0.5) * scalar;
		float width = width_pixel * scalar;
		float height = height_pixel * scalar;
		float area = area_pixel * scalar * scalar;

		outfile << "<ROI>" << std::endl;
		outfile << "<CONTOUR>" << c << "</CONTOUR>" << std::endl;
		
		outfile << "<X_PIXEL>" << x_pixel << "</X_PIXEL>" << std::endl;
		outfile << "<X>" << x << "</X>" << std::endl;
		outfile << "<Y_PIXEL>" << y_pixel << "</Y_PIXEL>" << std::endl;
		outfile << "<Y>" << y << "</Y>" << std::endl;
		
		outfile << "<WIDTH_PIXEL>" << width_pixel << "</WIDTH_PIXEL>" << std::endl;
		outfile << "<WIDTH>" << width << "</WIDTH>" << std::endl;
		outfile << "<HEIGHT_PIXEL>" << height_pixel << "</HEIGHT_PIXEL>" << std::endl;
		outfile << "<HEIGHT>" << height << "</HEIGHT>" << std::endl;
		
		outfile << "<DEPTH>" << depth  << "</DEPTH>" << std::endl;
		
		outfile << "<AREA_PIXEL>" << area_pixel << "</AREA_PIXEL>" << std::endl;
		outfile << "<AREA>" << area << "</AREA>" << std::endl;
		
		outfile << "<MAXVAL>" << contours[c]->getMaxValue() << "</MAXVAL>" << std::endl;
		outfile << "<MINVAL>" << *std::min_element(contours[c]->getValues()->begin(), contours[c]->getValues()->end()) << "</MINVAL>" << std::endl;
		outfile << "<IMAGE>" << "contoursMasked_" + std::to_string(((long long)c)) + ".png" << "</IMAGE>" << std::endl;
		outfile << "<IMAGEPHASE>" << "contoursPhase_" + std::to_string(((long long)c)) + ".png" << "</IMAGEPHASE>" << std::endl;
		outfile << "<IMAGESCORE>" << "contoursScore_" + std::to_string(((long long)c)) + ".png" << "</IMAGESCORE>" << std::endl;
		outfile << "</ROI>" << std::endl;

		int bounds[2] = { 0, 50 };
		cv::Mat plot = plotGraph(*contours[c]->getValues(), bounds);
		cv::imwrite(m_outdir + "/" + "contoursScore_" + std::to_string(((long long)c)) + ".png", plot);
	}
	outfile << "</DATA>" << std::endl;
	
	outfile << "<SETTINGS>" << std::endl;
	outfile << "  <DF>" << m_settings->getDatafolder() << "</DF>\n";
        outfile << "  <OF>" << m_settings->getOutputFolder() << "</OF>\n";
        outfile << "  <TF>" << m_settings->getTemplateFolder() << "</TF>\n";
        outfile << "  <QCF>" << m_settings->getQCfolder() << "</QCF>\n";
        outfile << "  <ONLINE>" << m_settings->getOnline() << "</ONLINE>\n";
        outfile << "  <IP>" << m_settings->getIp() << "</IP>\n";
        outfile << "  <PORT>" << m_settings->getPort() << "</PORT>\n";
        outfile << "  <SHOW>" << m_settings->getShow() << "</SHOW>\n";
        outfile << "  <MICS>" << m_settings->getMaxImageCacheStorage() << "</MICS>\n";
        outfile << "  <SHARP>" << m_settings->getUseSharpness() << "</SHARP>\n";
        outfile << "  <SHARP_METHOD>" << m_settings->getMethodSharpness() << "</SHARP_METHOD>\n";
        outfile << "  <ABS>" << m_settings->getUseAbs() << "</ABS>\n";
        outfile << "  <STEP>" << m_settings->getStepSize() << "</STEP>\n";
        outfile << "  <MIN>" << m_settings->getMinDepth() << "</MIN>\n";
        outfile << "  <MAX>" << m_settings->getMaxDepth() << "</MAX>\n";
        outfile << "  <WIDTH>" << m_settings->getWidth() << "</WIDTH>\n";
        outfile << "  <HEIGHT>" << m_settings->getHeight() << "</HEIGHT>\n";
        outfile << "  <REFINE>" << m_settings->getDoRefine() << "</REFINE>\n";
        outfile << "  <WINDOW>" << m_settings->getWindowsize() << "</WINDOW>\n";
        outfile << "  <MAX_THRESH>" << m_settings->getMaxThreshold() << "</MAX_THRESH>\n";
        outfile << "  <MIN_AREA>" << m_settings->getContourMinArea() << "</MIN_AREA>\n";
        outfile << "  <MERGE>" << m_settings->getDoMergebounds() << "</MERGE>\n";
        outfile << "  <MERGE_DEPTH>" << m_settings->getMergeThresholdDepth() << "</MERGE_DEPTH>\n";
        outfile << "  <MERGE_DIST>" << m_settings->getMergeThresholdDist() << "</MERGE_DIST>\n";
        outfile << "  <SCOPE>" << m_settings->getMicroscope() << "</SCOPE>\n";
	outfile << "  <STS>" << m_settings->getScreenToSource() << "</STS>\n";
        outfile << "  <PS>" << m_settings->getPixelSize() << "</PS>\n";	
	outfile << "</SETTINGS>" << std::endl;

	outfile << "</doc>" << std::endl;
	outfile.close();
}

std::string type2str(int type) {
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

void ReportWriter::saveROIImages(ImageCache* cache, std::vector<Contour*> contours)
{
	for (size_t c = 0; c < contours.size(); c++)
	{
		std::cout << "Save Contour " << c << " at depth " << std::to_string(((long long)contours[c]->getDepth())) << std::endl;
		int d = contours[c]->getDepth();
		cv::Mat *image = cache->getPhaseImage(d);

		cv::Rect bound_cont = contours[c]->getBoundingBox();
		bound_cont.x = bound_cont.x - 20;
		bound_cont.y = bound_cont.y - 20;
		bound_cont.width = bound_cont.width + 40;
		bound_cont.height = bound_cont.height + 40;

		if (bound_cont.x < 0) bound_cont.x = 0;
		if (bound_cont.y < 0) bound_cont.y = 0;
		if (bound_cont.y + bound_cont.height > image->rows) bound_cont.height = image->rows - bound_cont.y;
		if (bound_cont.x + bound_cont.width > image->cols) bound_cont.width = image->cols - bound_cont.x;

		cv::Mat roi = (*image)(bound_cont);

		cv::Mat image_display;
		cv::Mat drawing;
		normalize(roi, image_display, 0, 255, CV_MINMAX);
		image_display.convertTo(drawing, CV_8U);

		cv::imwrite(m_outdir + "/" + "contoursPhase_" + std::to_string(((long long)c)) + ".png", drawing);
	}

	for (size_t c = 0; c < contours.size(); c++)
	{
		std::cout << "Save Contour " << c << " at depth " << std::to_string(((long long)contours[c]->getDepth())) << std::endl;
		int d = contours[c]->getDepth();

		cv::Mat *image = cache->getAmplitudeImage(d);
		cv::Mat *mask = contours[c]->getMask();

		cv::Rect bound_orig = contours[c]->getBoundingBox();
		cv::Rect bound_cont = cv::Rect(bound_orig.tl(), bound_orig.br());
		
		bound_cont.x = bound_cont.x - 20;
		bound_cont.y = bound_cont.y - 20;
		bound_cont.width = bound_cont.width + 40;
		bound_cont.height = bound_cont.height + 40;
		
		if (bound_cont.x < 0)bound_cont.x = 0;
		if (bound_cont.y < 0) bound_cont.y = 0;
		if (bound_cont.y + bound_cont.height > image->rows) bound_cont.height = image->rows - bound_cont.y;
		if (bound_cont.x + bound_cont.width > image->cols) bound_cont.width = image->cols - bound_cont.x;

		cv::Mat roi = (*image)(bound_cont);

		cv::Mat image_display;
		cv::Mat drawing;
		normalize(roi, image_display, 0, 255, CV_MINMAX);
		image_display.convertTo(drawing, CV_8U);

		cv::Mat id_mask;
		cv::Mat d_mask;
		normalize(*mask, id_mask, 0, 255, CV_MINMAX);
		id_mask.convertTo(d_mask, CV_8U);
		
		/* 
 		 * in this section below I create a new Mat that is BGR because we had trouble
 		 * converting the grayscale image directly to color.  I copy every pixel over
 		 * and tint any pixels that are not included in the contour.  Then I draw the 
 		 * two principle axes in green and blue and then save the image.  It is not
 		 * yet added to the report but could be. As of now it mostly serves as how to
 		 * create the image an draw on it. Once we discuss exactly what we want on the 
 		 * report I will chage it such that the reports contain the new and more useful
 		 * contour images.
 		 */
		if (1)
		{
			cv::Mat m = cv::Mat::zeros(bound_cont.height, bound_cont.width, CV_8UC3);
			for (int row = 0; row < m.rows; row++) {
				for (int col = 0; col < m.cols; col++) {
					unsigned char d = drawing.at<unsigned char>(row, col) ;

					cv::Vec3b pixel;
					pixel[0] = d;
					pixel[1] = d;
					pixel[2] = d;
					
					cv::Point p = cv::Point(col, row) + bound_cont.tl();
					if (!bound_orig.contains(p)) {
						pixel[2] = 255 * 0.3 + 0.7 * pixel[2];
					} else {
						cv::Point p2 = bound_orig.tl() - bound_cont.tl();	
						unsigned char d2 = d_mask.at<unsigned char>(row - p2.y, col - p2.x);
						if (d2 == 0) {
							pixel[2] = 255 * 0.3 + 0.7 * pixel[2];
						}	
					}
					
					m.at<cv::Vec3b>(row, col) = pixel;
				}
			}

			//cv::Point2d center = contours[c]->getPCACenter() - cv::Point2d(bound_cont.x, bound_cont.y);
			//cv::Point2d p1a = center + contours[c]->getPCAAxis()[0];
			//cv::Point2d p1b = center - contours[c]->getPCAAxis()[0];
			//cv::Point2d p2a = center + contours[c]->getPCAAxis()[1];
			//cv::Point2d p2b = center - contours[c]->getPCAAxis()[1];
			//cv::line(m, p1a, p1b, cv::Scalar(0, 255, 0), 1, CV_AA);
			//cv::line(m, p2a, p2b, cv::Scalar(255, 0, 0), 1, CV_AA);
			
			cv::imwrite(m_outdir + "/" + "contoursMasked_" + std::to_string(((long long)c)) + ".png", m);
		}
		
		
		cv::imwrite(m_outdir + "/" + "contours_" + std::to_string(((long long)c)) + ".png", drawing);
	}
}

void ReportWriter::saveImage(cv::Mat image, std::string filename, bool normalizeImage)
{
	if (normalizeImage){
		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(m_outdir + "/" + filename, B);
	} 
	else
	{
		imwrite(m_outdir + "/" + filename, image);
	}
}

void ReportWriter::saveContourImage(std::vector<Contour*> contours, Settings * settings)
{
	cv::RNG rng(12345);
	cv::Mat drawing = cv::Mat::zeros(cv::Size(settings->getWidth(), settings->getHeight()), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		for (std::vector<cv::Point>::iterator it = contours[i]->getPoints()->begin(); it < contours[i]->getPoints()->end(); ++it)
		{
			drawing.at<cv::Vec3b>(it->y, it->x)[0] = color[0];
			drawing.at<cv::Vec3b>(it->y, it->x)[1] = color[1];
			drawing.at<cv::Vec3b>(it->y, it->x)[2] = color[2]; 
		}

		rectangle(drawing, contours[i]->getBoundingBox().tl(), contours[i]->getBoundingBox().br(), color, 2, 8, 0);
		putText(drawing, std::to_string(((long long)i)), contours[i]->getBoundingBox().br(),
		cv::FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
	}

	saveImage(drawing, "contours.png");
}


float ReportWriter::reconPixelSize(int depth)
{
	return (float) depth / m_screen_to_source * m_pixel_size;
}




