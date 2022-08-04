#include "Reporter.h"

int main(int argc, char* argv[])
{
	cout << "Reporter process started" << endl;
	string employeeFileName(argv[1]);
	string reportFileName(argv[2]);
	double wagePerHour = atof(argv[3]);

	ifstream fin(employeeFileName, ios::binary | ios::in);
	if (!fin.is_open())
	{
		cout << "Error while opening \"" << employeeFileName << "\" file" << endl;
		return 0;
	}
	ofstream fout(reportFileName, ios::binary | ios::out);
	if (!fout.is_open())
	{
		cout << "Error while opening \"" << reportFileName << "\" file" << endl;
		fin.close();
		return 0;
	}

	fout << "\"" << employeeFileName << "\" file report:" << endl;
	employee e;
	while (fin >> e)
	{
		double resultWage = e.hours * wagePerHour;
		fout << e << " ";
		fout << resultWage << endl;
	}

	fin.close();
	fout.close();
	cout << endl << "\"" << employeeFileName << "\" file report created successfully!" << endl;
	cout << endl << "Reporter process ended" << endl;
	return 0;
}