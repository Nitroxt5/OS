#include "Client.h"

int main(int argc, char* argv[])
{
	string procName = argv[1];
	cout << procName << " process started" << endl;
	HANDLE hNamedPipe = CreateFileA("\\\\.\\pipe\\filepipe", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hNamedPipe);
		return GetLastError();
	};
	bool working = true;
	unsigned int choice;
	unsigned int id;
	const char END = 'c';
	while (working)
	{
		choice = getUInt("Choose:\n1. Modify record\n2. Read record\n3. Quit");
		WriteFile(hNamedPipe, &choice, sizeof(int), NULL, NULL);
		switch (choice)
		{
		case 1:
		{
			id = getUInt("Enter record id you want to modify:");
			WriteFile(hNamedPipe, &id, sizeof(int), NULL, NULL);
			employee emp1;
			ReadFile(hNamedPipe, &emp1, sizeof(employee), NULL, NULL);
			if (emp1.num >= 0)
			{
				cout << "Recieved record: " << endl << emp1 << endl;
			}
			else
			{
				cout << "Requested record does not exist!" << endl;
				cout << "Press any key to end operation" << endl;
				_getch();
				WriteFile(hNamedPipe, &END, sizeof(char), NULL, NULL);
				break;
			}
			cout << "Enter new name" << endl;
			char name[10];
			cin.ignore();
			cin.getline(name, 9);
			cout << "Enter new work hours" << endl;
			double hours;
			cin >> hours;
			employee emp(emp1.num, name, hours);
			cout << "Press any key to send new record" << endl;
			_getch();
			WriteFile(hNamedPipe, &emp, sizeof(employee), NULL, NULL);
			cout << "Press any key to end operation" << endl;
			_getch();
			WriteFile(hNamedPipe, &END, sizeof(char), NULL, NULL);
			break;
		}
		case 2:
		{
			id = getUInt("Enter record id you want to read:");
			WriteFile(hNamedPipe, &id, sizeof(int), NULL, NULL);
			employee emp2;
			ReadFile(hNamedPipe, &emp2, sizeof(employee), NULL, NULL);
			if (emp2.num >= 0)
			{
				cout << "Recieved record: " << endl << emp2 << endl;
			}
			else
			{
				cout << "Requested record does not exist!" << endl;
			}
			cout << "Press any key to end operation" << endl;
			_getch();
			WriteFile(hNamedPipe, &END, sizeof(char), NULL, NULL);
			break;
		}
		default:
		{
			working = false;
		}
		}
	}

	CloseHandle(hNamedPipe);

	cout << procName << " process ended" << endl;
	system("pause");
	return 0;
}