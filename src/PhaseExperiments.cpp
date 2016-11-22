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
 *   [+] add absolute value based off settings
 *   [+] maybe normalize floating point values
 *   [+] change from .txt -> .csv format so it can be graphed
 *   [+] check performance online with smaller steps
 */
void PhaseExperiments::randomContourPixels(Contour *c, int samples, int start_depth, int stop_depth, int step) {
	int n = c->getArea();
	
	/**
         * only inspect at most the number of pixels
         * that are in the contour
         */
	if (samples > n) {
		samples = n;
	}
	
	std::vector<int> indexes(samples);
	cv::randu(indexes, 0, n);	
	
	std::ofstream out;
	out.open(m_outdir + "/" + "phaseTesting_" + std::to_string(((long long)c))  + ".txt");
	
	std::vector<cv::Point> & pointsRef = *c->getPoints();	
	
	for (int i : indexes) {
		cv::Point p = pointsRef[i];
		out << "(" << p.x << ", " << p.y << ")\n";
	
		for (int d = start_depth; d <= stop_depth; d += step)
		{
			// check if this is right
			float value = m_cache->getPhaseImage(d)->at<float>(p.x, p.y);

			out << "  Depth: " << d << "  Value: " << value << std::endl; 
		}
	}
	
	out.close();
	
	if (!out) {
		std::cerr << "Error occured writing phase testing file\n";
	}
}



