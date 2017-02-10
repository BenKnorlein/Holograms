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
///\file ContourDepthDetection.h
///\author Benjamin Knorlein
///\date 11/24/2016

#ifndef CONTOURDEPTHDETECTION_H
#define CONTOURDEPTHDETECTION_H

#include <string>
#include "ImageCache.h"
#include "Settings.h"
#include "Contour.h"

#include <opencv2/core/core.hpp>

class ContourDepthDetection {
public:
	ContourDepthDetection(ImageCache * cache, Settings * settings);
	~ContourDepthDetection();

	void findBestDepth(Contour * contour, int start, int stop, int step_width);

private:
	ImageCache * m_cache;
	Settings * m_settings;
};

#endif //CONTOURDEPTHDETECTION_H