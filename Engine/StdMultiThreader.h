#pragma once

typedef void(*ThreadFunctionType)(void*);

class ThreadInfo
{
public:
	int ThreadID;
	int NumberOfThreads;
	void* UserData = nullptr;
};

class StdMultiThreader
{
public:
	StdMultiThreader();
	~StdMultiThreader();

	void SingleMethodExecute();

	void SetSingleMethod(ThreadFunctionType method, void* data);

	void SetNumberOfThreads(unsigned int numThreads) { NumberOfThreads = numThreads; }
	unsigned int GetNumberOfThreads() { return NumberOfThreads; }

	int GetNumOfThreads() { return NumberOfThreads; }

protected:
	unsigned int NumberOfThreads = 1;
	ThreadInfo* threadInfo = nullptr;

	ThreadFunctionType method = nullptr;
	void* data = nullptr;
};