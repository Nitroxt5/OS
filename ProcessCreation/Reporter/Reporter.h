#pragma once

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct employee
{
	int num;
	char name[10];
	double hours;
};

ostream& operator<<(ostream& out, const employee& p)
{
	out << p.num << " " << p.name << " " << p.hours;
	return out;
}

istream& operator>>(istream& in, employee& p)
{
	in >> p.num >> p.name >> p.hours;
	return in;
}