#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <ranges>

using namespace std;
namespace view = ranges::views;

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

unsigned int getRecordsCount()
{
	unsigned int recordsCount;
	cout << "Enter records count:" << endl;
	cin >> recordsCount;
	cin.ignore();
	return recordsCount;
}

double getWagePerHour()
{
	double wagePerHour;
	cout << "Enter hourly payment:" << endl;
	cin >> wagePerHour;
	cin.ignore();
	return wagePerHour;
}

bool printFileContent(string workingDirectory, string fileName)
{
	ifstream fin(workingDirectory + "\\" + fileName, ios::binary | ios::in);
	if (!fin.is_open())
	{
		cout << "Error while opening \"" << fileName << "\"" << endl;
		return false;
	}
	cout << "Content of \"" << fileName << "\" file:" << endl << endl;
	string line;
	while (getline(fin, line))
	{
		cout << line << endl;
	}
	cout << endl;
	fin.close();
	return true;
}

bool createProcessAndCloseHandle(string name, string commandLine, string workingDirectory)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION piApp;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	if (!CreateProcessA(NULL, (LPSTR)commandLine.c_str(), NULL, NULL, false,
		CREATE_NEW_CONSOLE, NULL, workingDirectory.c_str(), &si, &piApp))
	{
		cout << "Error while creating " << name << " process" << endl;
		return false;
	}
	WaitForSingleObject(piApp.hProcess, INFINITE);
	CloseHandle(piApp.hThread);
	CloseHandle(piApp.hProcess);
	return true;
}

int main(int argc, char* argv[])
{
	string workingDirectory = getCWD(argv);
	string employeeFileName = getFileName("Enter a name of a binary file to fill with records:");
	auto recordsCount = getRecordsCount();

	string creatorCommandLine = "Creator.exe \"" + employeeFileName + "\" " + to_string(recordsCount);
	if (!createProcessAndCloseHandle("Creator", creatorCommandLine, workingDirectory))
	{
		return GetLastError();
	}

	if (!printFileContent(workingDirectory, employeeFileName))
	{
		return 0;
	}

	string reporterFileName = getFileName("Enter a name of a binary file to create a report:");
	double wagePerHour = getWagePerHour();

	string reporterCommandLine = "Reporter.exe \"" + employeeFileName + "\" \"" + reporterFileName + "\" " + to_string(wagePerHour);
	if (!createProcessAndCloseHandle("Reporter", reporterCommandLine, workingDirectory))
	{
		return GetLastError();
	}

	if (!printFileContent(workingDirectory, reporterFileName))
	{
		return 0;
	}

	return 0;
}