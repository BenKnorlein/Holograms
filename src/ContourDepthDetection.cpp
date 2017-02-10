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
///\file ContourDepthDetection.cpp
///\author Benjamin Knorlein
///\date 11/24/2016

#include "ContourDepthDetection.h"
#include <iostream>

ContourDepthDetection::ContourDepthDetection(ImageCache * cache, Settings * settings) : m_cache(cache), m_settings(settings)
{
	
}

ContourDepthDetection::~ContourDepthDetection()
{


}

void ContourDepthDetection::findBestDepth(Contour* contour, int start, int stop, int step_width)
{
	int best_depth = 0;
	float maxVal = -100000;
	contour->getValues()->clear();
	for (int d = start; d <= stop; d += step_width){
		cv::Mat image;
		if (!m_settings->getUseSharpness()){
			image = *m_cache->getPhaseImage(d);
		} 
		else
		{
			image = *m_cache->getGradientImage(d);
		}
		cv::Mat tmp;

		cv::Mat roi;
		((image)(contour->getBoundingBox())).copyTo(roi);

		if (m_settings->getUseAbs()) roi = cv::abs(roi);

		float val = cv::mean(roi, *contour->getMask())[0];
		contour->getValues()->push_back(val);
		if (val > maxVal)
		{
				maxVal = val;
				best_depth = d;
		}
	}
	contour->setDepth(best_depth);
	contour->setMaxValue(maxVal);
	std::cout << "Best Depth " << best_depth << std::endl;
}
