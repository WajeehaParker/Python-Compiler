//dot operator

#include "LexicalAnalyzer.h"
#include "Token.h"
#include <fstream>
#include <regex>

using namespace std;
struct WL {
	string word;
	unsigned line;
};
Token token;

vector<string> word;
vector<int> line_number;
std::ofstream outfile("output.txt");
vector<WL> wordLine;
static bool lineStart = false;
LexicalAnalyzer::LexicalAnalyzer(std::vector<string> fileInput)
{
	this->input = fileInput;
	input.push_back("~");
}


LexicalAnalyzer::~LexicalAnalyzer()
{
}

void pushTempStr(string &temp, unsigned Line)
{
		if(temp != "")
			wordLine.push_back({ temp, Line + 1 });
		else if (lineStart)
			wordLine.push_back({ temp, Line + 1 });
		temp = "";
}

void LexicalAnalyzer::tokenizer(bool showToken, bool printLine) {
	


	//variable declaration
	string 
		currentChar,
		nextChar, 
		temp,
		lineStr;
	unsigned 
		lineSize,
		noOfLines = input.size();

	bool
		multiLineComment = false,
		isFloat = false,
		eof = false,
		comment = false;
	

	//logic implementation
	for (unsigned line = 0; line < noOfLines ; line++)
	{
		bool lineEmpty = true;
		comment = false;
		lineSize = input.at(line).size();
		lineStr = input.at(line);
		if (lineSize && !regex_match(lineStr, regex("[\\t ]+")))
		{
			
			lineEmpty = false;
			lineStart = true;
				
		}
		else
		{
		
			lineStart = false;
		}
		for (unsigned charIndex = 0; charIndex < lineSize; charIndex++)
		{
			currentChar = lineStr[charIndex];
			if (lineSize > charIndex + 1)
			{
				nextChar = lineStr[charIndex + 1];
			}
			else
			{
				nextChar = "";
			}
			
			if (multiLineComment) 
			{	
				lineStart = false;
				if (currentChar == "#" && nextChar == "#")
				{
					multiLineComment = false;
					lineEmpty = true;
				}
				continue;
			}

		
			if (currentChar == "#")
			{	
				comment = true;
				lineStart = false;
				if (temp != "")
				{
					pushTempStr(temp+"\\n", line);
				}
				if (nextChar == "#")
				{
					multiLineComment = true;
				}
				else
					break;
				continue;
			}
			
			if (isFloat)
			{
				int e = 1;
				temp += currentChar;

				while (lineStr[++charIndex] )
				{	
					if ((lineStr[charIndex] == 'e' || lineStr[charIndex] == 'E') && e)
					{

						e = 0;
						temp += lineStr[charIndex];
						if (lineSize > charIndex + 1)
						{
							if (lineStr[charIndex + 1] == '+' || lineStr[charIndex + 1] == '-')
							{
								temp += lineStr[++charIndex];
							}
							while (lineSize > (charIndex + 1))							//edit
							{
								if (isdigit(lineStr[++charIndex]))
									temp += lineStr[charIndex];
							}
						}
						currentChar = "";
					}
					else if (isalnum(lineStr[charIndex]))
					{
						temp += lineStr[charIndex];
						currentChar = "";
						e = 0;
					}
					else
					{
						pushTempStr(temp, line);
						charIndex--;
						break;
					}
				}
				currentChar = "";
				isFloat = false;
				continue;
			}
			if (lineStart)
			{
				if (currentChar == "\t" || currentChar == " ")
				{
					lineEmpty = true;
					temp += currentChar;
					if (nextChar != "\t" && nextChar != " ")
					{
						pushTempStr(temp, line);
						lineStart = false;
						if(nextChar != "")	lineEmpty = false;
					}
					
					continue;
				}
				else {
					pushTempStr(temp, line);
					lineStart = false;
				}
			}
			if (currentChar == "'")
			{
				pushTempStr(temp, line);
				if (nextChar == "\\")
				{
					if(lineSize > charIndex+3)
					{
						wordLine.push_back({ lineStr.substr(charIndex, 4),line+1 });
						charIndex += 3;

					}
					else
					{
						wordLine.push_back({ lineStr.substr(charIndex, (charIndex+5)-lineSize),line+1 });
						charIndex += (charIndex + 4) - lineSize;

					}
					
				}
				else
				{
					if (lineSize > charIndex + 2)
					{
						wordLine.push_back({ lineStr.substr(charIndex, 3),line+1 });
						charIndex += 2;
					}
					else
					{
						wordLine.push_back({ lineStr.substr(charIndex, (charIndex + 4) - lineSize),line+1 });
						charIndex += (charIndex + 3) - lineSize;
					}
					
					
				}
				continue;
			}
			
			
			if (currentChar == "\"")
			{
				pushTempStr(temp, line);
				temp = currentChar;
				while (++charIndex < lineSize)
				{
					if (lineStr[charIndex] == '\"')
					{
						temp += lineStr[charIndex];
						pushTempStr(temp, line);
						break;
					}
					else if (lineStr[charIndex] == '\\')
					{
						temp += lineStr[charIndex];
						if (++charIndex < lineSize)
						{
							temp += lineStr[charIndex];
							continue;
						}
					}
					else {
						temp += lineStr[charIndex];
					}
			
				}
				pushTempStr(temp, line);
				continue;
			}


			if (isPunctuator(currentChar))
			{
				pushTempStr(temp, line);
				wordLine.push_back({currentChar,line+1});
				continue;
			}
			
			if (currentChar == ".")
			{
				if (!temp.empty())
				{
					//if (isdigit(temp.back()))
					if (regex_match(temp, regex("[+-]?[0-9]+")))
					{
						if (isdigit(nextChar[0]))//1.1
						{
							temp += currentChar;
							isFloat = true;
						}
						else                      //1.a
						{
							pushTempStr(temp, line);
							pushTempStr(currentChar, line);
						}

					}
					else
					{
						pushTempStr(temp, line);
						if (isdigit(nextChar[0]))//a.1
						{
							temp += currentChar;
							isFloat = true;
						}
						else                      //a.a
						{
							pushTempStr(currentChar, line);
						}

					}

				}
				else {
					if (isdigit(nextChar[0]))
					{
						temp = currentChar;
						isFloat = true;
					}
					else
					{
						pushTempStr(currentChar, line);
					}
				}
				continue;
			}

			if (isOperator_(currentChar))
			{
				pushTempStr(temp, line);
				if (isOperator_(currentChar + nextChar))
				{
					wordLine.push_back({ currentChar + nextChar, line+1 });
					++charIndex;
					
				}
				else {
					wordLine.push_back({ currentChar, line+1 });
				}
				continue;
			}

			if (currentChar == " " ||  currentChar == "\t")
			{
				pushTempStr(temp, line);
				continue;
			}
			if (currentChar == "~")
			{
				eof = true;
				temp += currentChar;
				break;

			}
		
			temp += currentChar;
		}
		pushTempStr(temp, line);
		if(!multiLineComment && !lineEmpty && !eof && !comment )
			wordLine.push_back({ "\\n", line + 1 });
		if (eof)
			break;
	}
	if(showToken)
	for (unsigned i = 0; i < wordLine.size(); i++)  //printing word list on console
		std::cout << wordLine.at(i).word << endl;
	if (printLine)
	{	
	
		int i = 0;
		while (i < noOfLines)
		{	
			string l = to_string(i + 1);
			l.resize(4);
			cout <<l<<  input.at(i) << endl;
			if (input.at(i) == "~")
				break;
			i++;
		}
	}
	//cout << "======================" << endl;

}

