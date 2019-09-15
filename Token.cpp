#include "Token.h"
#include <iostream>

#include <string>
using namespace std;

Token::Token()
{
	this->classPart = "";
	this->valuePart = "";
	this->line = 0;
}

Token::Token(string CP, string VP, int l)
{
	this->classPart = CP;
	this->valuePart = VP;
	this->line = l;
}

Token::~Token()
{
}

void Token::setToken(string CP, string VP, int l)
{
	this->classPart = CP;
	this->valuePart = VP;
	this->line = l;
}

string Token::getClassPart()
{
	return this->classPart;
}

string Token::getValuePart()
{
	return this->valuePart;
}

int Token::getLineNo()
{
	return this->line;
}