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
///\file ImageSource.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <string>

class ImageSource {
	public:
		virtual bool setSourceHologram(std::string folder, std::string filename, std::string background = "") = 0;
		virtual void getPhaseImage(int depth, float* data)  = 0;
		virtual void getIntensityImage(int depth, float* data) = 0;
		virtual void getAmplitudeImage(int depth, float* data) = 0;

		std::string getFolder(){ return m_folder; }
		std::string getFilename(){ return m_filename; }

	protected:
		std::string m_folder;
		std::string m_filename;
		std::string m_background;
};

#endif //IMAGESOURCE_H