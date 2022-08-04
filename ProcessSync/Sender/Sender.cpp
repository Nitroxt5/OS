#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <ranges>

using namespace std;
namespace view = ranges::views;

const auto MESSAGE_SIZE = 20U;

struct message
{
	char msg[MESSAGE_SIZE + 1];
};

unsigned int getUInt(string message)
{
	unsigned int recordsCount;
	cout << message << endl;
	cin >> recordsCount;
	cin.ignore();
	return recordsCount;
}

int main(int argc, char* argv[])
{
	string readyEventName(argv[3]);
	cout << readyEventName << " process started" << endl;
	string fileName(argv[1]);
	unsigned int recordsCount = atoi(argv[2]);
	HANDLE hReadyEvent = OpenEventA(EVENT_ALL_ACCESS, false, readyEventName.c_str());
	HANDLE hSemaphoreRecordsCount = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, "recordsCount");
	HANDLE hSemaphoreSpacesCount = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, "spacesCount");
	HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, false, "mutex");

	fstream file;
	bool working = true;
	unsigned int currentPos;
	unsigned int choice;
	message msg;
	SetEvent(hReadyEvent);
	while (working)
	{
		choice = getUInt("Choose:\n1. Send new message\n2. Quit");
		switch (choice)
		{
		case 1:
		{
			WaitForSingleObject(hSemaphoreSpacesCount, INFINITE);
			cout << "Enter new message" << endl;
			cin.getline(msg.msg, MESSAGE_SIZE);
			WaitForSingleObject(hMutex, INFINITE);
			file.open(fileName, ios::in | ios::out | ios::binary);
			if (!file.is_open())
			{
				cout << "Error while opening \"" << fileName << "\" file" << endl;
				CloseHandle(hReadyEvent);
				CloseHandle(hSemaphoreRecordsCount);
				CloseHandle(hSemaphoreSpacesCount);
				CloseHandle(hMutex);
				system("pause");
				return 0;
			}
			file.seekp(0, ios::beg);
			file.read((char*)&currentPos, sizeof(unsigned int));
			file.seekp(sizeof(unsigned int) + sizeof(message) * currentPos, ios::beg);
			file.write((char*)&msg, sizeof(message));
			currentPos++;
			if (currentPos == recordsCount)
			{
				currentPos = 0;
			}
			file.seekp(0, ios::beg);
			file.write((char*)&currentPos, sizeof(unsigned int));
			ReleaseMutex(hMutex);
			file.close();
			ReleaseSemaphore(hSemaphoreRecordsCount, 1, NULL);
			break;
		}
		default:
		{
			working = false;
		}
		}
	}

	CloseHandle(hReadyEvent);
	CloseHandle(hSemaphoreRecordsCount);
	CloseHandle(hSemaphoreSpacesCount);
	CloseHandle(hMutex);

	cout << readyEventName << " process ended" << endl;
	system("pause");
	return 0;
}