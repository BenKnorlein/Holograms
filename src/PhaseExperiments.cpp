#include "PhaseExperiments.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>

// Constructor
PhaseExperiments::PhaseExperiments(Settings *settings, ImageCache *cache, std::string outdir) 
{
	m_settings = settings;
	m_cache = cache;
	m_outdir = outdir;
	call_num = 0;
}

// Deconstructor
PhaseExperiments::~PhaseExperiments() {}

// Methods

/**
 * Description
 *   This function saves the pixel values of a selection of random points
 *   in a contour over all the depths.  Right now the 'random' points are
 *   not seeded so it will return the same random points every time the 
 *   program is run. Now this may actually be nice to have to be able
 *   to compare the same set of points but if we decide to change it, the
 *   static function randu can be seeded by:
 *     
 *       cv::theRNG().state = seed;
 *
 *   A seed of the current time will make it produce new results with each
 *   run.
 *
 *   The format of the .csv is as follows:
 *   	
 *   	Point,x_coordinate of point1,y_coordinate of point1
 *   	depth1.1,value1.1
 *   	depth1.2,value1.2
 *   	  .
 *   	  .
 *   	  .
 *   	Point,x_coordinate of point2,y_cordinate of point2
 *   	depth2.1,value2.1
 *   	depth2.2,value2.2
 *   	  .
 *   	  .
 *   	  .
 *
 *
 * Parameters
 *   [Contour *] c: the contour to inspect points from
 *   [int] samples: the number of points to inspect
 *   [int] start_depth: the depth to start at (inclusive)
 *   [int] stop_depth: the depth to stop at (inclusive)
 *   [int] step: the size of a jump to make in between depths
 *
 * Returns
 *   void
 *
 * TODO
 *   [+] check performance online with smaller steps
 */
void PhaseExperiments::randomContourPixels(Contour *c, int samples, int start_depth, int stop_depth, int step) {
	int n = c->getArea();
	call_num++;
	
	/**
         * only inspect at most the number of pixels
         * that are in the contour
         */
	if (samples > n) {
		samples = n;
	}
	
	std::vector<int> indexes(samples);
	cv::randu(indexes, 0, n);	
	
	std::ofstream txt_out, csv_out;
	txt_out.open(m_outdir + "/" + "phaseTesting_call" + std::to_string(call_num) + ".txt");
	csv_out.open(m_outdir + "/" + "phaseTesting_call" + std::to_string(call_num) + ".csv");	

	std::vector<cv::Point> & pointsRef = *c->getPoints();	
	
	for (int i : indexes) {
		cv::Point p = pointsRef[i];
		txt_out << "(" << p.x << ", " << p.y << ")\n";
		csv_out << "Point," << p.x << "," << p.y << std::endl;
	
		for (int d = start_depth; d <= stop_depth; d += step)
		{
			// check if this is right
			float value = m_cache->getPhaseImage(d)->at<float>(p.x, p.y);

			txt_out << "  Depth: " << d << "  Value: " << value << std::endl; 
			csv_out << d << "," << value << std::endl;
		}
	}
	
	txt_out.close();
	csv_out.close();
	
	if (!txt_out) {
		std::cerr << "Error occured writing phase testing .txt file\n";
	}
	
	if (!csv_out) {
		std::cerr << "Error occured writing phase testing .cvs file\n";
	}

}



