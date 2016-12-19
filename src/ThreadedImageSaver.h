///\file ThreadedImageSaver.h
///\author Benjamin Knorlein
///\date 12/15/2016

#ifndef THREADEDIMAGESAVER_H
#define THREADEDIMAGESAVER_H

#include <string>
#include <vector>
#include <windows.h>

class Settings;

class ThreadedImageSaver {
public:

	ThreadedImageSaver(Settings* settings);
	~ThreadedImageSaver();

	void add(std::string name, float* data);
	bool save();
	DWORD ThreadedImageSaver::run();

private:
	std::vector<float*> data_vec;
	std::vector<std::string> name_vec;

	int width, height;
	bool isQuitting;
	HANDLE thread;
};

#endif //THREADEDIMAGESAVER_H
