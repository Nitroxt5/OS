#include "Server.h"


DWORD WINAPI Server(LPVOID param1)
{
	serverParams* param = (serverParams*)param1;
	HANDLE hNamedPipe = CreateNamedPipeA("\\\\.\\pipe\\filepipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, param->clientCount, 0, 0, INFINITE, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hNamedPipe);
		return GetLastError();
	}
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	PROCESS_INFORMATION piApp;
	string procName = "Client" + to_string(param->threadNum);
	string clientCommandLine = "Client.exe " + procName;
	if (!CreateProcessA(NULL, (LPSTR)clientCommandLine.c_str(), NULL, NULL, false,
		CREATE_NEW_CONSOLE, NULL, NULL, &si, &piApp))
	{
		cout << "Error while creating " << procName << " process" << endl;
		SetEvent(param->hEndEvent);
		return GetLastError();
	}
	unsigned int choice = 0;
	int row = 0;
	bool working = true;
	bool recordExist;
	char end = ' ';
	employee emp;
	fstream fin;
	if (ConnectNamedPipe(hNamedPipe, NULL))
	{
		while (working)
		{
			ReadFile(hNamedPipe, &choice, sizeof(unsigned int), NULL, NULL);
			int num = 0;
			switch (choice)
			{
			case 1:
			{
				ReadFile(hNamedPipe, &num, sizeof(unsigned int), NULL, NULL);
				row = findRecord(param->fileName, num, param->recordCount, emp);
				recordExist = emp.num == -1 ? false : true;
				if (row == -1)
				{
					SetEvent(param->hEndEvent);
					return 0;
				}
				WaitForSingleObject(param->hModifySemaphores[row], INFINITE);
				if (recordExist)
				{
					fin.open(param->fileName, ios::in | ios::out | ios::binary);
					if (!fin.is_open())
					{
						cout << "Error while opening \"" << param->fileName << "\" file" << endl;
						SetEvent(param->hEndEvent);
						return 0;
					}
					fin.seekp(sizeof(employee) * row, ios::beg);
					fin.read((char*)&emp, sizeof(employee));
				}
				WriteFile(hNamedPipe, &emp, sizeof(employee), NULL, NULL);
				ReadFile(hNamedPipe, &emp, sizeof(employee), NULL, NULL);
				if (recordExist)
				{
					fin.seekp(sizeof(employee) * row, ios::beg);
					fin.write((char*)&emp, sizeof(employee));
					fin.close();
				}
				ReadFile(hNamedPipe, &end, sizeof(char), NULL, NULL);
				ReleaseSemaphore(param->hModifySemaphores[row], 1, NULL);
				break;
			}
			case 2:
			{
				ReadFile(hNamedPipe, &num, sizeof(unsigned int), NULL, NULL);
				row = findRecord(param->fileName, num, param->recordCount, emp);
				recordExist = emp.num == -1 ? false : true;
				if (row == -1)
				{
					SetEvent(param->hEndEvent);
					return 0;
				}
				EnterCriticalSection(&param->cs[row]);
				if (param->readerCount[row] == 0)
				{
					WaitForSingleObject(param->hModifySemaphores[row], INFINITE);
				}
				++param->readerCount[row];
				LeaveCriticalSection(&param->cs[row]);
				if (recordExist)
				{
					fin.open(param->fileName, ios::in | ios::out | ios::binary);
					if (!fin.is_open())
					{
						cout << "Error while opening \"" << param->fileName << "\" file" << endl;
						SetEvent(param->hEndEvent);
						return 0;
					}
					fin.seekp(sizeof(employee) * row, ios::beg);
					fin.read((char*)&emp, sizeof(employee));
					fin.close();
				}
				WriteFile(hNamedPipe, &emp, sizeof(employee), NULL, NULL);
				ReadFile(hNamedPipe, &end, sizeof(char), NULL, NULL);
				EnterCriticalSection(&param->cs[row]);
				--param->readerCount[row];
				if (param->readerCount[row] == 0)
				{
					ReleaseSemaphore(param->hModifySemaphores[row], 1, NULL);
				}
				LeaveCriticalSection(&param->cs[row]);
				break;
			}
			default:
			{
				working = false;
			}
			}
		}
	}
	else
	{
		SetEvent(param->hEndEvent);
		return GetLastError();
	}

	DisconnectNamedPipe(hNamedPipe);
	CloseHandle(hNamedPipe);
	CloseHandle(piApp.hThread);
	CloseHandle(piApp.hProcess);
	SetEvent(param->hEndEvent);

	return 0;
}

int main()
{
	cout << "Server process started" << endl;

	string fileName = getFileName("Enter binary file name:");
	auto recordCount = getUInt("Enter records count");
	if (!fillFileWithRecords(fileName, recordCount))
	{
		return 0;
	}
	cout << "File content:" << endl;
	if (!retransmitFile(fileName, recordCount))
	{
		return 0;
	}
	auto clientCount = getUInt("Enter Client process count:");

	HANDLE* hModifySemaphores = new HANDLE[recordCount];
	auto readerCount = new unsigned int[recordCount];
	for (auto i : view::iota(0U, recordCount))
	{
		readerCount[i] = 0;
		hModifySemaphores[i] = CreateSemaphoreA(NULL, 1, 1, NULL);
	}
	HANDLE* hEndEvents = new HANDLE[clientCount];
	CRITICAL_SECTION* cs = new CRITICAL_SECTION[clientCount];
	HANDLE* hServerThreads = new HANDLE[clientCount];
	for (auto i : view::iota(0U, clientCount))
	{
		InitializeCriticalSection(&cs[i]);
	}
	serverParams** params = new serverParams * [clientCount];
	for (auto i : view::iota(0U, clientCount))
	{
		hEndEvents[i] = CreateEventA(NULL, true, false, NULL);
		params[i] = new serverParams(i, clientCount, recordCount, readerCount, fileName, cs,
			hModifySemaphores, hEndEvents[i]);
		hServerThreads[i] = CreateThread(NULL, 0, Server, (void*)params[i], 0, NULL);
		if (hServerThreads[i] == NULL)
		{
			for (auto j : view::iota(0U, clientCount))
			{
				DeleteCriticalSection(&cs[j]);
			}
			for (auto j : view::iota(0U, i))
			{
				CloseHandle(hServerThreads[j]);
				CloseHandle(hEndEvents[j]);
				delete params[j];
			}
			CloseHandle(hEndEvents[i]);
			delete params[i];
			for (auto j : view::iota(0U, recordCount))
			{
				CloseHandle(hModifySemaphores[j]);
			}
			delete[] cs;
			delete[] params;
			delete[] hServerThreads;
			delete[] hModifySemaphores;
			delete[] hEndEvents;
			delete[] readerCount;
			return GetLastError();
		}
	}

	WaitForMultipleObjects(clientCount, hEndEvents, true, INFINITE);
	cout << "Final file content:" << endl;
	if (!retransmitFile(fileName, recordCount))
	{
		return 0;
	}
	cout << "Press any key to end Server process" << endl;
	_getch();

	for (auto i : view::iota(0U, clientCount))
	{
		DeleteCriticalSection(&cs[i]);
		CloseHandle(hServerThreads[i]);
		CloseHandle(hEndEvents[i]);
		delete params[i];
	}
	for (auto i : view::iota(0U, recordCount))
	{
		CloseHandle(hModifySemaphores[i]);
	}
	delete[] cs;
	delete[] params;
	delete[] hServerThreads;
	delete[] hModifySemaphores;
	delete[] hEndEvents;
	delete[] readerCount;

	cout << "Server process ended" << endl;
	return 0;
}
