#pragma once
#ifndef TOKEN_H
#define TOKEN_H
#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <string>
using namespace std;


class Token
{
public:
	Token();
	Token(string CP, string VP, int l);
	~Token();
	void setToken(string CP, string VP, int l);
	string getClassPart();
	string getValuePart();
	int getLineNo();

private:
	string classPart;
	string valuePart;
	int line;
	
};

#endif