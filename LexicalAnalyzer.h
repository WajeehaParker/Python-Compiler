#pragma once
#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

#include <string>
#include <vector>
#include "Token.h"
using namespace std;


class LexicalAnalyzer
{
public:
	LexicalAnalyzer(std::vector<string> input);
	~LexicalAnalyzer();

	void tokenizer(bool showToken = false, bool printLine = false);

	void classifier();
 

	string isID(string word, int num);
	string isID_(string &word, int num);

	bool isInt(string word);
	bool isFloat(string word);
	bool isChar(string word);
	bool isString(string word);
	bool isPunctuator(string word);
	bool isIndent(string word);
	string isOperator(string word);
    bool isOperator_(string word);

	string isKeyword(string word);

	vector<Token> tokenlist;
private:
	//string str;
	vector<string> input;
	string line;
	
};

#endif