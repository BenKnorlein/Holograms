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