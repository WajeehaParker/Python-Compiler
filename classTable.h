#pragma once
#ifndef CLASSTABLE_H
#define CLASSTABLE_H

#include <string>
#include <vector>
#include "functionTable.h"
using namespace std;

class classTable
{
public:
	classTable();
	classTable(string name, string type, Am am = PUBLIC, bool Static = false, bool final = false, string extd1 = "", string extd2="");
	~classTable();
	void setclassTable(string name, string type, Am am, bool Static, bool final , string ext1, string ext2);
	void setLink(vector<functionTable> l);
	vector<functionTable> getLink();
	string getName();
	string getType();
	enum Am getAM();
	bool isStatic();
	bool isFinal();
	string getExtend1();
	string getExtend2();
	

private:
	vector<functionTable> link;
	string name;
	string type;
	enum Am accessModifier;     //1.public,private,protected  2.static  3.final
	bool Static;
	bool final;
	string extend1;
	string extend2;	
};

#endif