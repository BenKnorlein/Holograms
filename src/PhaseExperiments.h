//file: PhaseExperiments.h

#ifndef PHASEEXPERIMENTS_H
#define PHASEEXPERIMENTS_H

#include <string>
#include "Contour.h"
#include "Settings.h"
#include "ImageCache.h"
#include <opencv2/core/core.hpp>

class PhaseExperiments {
	public:
		PhaseExperiments(Settings *settings, ImageCache *cache, std::string outdir);
		~PhaseExperiments();
		
		void randomContourPixels(Contour *c, int samples, int start_depth, int stop_depth, int step);

	private:
		int call_num;
		std::string m_outdir;
		Settings *m_settings;
		ImageCache *m_cache;
};
		
		
#endif //PHASEEXPERIMENTS_H
