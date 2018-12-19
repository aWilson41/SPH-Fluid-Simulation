#include "StdMultiThreader.h"
#include <thread>
#include <vector>

StdMultiThreader::StdMultiThreader()
{
#ifdef __APPLE__
	NumberOfThreads = 4;
#else
	NumberOfThreads = std::thread::hardware_concurrency();
#endif

	threadInfo = new ThreadInfo[NumberOfThreads];
}

void StdMultiThreader::SetSingleMethod(ThreadFunctionType method, void* data)
{
	StdMultiThreader::method = method;
	StdMultiThreader::data = data;
}

void StdMultiThreader::SingleMethodExecute()
{
	// Spawn all the threads
	std::vector<std::thread> threads = std::vector<std::thread>(NumberOfThreads);
	for (unsigned int i = 0; i < NumberOfThreads; i++)
	{
		threadInfo[i].NumberOfThreads = NumberOfThreads;
		threadInfo[i].ThreadID = i;
		threadInfo[i].UserData = data;
		threads[i] = std::thread(method, &threadInfo[i]);
	}
	// Wait for all the threads to finish
	for (unsigned int i = 0; i < NumberOfThreads; i++)
	{
		threads[i].join();
	}
}

StdMultiThreader::~StdMultiThreader()
{
	if (threadInfo != nullptr)
		delete[] threadInfo;
}