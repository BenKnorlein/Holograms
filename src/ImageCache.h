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
///\file ImageCache.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include "ImageSource.h"
#include <opencv2/core/core.hpp>

class Settings;
class ImageCache {
public:
	ImageCache(ImageSource * source, Settings * settings, int maxImages, int width = 2048, int height = 2048);
	~ImageCache();

	cv::Mat* getPhaseImage(int depth);
	cv::Mat* getIntensityImage(int depth);
	cv::Mat* getAmplitudeImage(int depth);
	cv::Mat* getGradientImage(int depth);

	ImageSource *getImageSource(){ return m_source; }

private:
	int findInCache(int mode, int depth);
	void removeOldestImage();
	void removeBorders(cv::Mat* image, int depth);

	int m_maxImages;
	std::vector<cv::Mat> m_images;
	std::vector<float *>m_images_data;
	std::vector<int> m_depths;
	std::vector<int> m_mode;
	int m_width;
	int m_height;
	double m_conversionToWorld;
	ImageSource *m_source;
	Settings* m_settings;
};

#endif //IMAGECACHE_H