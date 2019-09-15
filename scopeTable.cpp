#include "scopeTable.h"
#include <string>
using namespace std;

scopeTable::scopeTable()
{
	this->name = "";
	this->type = "";
	this->scope = 0;
}

scopeTable::scopeTable(string name, string type, int scope)
{
	this->name = name;
	this->type = type;
	this->scope = scope;
}

scopeTable::~scopeTable()
{
}

void scopeTable::setScopeTable(string name, string type, int scope)
{
	this->name = name;
	this->type = type;
	this->scope = scope;
}

string scopeTable::getName()
{
	return this->name;
}

string scopeTable::getType()
{
	return this->type;
}

int scopeTable::getScope()
{
	return this->scope;
}