void LexicalAnalyzer::classifier()
{
	vector<int> stack;
	stack.push_back(0);


	for (unsigned i = 0; i < wordLine.size(); i++)
	{
		string word = wordLine.at(i).word;
		int line = wordLine.at(i).line;
		if (isID_(word, line) != "fals")
		{
			if (isKeyword(word) != "false")
			{
				if (isKeyword(word) == word)
					token = Token(isKeyword(word), isKeyword(word), line);
				else
					token = Token(isKeyword(word), word, line);
			}
			else
				token = Token("ID", word, line);
		}
		else if (isInt(word))
			token = Token("int_const", word, line);
		else if (isFloat(word))
			token = Token("float_const", word, line);
		else if (isChar(word))
		{
			string w = word.substr(1, word.size() - 2);
			token = Token("char_const", w, line);
		}
		else if (isString(word))
		{
			string w = word.substr(1, word.size() - 2);
			token = Token("string_const", w, line);
		}
		else if (isPunctuator(word))
			token = Token(word, "", line);
		else if (isOperator(word) != "false")
		{
			if (isOperator(word) == word)
				token = Token(word, word, line);
			else
				token = Token(isOperator(word), word, line);
		}
		else if (isIndent(word))
		{

			int spaceCount = 0;
			for (unsigned i = 0; i < word.size(); i++)
			{
				if (word[i] == '\t')
				{
					spaceCount = (spaceCount + 8) / 8 * 8;  //??
				}
				else
						spaceCount++;
			}
			if (stack.back() < spaceCount)
			{
				token = Token("IndentInit", to_string(spaceCount), line);
				stack.push_back(spaceCount);
			}
			else if (stack.back() > spaceCount)
			{
				while (stack.back() > spaceCount)
				{
					stack.pop_back();
					token = Token("IndentOut", to_string(stack.back()), line);
					if (stack.back() > spaceCount)
					{
						outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
						tokenlist.push_back(token);
						/*token = Token("nl", "\\n", line);
						outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
						tokenlist.push_back(token);
*/
					}
				}
			}
			else
				continue;
		}
		else if (word == "\\n")
		{
			token = Token("nl", "\\n", line);
		}
		else
						token = Token("InvalidToken", word, line);

		outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
		tokenlist.push_back(token);
	}
}

