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
///\file ThreadedImageSaver.cpp
///\author Benjamin Knorlein
///\date 12/15/2016

#include "ThreadedImageSaver.h"
#include "Settings.h"
#include <iostream>


HANDLE ghMutex;

static DWORD WINAPI StaticThreadStart(void* Param)
{
	ThreadedImageSaver* This = (ThreadedImageSaver*)Param;
	return This->run();
}

ThreadedImageSaver::ThreadedImageSaver(Settings* settings) : isQuitting(false)
{
	width = settings->getWidth();
	height = settings->getHeight();

	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
	}
	DWORD ThreadID;
	thread = CreateThread(NULL, 0, StaticThreadStart, (void*) this, 0, &ThreadID);
}

ThreadedImageSaver::~ThreadedImageSaver()
{
	DWORD dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval
	
	while ((name_vec.size() > 0 && data_vec.size() > 0))
	{
		ReleaseMutex(ghMutex);
		DWORD dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			INFINITE);  // no time-out interval
	}
	isQuitting = true;
	ReleaseMutex(ghMutex);

	WaitForSingleObject(thread, INFINITE);
}

DWORD ThreadedImageSaver::run()
{
	while (!isQuitting)
	{		
		while (save())
		{
			
		}
		Sleep(100);
	}
	return 0;
}

void ThreadedImageSaver::add(std::string name, float* data)
{
	DWORD dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	data_vec.push_back(data);
	name_vec.push_back(name);

	ReleaseMutex(ghMutex);
}

bool ThreadedImageSaver::save()
{
	std::string name = "";
	float * data = NULL;

	DWORD dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

		if (name_vec.size() > 0 && data_vec.size() > 0){
			name = name_vec[0];
			data = data_vec[0];

			data_vec.erase(data_vec.begin(), data_vec.begin() + 1);
			name_vec.erase(name_vec.begin(), name_vec.begin() + 1);
		}

	ReleaseMutex(ghMutex);

	if (data != NULL){
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);
		delete[] data;
		//Sleep(500);
		return true;
	}

	return false;
}