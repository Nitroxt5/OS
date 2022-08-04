#include "Creator.h"

int main(int argc, char* argv[])
{
	cout << "Creator process started" << endl;
	string filename(argv[1]);
	unsigned int recordsCount = atoi(argv[2]);
	ofstream fout(filename, ios::binary);
	if (!fout.is_open())
	{
		cout << "Error while opening \"" << filename << "\"" << endl;
		return 0;
	}

	cout << "Enter " << recordsCount << " employee records:" << endl;
	for (auto i : view::iota(0U, recordsCount))
	{
		employee e;
		cin >> e;
		fout << e << endl;
	}

	fout.close();
	cout << "Creator process ended" << endl;
	return 0;
}