#include "classTable.h"

#include <string>
using namespace std;

classTable::classTable()
{
}

classTable::classTable(string name, string type, Am am, bool Static, bool final, string extd1, string extd2)
{
	this->name = name;
	this->type = type;
	this->accessModifier = am;
	this->Static = Static;
	this->final = final;
	this->extend1 = extd1;
	this->extend2 = extd2;
}


classTable::~classTable()
{
}

void classTable::setclassTable(string name, string type, Am am, bool Static, bool final, string ext1, string ext2)
{
	this->name = name;
	this->type = name;
	this->accessModifier = am;
	this->Static = Static;
	this->final = final;
	this->extend1 = ext1;
	this->extend2 = ext2;
}

void classTable::setLink(vector<functionTable> l)
{
	this->link = l;
}

vector<functionTable> classTable::getLink()
{
	return this->link;
}

string classTable::getName()
{
	return this->name;
}

string classTable::getType()
{
	return this->type;
}


Am classTable::getAM()
{
	return this->accessModifier;
}



bool classTable::isStatic()
{
	return this->Static;
}

bool classTable::isFinal()
{
	return this->final;
}

string classTable::getExtend1()
{
	return this->extend1;
}

string classTable::getExtend2()
{
	return this->extend2;
}

