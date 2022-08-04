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

void printMSG(char* msg)
{
	for (auto i : view::iota(0U, MESSAGE_SIZE))
	{
		cout << msg[i];
		if (msg[i] == '\0')
		{
			break;
		}
	}
	cout << endl;
}

string getCWD(char* argv[])
{
	string cwd(argv[0]);
	for (auto i : view::iota(0U, 3U))
	{
		cwd = cwd.substr(0, cwd.find_last_of("\\"));
	}
	return cwd;
}

string getFileName(string requestMessage)
{
	string fileName;
	cout << requestMessage << endl;
	getline(cin, fileName);
	return fileName;
}

unsigned int getUInt(string message)
{
	unsigned int recordsCount;
	cout << message << endl;
	cin >> recordsCount;
	cin.ignore();
	return recordsCount;
}

bool createProcess(string name, string commandLine, string workingDirectory, LPSTARTUPINFOA si, LPPROCESS_INFORMATION piApp)
{
	if (!CreateProcessA(NULL, (LPSTR)commandLine.c_str(), NULL, NULL, false,
		CREATE_NEW_CONSOLE, NULL, workingDirectory.c_str(), si, piApp))
	{
		cout << "Error while creating " << name << " process" << endl;
		return false;
	}
	return true;
}

int main(int argc, char* argv[])
{
	string workingDirectory = getCWD(argv);
	cout << "Reciever process started" << endl;
	string fileName = getFileName("Enter binary file name:");
	auto recordsCount = getUInt("Enter records count:");
	auto currentPos = 0U;
	ofstream fout(workingDirectory + "\\" + fileName, ios::out | ios::binary);
	if (!fout.is_open())
	{
		cout << "Error while opening \"" << fileName << "\" file" << endl;
		return 0;
	}
	fout.write((char*)&currentPos, sizeof(unsigned int));
	fout.close();
	auto senderCount = getUInt("Enter Sender process count");

	HANDLE hMutex = CreateMutexA(NULL, false, "mutex");
	HANDLE hSemaphoreRecordsCount = CreateSemaphoreA(NULL, 0, recordsCount, "recordsCount");
	HANDLE hSemaphoreSpacesCount = CreateSemaphoreA(NULL, recordsCount, recordsCount, "spacesCount");
	HANDLE* hReadyEvent = new HANDLE[senderCount];
	STARTUPINFOA* si = new STARTUPINFOA[senderCount];
	PROCESS_INFORMATION* piApp = new PROCESS_INFORMATION[senderCount];
	string senderCommandLine = "Sender.exe " + fileName + " " + to_string(recordsCount);
	for (auto i : view::iota(0U, senderCount))
	{
		ZeroMemory(&si[i], sizeof(STARTUPINFOA));
		si[i].cb = sizeof(STARTUPINFOA);
		string eventName = "Sender" + to_string(i);
		string newSenderCommandLine = senderCommandLine + " " + eventName;
		hReadyEvent[i] = CreateEventA(NULL, true, false, eventName.c_str());
		if (!createProcess(eventName, newSenderCommandLine, workingDirectory, &si[i], &piApp[i]))
		{
			for (auto j : view::iota(0U, i))
			{
				CloseHandle(piApp[j].hThread);
				CloseHandle(piApp[j].hProcess);
				CloseHandle(hReadyEvent[j]);
			}
			CloseHandle(hReadyEvent[i]);
			CloseHandle(hSemaphoreRecordsCount);
			CloseHandle(hSemaphoreSpacesCount);
			CloseHandle(hMutex);
			delete[] hReadyEvent;
			delete[] piApp;
			delete[] si;
			return GetLastError();
		}
	}
	WaitForMultipleObjects(senderCount, hReadyEvent, true, INFINITE);

	message msg;
	unsigned int choice;
	bool working = true;
	fstream fin;
	while (working)
	{
		choice = getUInt("Choose:\n1. Read new message\n2. Quit");
		switch (choice)
		{
		case 1:
		{
			WaitForSingleObject(hSemaphoreRecordsCount, INFINITE);
			fin.open((workingDirectory + "\\" + fileName).c_str(), ios::in | ios::binary);
			if (!fin.is_open())
			{
				cout << "Error while opening \"" << fileName << "\" file" << endl;
				for (auto j : view::iota(0U, senderCount))
				{
					CloseHandle(piApp[j].hThread);
					CloseHandle(piApp[j].hProcess);
					CloseHandle(hReadyEvent[j]);
				}
				CloseHandle(hSemaphoreRecordsCount);
				CloseHandle(hSemaphoreSpacesCount);
				CloseHandle(hMutex);
				delete[] hReadyEvent;
				delete[] piApp;
				delete[] si;
				return 0;
			}
			fin.seekp(sizeof(unsigned int) + sizeof(message) * currentPos, ios::beg);
			fin.read((char*)&msg, sizeof(message));
			fin.close();
			cout << "Recieved message: ";
			printMSG(msg.msg);
			currentPos++;
			if (currentPos == recordsCount)
			{
				currentPos = 0;
			}
			ReleaseSemaphore(hSemaphoreSpacesCount, 1, NULL);
			break;
		}
		default:
		{
			working = false;
		}
		}
	}

	for (auto i : view::iota(0U, senderCount))
	{
		CloseHandle(piApp[i].hThread);
		CloseHandle(piApp[i].hProcess);
		CloseHandle(hReadyEvent[i]);
	}
	CloseHandle(hSemaphoreRecordsCount);
	CloseHandle(hSemaphoreSpacesCount);
	CloseHandle(hMutex);
	delete[] hReadyEvent;
	delete[] piApp;
	delete[] si;

	cout << "Reciever process ended" << endl;
	return 0;
}