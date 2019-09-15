#pragma once
#ifndef SCOPETABLE_H
#define SCOPETABLE_H

#include <string>
using namespace std;

class scopeTable
{
public:
	scopeTable();
	scopeTable(string name, string type, int scope);
	~scopeTable();
	void setScopeTable(string name, string type, int scope);
	string getName();
	string getType();
	int getScope();

private:
	string name;
	string type;
	int scope;
};

#endif