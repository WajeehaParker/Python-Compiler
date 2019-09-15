
#include<iostream>
#include<fstream>
#include <string>
#include <vector>
#include "LexicalAnalyzer.h"
#include "SyntaxAndSementicAnalyzer.h"

using namespace std;

vector<string> fileInput;

int main() {
	ifstream file("text2.txt", ifstream::in);
	if (file.is_open())
	{
		string line;
		while (getline(file, line))
			fileInput.push_back(line.c_str());
		file.close();
	}

	LexicalAnalyzer lexzer(fileInput);
	lexzer.tokenizer(0, true);
	lexzer.classifier();

	SyntaxAndSementicAnalyzer ssa(lexzer.tokenlist);
	cout << endl << "result: " << ssa.start() << endl;

	system("PAUSE");
	return 0;
}