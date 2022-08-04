#include <windows.h>
#include <iostream>
#include <string>
#include <ranges>

using namespace std;
namespace view = ranges::views;

struct MinMax
{
	int* arr;
	unsigned int n;
	int min;
	int max;

	MinMax(int* a, int count)
	{
		n = count;
		arr = new int[n];
		for (auto i : view::iota(0U, n))
		{
			arr[i] = a[i];
		}
		min = max = arr[0];
	}

	~MinMax()
	{
		delete[] arr;
	}
};

struct AverageValue {
	unsigned int n;
	int* arr;
	double avg;

	AverageValue(int* a, int count)
	{
		n = count;
		arr = new int[n];
		for (auto i : view::iota(0U, n))
		{
			arr[i] = a[i];
		}
		avg = arr[0];
	}

	~AverageValue()
	{
		delete[] arr;
	}
};

DWORD WINAPI minMax(LPVOID param1)
{
	cout << "MinMax thread started" << endl;
	MinMax* param = (MinMax*)param1;
	for (auto i : view::iota(1U, param->n))
	{
		if (param->arr[i] > param->max)
		{
			param->max = param->arr[i];
		}
		Sleep(7);
		if (param->arr[i] < param->min)
		{
			param->min = param->arr[i];
		}
		Sleep(7);
	}
	cout << "Min is: " << param->min << endl;
	cout << "Max is: " << param->max << endl;
	cout << "MinMax thread ended" << endl;
	return 0;
}

DWORD WINAPI average(LPVOID param1)
{
	cout << "Average thread started" << endl;
	AverageValue* param = (AverageValue*)param1;
	for (auto i : view::iota(1U, param->n))
	{
		param->avg += param->arr[i];
		Sleep(12);
	}
	param->avg /= param->n;
	cout << "Average is: " << param->avg << endl;
	cout << "Average thread ended" << endl;
	return 0;
}

void printArray(int* arr, unsigned int n, string message)
{
	cout << message << endl;
	for (auto i : view::iota(0U, n))
	{
		cout << arr[i] << " ";
	}
	cout << endl;
}

unsigned int getArraySize()
{
	unsigned int n;
	cout << "Enter array size:" << endl;
	cin >> n;
	return n;
}

int* getArray(unsigned int size)
{
	int* arr = new int[size];
	cout << "Enter " << size << " elements" << endl;
	for (auto i : view::iota(0U, size))
	{
		cin >> arr[i];
	}
	return arr;
}

void replaceMinMaxWithAverage(int* arr, unsigned int size, int min, int max, double avg)
{
	for (auto i : view::iota(0U, size))
	{
		if (arr[i] == min || arr[i] == max)
		{
			arr[i] = avg;
		}
	}
}

void clearData(int* arr, MinMax* minMaxParam, AverageValue* averageParam)
{
	delete[] arr;
	delete minMaxParam;
	delete averageParam;
}

int main()
{
	auto n = getArraySize();
	int* arr = getArray(n);
	MinMax* minMaxParam = new MinMax(arr, n);
	AverageValue* averageParam = new AverageValue(arr, n);

	printArray(arr, n, "Your array:");

	HANDLE hMinMaxThread = CreateThread(NULL, 0, minMax, (void*)minMaxParam, 0, 0);
	if (hMinMaxThread == NULL)
	{
		cout << "Error while creating MinMax thread" << endl;
		clearData(arr, minMaxParam, averageParam);
		return GetLastError();
	}

	HANDLE hAverageThread = CreateThread(NULL, 0, average, (void*)averageParam, 0, 0);
	if (hAverageThread == NULL)
	{
		cout << "Error while creating Average thread" << endl;
		clearData(arr, minMaxParam, averageParam);
		return GetLastError();
	}

	WaitForSingleObject(hMinMaxThread, INFINITE);
	WaitForSingleObject(hAverageThread, INFINITE);
	CloseHandle(hMinMaxThread);
	CloseHandle(hAverageThread);

	replaceMinMaxWithAverage(arr, n, minMaxParam->min, minMaxParam->max, averageParam->avg);

	printArray(arr, n, "Updated array:");

	clearData(arr, minMaxParam, averageParam);

	return 0;
}