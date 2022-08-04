#include <iostream>
#include <windows.h>
#include <string>
#include <ranges>

using namespace std;
namespace view = ranges::views;

struct Marker
{
	unsigned int* arr;
	unsigned int size;
	unsigned int num;
	HANDLE hStartEvent;
	HANDLE hCantWorkEvent;
	HANDLE hContinueWorkEvent;
	HANDLE hEndWorkEvent;
	CRITICAL_SECTION* cs;
	Marker(unsigned int* a, unsigned int size, unsigned int num, HANDLE startEvent, HANDLE cantWorkEvent,
		HANDLE continueWorkEvent, HANDLE endWorkEvent, CRITICAL_SECTION* section)
	{
		arr = a;
		this->size = size;
		this->num = num;
		hStartEvent = startEvent;
		hCantWorkEvent = cantWorkEvent;
		hContinueWorkEvent = continueWorkEvent;
		hEndWorkEvent = endWorkEvent;
		cs = section;
	}
};

void resetMarkedElements(unsigned int* arr, unsigned int size, unsigned int value)
{
	for (auto i : view::iota(0U, size))
	{
		if (arr[i] == value)
		{
			arr[i] = 0;
		}
	}
}

DWORD WINAPI marker(LPVOID param1)
{
	Marker* param = (Marker*)param1;
	WaitForSingleObject(param->hStartEvent, INFINITE);

	unsigned int markedCount = 0;
	unsigned int randomNumber = 0;
	EnterCriticalSection(param->cs);
	cout << "Marker thread " << param->num << " started" << endl << endl;
	LeaveCriticalSection(param->cs);
	srand(param->num);

	while (true)
	{
		randomNumber = rand() % param->size;

		EnterCriticalSection(param->cs);
		if (param->arr[randomNumber] == 0)
		{
			Sleep(5);
			param->arr[randomNumber] = param->num;
			LeaveCriticalSection(param->cs);

			markedCount++;
			Sleep(5);
		}
		else
		{
			cout << "Unable to continue!" << endl;
			cout << "Thread number: " << param->num << endl;
			cout << "Elements marked: " << markedCount << endl;
			cout << "Impossible to mark an element with index: " << randomNumber << endl << endl;
			LeaveCriticalSection(param->cs);

			SetEvent(param->hCantWorkEvent);
			HANDLE events[2] = { param->hContinueWorkEvent, param->hEndWorkEvent };
			DWORD eventNum = WaitForMultipleObjects(2, events, false, INFINITE);

			if (eventNum == 1)
			{
				resetMarkedElements(param->arr, param->size, param->num);
				break;
			}
		}
	}

	cout << "Marker thread " << param->num << " ended" << endl;

	return 0;
}

void printArray(unsigned int* arr, unsigned int size)
{
	for (auto i : view::iota(0U, size))
	{
		cout << arr[i] << " ";
	}
	cout << endl;
}

void continueNotEndedThreads(HANDLE* hCantWorkEvent, bool* isThreadEnded, unsigned int threadCount)
{
	for (auto j : view::iota(0U, threadCount))
	{
		if (!isThreadEnded[j])
		{
			ResetEvent(hCantWorkEvent[j]);
		}
	}
}

unsigned int getUInt(string message)
{
	unsigned int n;
	cout << message << endl;
	cin >> n;
	return n;
}

unsigned int getThreadNum(bool* isThreadEnded, unsigned int threadCount)
{
	unsigned int threadNum = getUInt("Enter number of a thread you want to end:");
	while (threadNum < 1 || threadNum > threadCount || isThreadEnded[threadNum - 1])
	{
		cout << "Error!" << endl;
		threadNum = getUInt("Enter number of a thread you want to end:");
	}
	return threadNum;
}

template <class T>
void initializeArrayWithValue(T* arr, unsigned int size, T value)
{
	for (auto i : view::iota(0U, size))
	{
		arr[i] = value;
	}
}

int main()
{
	auto size = getUInt("Enter array size:");
	unsigned int* arr = new unsigned int[size];

	initializeArrayWithValue(arr, size, 0U);

	auto threadCount = getUInt("Enter Marker thread count:");

	CRITICAL_SECTION* cs = new CRITICAL_SECTION();
	InitializeCriticalSection(cs);

	HANDLE hStartEvent = CreateEventA(NULL, true, false, NULL);
	HANDLE hContinueWorkEvent = CreateEventA(NULL, true, false, NULL);
	HANDLE* hCantWorkEvent = new HANDLE[threadCount];
	HANDLE* hEndWorkEvent = new HANDLE[threadCount];
	for (auto i : view::iota(0U, threadCount))
	{
		hCantWorkEvent[i] = CreateEventA(NULL, true, false, NULL);
		hEndWorkEvent[i] = CreateEventA(NULL, true, false, NULL);
	}

	HANDLE* hMarkerThread = new HANDLE[threadCount];
	Marker** param1 = new Marker * [threadCount];
	for (auto i : view::iota(0U, threadCount))
	{
		param1[i] = new Marker(arr, size, i + 1, hStartEvent, hCantWorkEvent[i], hContinueWorkEvent, hEndWorkEvent[i], cs);
		hMarkerThread[i] = CreateThread(NULL, 0, marker, (void*)param1[i], NULL, NULL);
		if (hMarkerThread[i] == NULL)
		{
			for (auto j : view::iota(0U, i))
			{
				delete param1[j];
				CloseHandle(hMarkerThread[i]);
			}
			delete param1[i];
			for (auto j : view::iota(0U, threadCount))
			{
				CloseHandle(hCantWorkEvent[j]);
				CloseHandle(hEndWorkEvent[j]);
			}
			CloseHandle(hStartEvent);
			CloseHandle(hContinueWorkEvent);
			delete[] arr;
			delete[] hCantWorkEvent;
			delete[] hEndWorkEvent;
			delete[] hMarkerThread;
			delete[] param1;
			return GetLastError();
		}
	}

	SetEvent(hStartEvent);

	bool* isThreadEnded = new bool[threadCount];
	initializeArrayWithValue(isThreadEnded, threadCount, false);
	for (auto i : view::iota(0U, threadCount))
	{
		WaitForMultipleObjects(threadCount, hCantWorkEvent, true, INFINITE);
		printArray(arr, size);
		auto threadNum = getThreadNum(isThreadEnded, threadCount);
		isThreadEnded[threadNum - 1] = true;
		SetEvent(hEndWorkEvent[threadNum - 1]);
		WaitForSingleObject(hMarkerThread[threadNum - 1], INFINITE);
		printArray(arr, size);
		continueNotEndedThreads(hCantWorkEvent, isThreadEnded, threadCount);
		SetEvent(hContinueWorkEvent);
		ResetEvent(hContinueWorkEvent);
	}
	WaitForMultipleObjects(threadCount, hMarkerThread, true, INFINITE);

	DeleteCriticalSection(cs);
	delete cs;
	for (auto i : view::iota(0U, threadCount))
	{
		delete param1[i];
		CloseHandle(hCantWorkEvent[i]);
		CloseHandle(hEndWorkEvent[i]);
		CloseHandle(hMarkerThread[i]);
	}
	CloseHandle(hStartEvent);
	CloseHandle(hContinueWorkEvent);
	delete[] arr;
	delete[] isThreadEnded;
	delete[] hCantWorkEvent;
	delete[] hEndWorkEvent;
	delete[] hMarkerThread;
	delete[] param1;

	return 0;
}