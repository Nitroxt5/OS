#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>
#include <ranges>

using namespace std;
namespace view = ranges::views;

struct employee
{
	int num;
	char name[10];
	double hours;

	employee(int n, char nam[10], double h)
	{
		num = n;
		strcpy_s(name, nam);
		hours = h;
	}

	employee()
	{
		num = 0;
		strcpy_s(name, "None");
		hours = 0;
	}
};

ostream& operator<<(ostream& out, const employee& p)
{
	out << p.num << " " << p.name << " " << p.hours << endl;
	return out;
}

istream& operator>>(istream& in, employee& p)
{
	in >> p.num >> p.name >> p.hours;
	return in;
}

bool retransmitFile(string fileName, unsigned int count)
{
	ifstream fin(fileName, ios::in | ios::binary);
	if (!fin.is_open())
	{
		cout << "Error while opening \"" << fileName << "\" file" << endl;
		return false;
	}
	for (auto i : view::iota(0U, count))
	{
		employee tmp;
		fin.read((char*)&tmp, sizeof(employee));
		cout << tmp;
	}
	fin.close();
	return true;
}

bool fillFileWithRecords(string fileName, unsigned int recordCount)
{
	ofstream fout(fileName, ios::out | ios::binary);
	if (!fout.is_open())
	{
		cout << "Error while opening \"" << fileName << "\" file" << endl;
		return false;
	}
	cout << "Enter " << recordCount << " employee records" << endl;
	for (auto i : view::iota(0U, recordCount))
	{
		employee emp;
		cin >> emp;
		fout.write((char*)&emp, sizeof(employee));
	}
	fout.close();
	return true;
}

int findRecord(string fileName, int num, unsigned int recordCount, employee& emp)
{
	ifstream fin(fileName, ios::in | ios::binary);
	if (!fin.is_open())
	{
		cout << "Error while opening \"" << fileName << "\" file" << endl;
		return -1;
	}
	unsigned int row = 0;
	for (auto i : view::iota(0U, recordCount))
	{
		fin.read((char*)&emp, sizeof(employee));
		if (emp.num == num)
		{
			row = i;
			break;
		}
		if (i == recordCount - 1)
		{
			emp.num = -1;
		}
	}
	fin.close();
	return row;
}

struct serverParams
{
	unsigned int threadNum;
	unsigned int clientCount;
	unsigned int recordCount;
	unsigned int* readerCount;
	string fileName;
	CRITICAL_SECTION* cs;
	HANDLE* hModifySemaphores;
	HANDLE hEndEvent;

	serverParams(unsigned int n, unsigned int clients, unsigned int records, unsigned int* readers,
		string name, CRITICAL_SECTION* c, HANDLE* hMSem, HANDLE hEndEv)
	{
		threadNum = n;
		clientCount = clients;
		recordCount = records;
		readerCount = readers;
		fileName = name;
		cs = c;
		hModifySemaphores = hMSem;
		hEndEvent = hEndEv;
	}
};

string getFileName(string requestMessage)
{
	string fileName;
	cout << requestMessage << endl;
	getline(cin, fileName);
	return fileName;
}

unsigned int getUInt(string message)
{
	unsigned int n;
	cout << message << endl;
	cin >> n;
	return n;
}
