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