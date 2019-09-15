#include "functionTable.h"

#include <string>
using namespace std;

functionTable::functionTable()
{
	this->name = "";
}

functionTable::functionTable(string name, string type, Am am, bool final, bool Static)
{
	this->name = name;
	this->type = type;
	this->accessModifier = am;
	this->final = final;
	this->Static = Static;

}



functionTable::~functionTable()
{
}



void functionTable::setFunctionTable(string name, string type, Am am, bool final, bool Static)
{
	this->name = name;
	this->type = type;
	this->accessModifier = am;
	this->final = final;
	this->Static = Static;
}

string functionTable::getName()
{
	return this->name;
}

string functionTable::getType()
{
	return this->type;
}

Am functionTable::getAM()
{
	return this->accessModifier;
}

bool functionTable::isFinal()
{
	return this->final;
}

bool functionTable::isStatic()
{
	return this->Static;
}
