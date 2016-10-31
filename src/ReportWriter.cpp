///\file ReportWriter.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "ReportWriter.h"
#include "Settings.h"
#include <iostream>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo.hpp>

ReportWriter::ReportWriter(std::string outdir, std::string filename, std::string templateFolder) : m_outdir(outdir + "/" + filename), m_filename(filename), m_templateFolder(templateFolder)
{
	

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

	outfile << "<DATA>" << std::endl;
	outfile << "<FILENAME>" << m_filename << "</FILENAME>" << std::endl;
	outfile << "<CONTOURIMAGE>contours.png</CONTOURIMAGE>" << std::endl;
	outfile << "<MAXIMAGE>maximum.png</MAXIMAGE>" << std::endl;
	outfile << "<TIME>" << time << "</TIME>" << std::endl;

	for (size_t c = 0; c < contours.size(); c++)
	{
		outfile << "<ROI>" << std::endl;
		outfile << "<CONTOUR>" << c << "</CONTOUR>" << std::endl;
		
		// TODO convert pixels to real units
		outfile << "<X>" << (contours[c]->getBoundingBox().tl() + contours[c]->getBoundingBox().br()).x * 0.5 << "</X>" << std::endl;
		outfile << "<Y>" << (contours[c]->getBoundingBox().tl() + contours[c]->getBoundingBox().br()).y * 0.5 << "</Y>" << std::endl;
		outfile << "<WIDTH>" << contours[c]->getBoundingBox().width << "</WIDTH>" << std::endl;
		outfile << "<HEIGHT>" << contours[c]->getBoundingBox().height << "</HEIGHT>" << std::endl;
		outfile << "<DEPTH>" << contours[c]->getDepth() << "</DEPTH>" << std::endl;
		outfile << "<VAL>" << contours[c]->getValue() << "</VAL>" << std::endl;
		outfile << "<IMAGE>" << "contours_" + std::to_string(((long long)c)) + ".png" << "</IMAGE>" << std::endl;
		outfile << "<IMAGEPHASE>" << "contoursPhase_" + std::to_string(((long long)c)) + ".png" << "</IMAGEPHASE>" << std::endl;
		outfile << "</ROI>" << std::endl;
	}
	outfile << "</DATA>" << std::endl;
	outfile << "</doc>" << std::endl;

	outfile.close();
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

		//if (1)
		//{
		//	cv::Mat rgb_mat;
		//	image_display.convertTo(rgb_mat, CV_GRAY2RGB);
		//	cv::Point2d center = contours[c]->getPCACenter() - cv::Point2d(bound_cont.x, bound_cont.y);
		//	cv::Point2d p1a = center + contours[c]->getPCAAxis()[0];
		//	cv::Point2d p1b = center - contours[c]->getPCAAxis()[0];
		//	cv::Point2d p2a = center + contours[c]->getPCAAxis()[1];
		//	cv::Point2d p2b = center - contours[c]->getPCAAxis()[1];
		//	cv::line(rgb_mat, p1a, p1b, cv::Scalar(0, 255, 0), 1, CV_AA);
		//	cv::line(rgb_mat, p2a, p2b, cv::Scalar(255, 0,255), 1, CV_AA);
		//	cv::imwrite(m_outdir + "/" + "contoursAxis_" + std::to_string(((long long)c)) + ".png", rgb_mat);

		//	cv::Mat rgb_mask;
		//	image_display.convertTo(rgb_mat, CV_GRAY2RGB);
		//}


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
