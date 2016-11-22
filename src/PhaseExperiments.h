//file: PhaseExperiments.h

#ifndef PHASEEXPERIMENTS_H
#define PHASEEXPERIMENTS_H

#include <string>
#include "Contour.h"
#include "Settings.h"
#include <opencv2/core/core.hpp>

class PhaseExperiments {
	public:
		PhaseExperiments(std::string outdir);
		~PhaseExperiments();
		
		void randomContourPixels(Contour *c, int samples, int start_depth, int stop_depth, int step);
	private:
		std::string m_outdir;
		
			
};
		
		
#endif //PHASEEXPERIMENTS_H
