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

unsigned int getUInt(string message)
{
	unsigned int n;
	cout << message << endl;
	cin >> n;
	return n;
}