string LexicalAnalyzer::isID_(string &word, int num)
{
	
	if (word[0] == '_')
	{

		if (word[1] == '_')
		{
			word = word.substr(2);
			token = Token("accessModifier", "private", num);
		
		}
		else {
			word = word.substr(1);
			token = Token("accessModifier", "protected", num);
			
		}
		outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
		tokenlist.push_back(token);
	}
	if (word[0] == '@')
	{
		token = Token("static", "@", num);
		outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
		tokenlist.push_back(token);
		if (word[1] == '$')
		{
			word = word.substr(2);
			token = Token("final", "$", num);
			outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
			tokenlist.push_back(token);
		}
		else {
			word = word.substr(1);
		}
	}
	else if (word[0] == '$')
	{
		token = Token("final", "$", num);
		outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
		tokenlist.push_back(token);
		if (word[1] == '$')
		{
			word = word.substr(2);
			token = Token("static", "@", num);
			outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
			tokenlist.push_back(token);
		}
		else {
			word = word.substr(1);
		}
	}
	
	if (regex_match(word, regex("[A-Za-z][A-Za-z0-9_]*")))
		return word;
	
	return "fals";
}
string LexicalAnalyzer::isID(string word, int num)
{
	string w = "";
	regex b("[A-Za-z][A-Za-z0-9_]*");
	if (word[0] == '_' && word[1] == '_')
	{
		w = word.substr(2, word.size() - 1);
		token = Token("accessmodifier", "private", num);
	}
	else if (word[0] == '_' && word[1] != '_')
	{
		w = word.substr(1, word.size() - 1);
		token = Token("accessmodifier", "protected", num);
	}
	else if (word[0] == '@')
	{
		w = word.substr(1, word.size() - 1);
		token = Token("static", "", num);
	}
	else if (word[0] == '$')
	{
		w = word.substr(1, word.size() - 1);
		token = Token("final", "", num);
	}
	else
	{
		if (regex_match(word, b))
			return word;
	}
	if (w != "")
	{
		if (isKeyword(w) == "false")
		{
			if (regex_match(w, b))
			{
				outfile << "(" << token.getClassPart() << ", " << token.getValuePart() << ", " << token.getLineNo() << ")" << std::endl;
				tokenlist.push_back(token);
				return w;
			}
		}
	}
	return "false";
}
// <=======================================INTEGER===============================================>

