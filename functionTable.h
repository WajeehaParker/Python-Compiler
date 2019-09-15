#pragma once
#ifndef FUNCTIONTABLE_H
#define FUNCTIONTABLE_H

#include <string>
#include<vector>
using namespace std;
enum Am
{
	PUBLIC,
	PROTECTED,
	PRIVATE
};
struct Type {
	string PL;
	string RT;
};

class functionTable
{
public:
	functionTable();
	functionTable(string name, string type, Am am = PUBLIC, bool final = false, bool Static = false);
	~functionTable();
	void setFunctionTable(string name, string type, Am am = PUBLIC, bool final = false, bool Static = false);
	string getName();
	string getType();
	Am getAM();
	bool isFinal();
	bool isStatic();

private:
	string name;
	string type;
	enum Am accessModifier;     //1.public,private,protected 
	bool final;
	bool Static;
};

#endif