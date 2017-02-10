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
///\file OfflineReader.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "OfflineReader.h"
#include <iostream>
#include <iostream>

bool OfflineReader::setSourceHologram(std::string folder, std::string filename, std::string background)
{
	m_background = background;
	m_folder = folder;
	m_filename = filename;
	return true;
}

void OfflineReader::getPhaseImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Phase_" + std::to_string(depth) + ".ext";

	std::cerr << "Load " << name << std::endl;
	FILE* file = fopen(name.c_str(), "rb");
	
	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);

	
	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}

void OfflineReader::getIntensityImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Intensity_" + std::to_string(depth) + ".ext";

	std::cerr << "Load " << name << std::endl;

	FILE* file = fopen(name.c_str(), "rb");

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);


	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}

void OfflineReader::getAmplitudeImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Amplitude_" + std::to_string(depth) + ".ext";
	
	std::cerr << "Load " << name << std::endl;

	FILE* file = fopen(name.c_str(), "rb");

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);


	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}