bool LexicalAnalyzer::isInt(string word)
{
	regex b("[+-]?[0-9]+");
	if (regex_match(word, b))
		return true;
	return false;
}
bool LexicalAnalyzer::isIndent(string word)
{
	regex b("[\t ]*");
	if (regex_match(word, b))
		return true;
	return false;
}

// <=========================================FLOAT===============================================>

bool LexicalAnalyzer::isFloat(string word)
{
	regex b("[+-]?[0-9]*\\.[0-9]+([Ee][+-]?[0-9]+)?");
	if (regex_match(word, b))
		return true;
	return false;
}

// <=====================================CHARACTER===============================================>

bool LexicalAnalyzer::isChar(string word)
{
	//regex b("^'([^'\\\\])'$");
	regex b("'(\\\\([\\\\'tn0rb])|[a-zA-Z0-9 ])'");
	//regex b("\'(\\(\"|[rbtn0]|\'|\\)|[rbtn0]|\\S)\'");
	if (regex_match(word, b))
		return true;
	return false;
}

// <========================================STRING===============================================>

bool LexicalAnalyzer::isString(string word)
{
	
	regex b("\"([^\\\\\"]|\\\\[rbtn0\"\\\\])*\"");
	//regex b("\"(\\\\([\\\\'tn0rb0-9])|[a-zA-Z0-9|.)*\"");
	//regex b("\"(\\(\"|[rbtn0]|\'|\\)|[rbtn0]|\\S)*\"");
	if (regex_match(word, b))
		return true;
	return false;
}

// <====================================PUNCTUATOR===============================================>

bool LexicalAnalyzer::isPunctuator(string word)
{
	vector<string> punc = { ",", ":", "(", ")", "{", "}", "[", "]"};
	for (unsigned i = 0; i < punc.size(); i++)
		if(punc.at(i)==word)
			return true;
	return false;
}

// <=====================================OPERATOR===============================================>

string LexicalAnalyzer::isOperator(string word)
{
	if (word == "~")
		return "EOF";
	else if (word == "!")
		return "not";
	else if (word == "/" || word == "%")
		return "DM";
	else if (word == "==" || word == "!=" || word == "<=" || word == ">=" || word == "<" || word == ">")
		return "RelOp";
	else if (word == "+=" || word == "-=" || word == "*=" || word == "/=" || word == "%=")
		return "AsOp";
	else if (word == "->" || word == ".")
		return "AcOp";
	else if (word == "+" || word == "-")
		return "PM";
	else if (word == "*" || word == "|" || word == "&" || word == "=" || word == "^" || word == ":" || word == "?" )
		return word;
	return "false";
}



bool LexicalAnalyzer::isOperator_(string word)
{
	
	vector<string> operators = { "!" ,  "/", "%", "==", "!=",  "<=" , ">=", "<" , ">" ,  "+=", "-=", "*=" , "/=", "%=","->", ".", "*",  "|", "&", "=","^",":", "?","+" ,"-" };
	return (find(operators.begin(), operators.end(), word) != operators.end());
}

// <=======================================KEYWORD===============================================>

string LexicalAnalyzer::isKeyword(string word)
{
	if (word == "int" || word == "float" || word == "char" || word == "boolean")
		return "DT";
	else if (word == "True" || word == "1" || word == "False" || word == "0")
		return "bool_const";
	else if (word == "self" || word == "this")
		return "this";

	vector<string> keywords = {"And", "assert", "break", "class", "continue", "def", "del",
		"elif", "else", "except", "finally", "for", "global", "if", "in", "lambda", "new",
		"None", "Not", "Or", "pass", "raise", "return", "string", "super",
		"try", "while", "exception", "List", "dict"};

	for (unsigned i = 0; i < keywords.size(); i++)
		if (keywords.at(i) == word)
			return word;
	return "false";
}