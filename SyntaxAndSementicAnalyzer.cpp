#include "SyntaxAndSementicAnalyzer.h"
#include <stack>
#include <regex>
#include <fstream>
vector<Token> tlist;
int itr;

//Intermediate Code Generation declaration
int icg_label = 0;
int icg_tempVarCount = 0;
int icg_argCount = 0;
string icg_lineString = "";
string icg_tempOprand = "";

ofstream icg_writer("ICG.txt");
vector<string> icg_tempStack;
vector<string> icg_tempLabelStack;
bool icg_exp = false;
bool icg_argFlag = false;
bool icg_paramTypeFlag = false;

bool isFunc = false, ifCondition=false, isLoop=false, isTry=false, global_=false;
int currentScope = -1, scopeCount=-1;
string currentFuncType;               //holds ongoing function type to check for return statement type.
string currentClass = "global";       //functions and attributes outside class are stored in class named global.
vector<classTable> c_table;
vector<scopeTable> s_table;
stack<int> previous_scope;                            //to keep track in case of nested loops.
stack<bool> isFunc_, ifCondition_, isLoop_, isTry_;   //to keep track in case of nested loops.
stack<string> parentClass;

vector<string> exp_ORSet = { "nl", "]", ")", ",", "in" };
vector<string> exp_ANDSet = { "Or", "nl", "]", ")", ",", "in" };
vector<string> exp_RELOPSet = { "AND", "Or", "nl", "]", ")", ",", "in" };
vector<string> exp_PMSet = { "RelOp", "AND", "Or", "nl", "]", ")", ",", "in" };
vector<string> exp_MDMSet = { "PM", "RelOp", "AND", "Or", "nl", "]", ")", ",", "in" };
vector<string> id_relSet = { "DM", "*", "PM", "RelOp", "AND", "Or", "nl", "]", ")", "=" , ",", "AsOp", "in", ":", "}" };
vector<string> id_rel2Set = { "DM", "*", "PM", "RelOp", "AND", "Or", "nl", "]", ")", "=" , ",", "AsOp", "in", ":", "}" };
vector<string> inSet = { "DM", "*", "PM", "RelOp", "AND", "Or", "nl", "]", ")", "," };
vector<string> elseSet = { "Not", "IndentOut", "def", "class", "*", "accessModifier", "for", "while", "if", "try", "del", "EOF", "nl", "static", "final", "ID", "int_const", "float_const", "string_const", "char_const", "bool_const" };
vector<string> AcOpSet = { "DM", "*", "PM", "RelOp", "AND", "Or", ")", "nl", ",",  "]", "in", "AsOp" };

SyntaxAndSementicAnalyzer::SyntaxAndSementicAnalyzer(vector<Token> tl)
{
	tlist = tl;
	itr = 0;
}

SyntaxAndSementicAnalyzer::~SyntaxAndSementicAnalyzer()
{
	icg_writer.close();
}

int icg_createLabel() {
	return ++icg_label;
}
int icg_createTempVar()
{
	return  ++icg_tempVarCount;
}

int getCurrentClassIndex()
{
	for (int i = 0; i < c_table.size(); i++)
	{
		if (c_table.at(i).getName() == currentClass)
			return i;
	}
	return -1;
}

int getClassIndex(string name)
{
	for (int i = 0; i < c_table.size(); i++)
	{
		if (c_table.at(i).getName() == name)
			return i;
	}
	return -1;
}

vector<string> split(const string & s, string rgx_str = "\\s+")
{
	vector<string> elems;
	regex rgx(rgx_str);
	sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
	sregex_token_iterator end;

	while (iter != end) {
		//std::cout << "S43:" << *iter << std::endl;
		elems.push_back(*iter);
		++iter;
	}
	return elems;
}

bool isVar(string type)
{
	if (type.find(">") != string::npos) {
		return false;
	}
	return true;
}

string getRT(string str)
{
	if (!isVar(str))
	{
		return split(str, ">").back();
	}
	return str;
}

string getPL(string str)
{
	return split(str, ">").front(); 
}

string classTableLookup(string name)
{
	for (int i = 0; i < c_table.size(); i++)
		if (c_table.at(i).getName() == name)
			return c_table.at(i).getType();
	return "false";
}

string functionTableLookup(string name, string type)
{
	vector<functionTable> inherit1, inherit2;
	int index = getCurrentClassIndex(), index1;
	vector<functionTable> link = c_table.at(index).getLink();        //functionTable of current class
	vector<functionTable> link_global = c_table.at(0).getLink();     //functionTable of global class
	if (c_table.at(index).getExtend1() != "")                        //functionTable of inherit class1
	{
		index1 = getClassIndex(c_table.at(index).getExtend1());
		inherit1= c_table.at(index1).getLink();
	}
	if (c_table.at(index).getExtend2() != "")                         //functionTable of inherit class2
	{
		index1 = getClassIndex(c_table.at(index).getExtend2());
		inherit2 = c_table.at(index1).getLink();
	}

	if (isVar(type))                        //if variable searching by name.
	{
		for (int i = 0; i < link.size(); i++)          //in current class
			if (link.at(i).getName() == name)
				return link.at(i).getType();
		if (c_table.at(index).getExtend1() != "")   
			for (int i = 0; i < inherit1.size(); i++)      //in inherit class1
				if (inherit1.at(i).getName() == name)
					return inherit1.at(i).getType();
		if (c_table.at(index).getExtend2() != "")   
			for (int i = 0; i < inherit2.size(); i++)       //in inherit class2
				if (inherit2.at(i).getName() == name)
					return inherit2.at(i).getType();
		for (int i = 0; i < link_global.size(); i++)    //globally
			if (link_global.at(i).getName() == name)
				return link_global.at(i).getType();
	}

	else {                                  //if function, checking name and PL
		for (int i = 0; i < link.size(); i++)            //in current class
			if (link.at(i).getName() == name && getPL(link.at(i).getType())==getPL(type) && getRT(link.at(i).getType())!="")
				return link.at(i).getType();
		if (c_table.at(index).getExtend1() != "")
			for (int i = 0; i < inherit1.size(); i++)    //in inherit class1
				if (inherit1.at(i).getName() == name && getPL(inherit1.at(i).getType()) == getPL(type) && getRT(link.at(i).getType()) != "")
					return inherit1.at(i).getType();
		if (c_table.at(index).getExtend2() != "")
			for (int i = 0; i < inherit2.size(); i++)    //in inherit class2
				if (inherit2.at(i).getName() == name && getPL(inherit2.at(i).getType()) == getPL(type) && getRT(link.at(i).getType()) != "")
					return inherit2.at(i).getType();
		for (int i = 0; i < link_global.size(); i++)    //globally
			if (link_global.at(i).getName() == name && getPL(link_global.at(i).getType()) == getPL(type) && getRT(link.at(i).getType()) != "")
				return link_global.at(i).getType();
	}
	return "false";
}

string scopeTableLookup(string name)
{
	stack<int> s=previous_scope;
	for (int i = 0; i < s_table.size(); i++)        //checking in curren scope
	{
		if (s_table.at(i).getName() == name && s_table.at(i).getScope() == currentScope)
			return s_table.at(i).getType();
	}
	while (!s.empty())                             //iterating through previous scope and checking in them.
	{
		for(int i=0; i<s_table.size(); i++)
			if (s_table.at(i).getName() == name && s_table.at(i).getScope() == s.top())
				return s_table.at(i).getType();
		s.pop();
	}
	return "false";
}

bool insertClassTable(string name, Am am = PUBLIC, bool Static = false, bool final = false, string extd1 = "", string extd2 = "")
{
	if (classTableLookup(name)=="false")
	{
		c_table.push_back(classTable(name, "class", am, Static, final, extd1, extd2));
		return true;
	}
	return false;
}

bool insertFunctionTable(string name, string type, Am am = PUBLIC, bool Final = false, bool Static = false)
{
	int index = getCurrentClassIndex();
	vector<functionTable> link = c_table.at(index).getLink(); //link
	if (functionTableLookup(name, type) == "false")
	{
		link.push_back(functionTable(name, type, am, Final, Static));
		c_table.at(index).setLink(link);
		return true;
	}
	/*else
	{
		if (isVar(type))
			cout << "Redefinition error. Attribute already defined.";
		else
			cout << "Redefinition error. Function already defined.";
	}*/
	return false;
}

bool insertScopeTable(string name, string type)
{
	if (scopeTableLookup(name) == "false")
	{
		s_table.push_back(scopeTable(name, type, currentScope));
		return true;
	}
	cout << "Redefinition error. Attribute already defined.";
	return false;
}

string isCompatible(string operand1, string operand2, string op)
{
	if (op == "")                       //checking types for assignment statement
		if (operand1 == operand2 || (operand1=="float_const" && operand2=="int_const"))
			return operand1;
	if (((operand1 == "string_const" && operand2 == "string_const") || (operand1 == "char_const" && operand2 == "string_const") || (operand1 == "string_const" && operand2 == "char_const")) && op == "+")
		return "string_const";
	if (((operand1 == "float_const" && operand2 == "int_const") || (operand1 == "int_const" && operand2 == "float_const") || (operand1 == "float_const" && operand2 == "float_const")) && (op == "+" || op == "-" || op == "*" || op == "/" || op == "%"))
		return "float_const";
	if ((operand1 == "int_const" && operand2 == "int_const") && (op == "+" || op == "-" || op == "*" || op == "/" || op == "%"))
		return "int_const";
	if ((operand1 == "bool_const" || operand1 == "1" || operand1 == "0") && (operand2 == "bool_const" || operand2 == "1" || operand2 == "0") && (op == "+" || op == "-" || op == "And" || op == "Or"))
		return "bool_const";
	if (operand1 == operand2 && op == "RelOp")
		return operand1;
	if (((operand1 == "int_const" && operand2 == "float_const") || (operand1 == "float_const" && operand2 == "int_const")) && op == "RelOp")
		return "float_const";
	if (((operand1 == operand2 && operand1!="char_const") || (operand1 == "float_const" && operand2 == "int_const")) && op == "AsOp")
		return operand1;
	return "false";
}

bool SyntaxAndSementicAnalyzer::start()
{
	icg_lineString = icg_tempOprand = "";
	icg_exp = false;

	if(classTableLookup("global")=="false")                        //attrinbutes and function outside the class are saved in the class named global
		c_table.push_back(classTable("global", "class"));
	if (tlist.at(itr).getClassPart() == "def")
	{
		itr++;
		if (function())
			if (start())
				return true;
	}
	else if (Class())
	{
		if (start()) 
			return true;
	}
	else if (For())
	{
		if (start())
			return true;
	}
	else if (While())
	{
		if (start())
			return true;
	}
	else if (If())
	{
		if (start())
			return true;
	}
	else if (Try())
	{
		if (start())
			return true;
	}
	else if (Del())
	{
		if (tlist.at(itr).getClassPart() == "nl")
		{
			itr++;
			if (start())
				return true;
		}
	}
	else if (initialize())
	{	
		if (tlist.at(itr).getClassPart() == "nl")
		{
			itr++;
			if (start())
				return true;
		}
	}
	else if (tlist.at(itr).getClassPart() == "EOF")
	{
		itr++;
		return true;
	}
	cout << "Syntax error at line number: " << tlist.at(itr).getLineNo();
	return false;
}
			
bool SyntaxAndSementicAnalyzer::initialize()
{
	string N="", T1, accessMod="", s_f, T, n1;
	bool Static, Final;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		icg_lineString += N = tlist.at(itr).getValuePart();
		itr++;
		if (id_rel_(N))
			return true;
	}
	else if ((T1=Const())!="false")
	{
		if (init2(T1)!="false")
			return true;
	}
	else if (tlist.at(itr).getClassPart() == "*")
	{
		N = "*";
		itr++;
		if (init3(N))
			return true;
	}
	else if (tlist.at(itr).getClassPart() == "accessModifier")
	{
		accessMod = tlist.at(itr).getClassPart();
		itr++;
		if (init4(N, accessMod))
			return true;
	}
	else if ((s_f = static_final()) != "false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N= tlist.at(itr).getValuePart();
			itr++;
			if ((N = ID_rel(N)) != "false")
			{
				if (tlist.at(itr).getClassPart() == "=")
				{
					itr++;
					if ((T = init5()) != "false")
					{
						if (!isFunc)                                          //if outside function insert in function table
						{
							if (s_f == "s") Static = true;                     //static final
							else if (s_f == "f") Static = true;
							else if (s_f == "sf") { Static = true; Final = true; }

							if (!insertFunctionTable(N, T, PUBLIC, Final, Static))
								return false;
						}
						else {                                                 //if inside function insert in scope table
							if (!insertScopeTable(N, T))
								return false;
						}
						return true;
					}
				}
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == "this")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "AcOp")
		{
			itr++;
			if (tlist.at(itr).getClassPart() == "ID")
			{
				N= tlist.at(itr).getValuePart();
				itr++;
				if ((N = ID_rel(N)) != "false")                  //N=N:T   - IDrel may return function call or variable
				{
					T= split(N, ":").back();                     //T and N will be different if function call and same if variable
					N = split(N, ":").front();
					if (T != N)
					{
						if (functionTableLookup(N, T) != "false")  //looking up for function
							if (init1(N))
								return true;
					}
					else {
						if (functionTableLookup(N, "") != "false")  //looking up for variable
							if (init1(N))
								return true;
					}
				}
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == "(")
	{
		itr++;
		if ((T=exp())!="false")
			if (tlist.at(itr).getClassPart() == ")")
			{
				itr++;
				if (OE(T)!="false")
					return true;
			}
	}
	else if (tlist.at(itr).getClassPart() == "Not")
	{
		itr++;
		icg_lineString += "Not ";
		if ((T=exp_F())!="false")
			if ((T1=OE(T))!="false")
			{
				icg_writer << icg_lineString + icg_tempOprand << endl;
				return true;
			}
	}
	else if (list2())
		return true;
	else if (Dictionary2())
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::init1(string N)
{
	stack<string> temp = parentClass;
	string T1, T2, op;
	if(!isFunc)
		T1 = functionTableLookup(N, "");
	else {
		if ((T1 = scopeTableLookup(N)) == "false")
			T1 = functionTableLookup(N, "");
	}
	while (!temp.empty())                   //when dealing with acop
	{
		currentClass = temp.top();
		temp.pop();
	}
	/*string T1 = functionTableLookup(N, "");*/
	if (tlist.at(itr).getClassPart() == "AsOp")
	{
		//op = tlist.at(itr).getValuePart();
		op = tlist.at(itr).getClassPart();
		icg_tempOprand = " = " + icg_lineString + tlist.at(itr).getValuePart()[0];
		itr++;
		if (T1!="false" && ((T2 = exp()) != "false"))
		{
			if (isCompatible(T1, T2, op)!="false")
			{
				icg_writer << icg_lineString + icg_tempOprand << endl;
				icg_lineString = icg_tempOprand = "";
				return true;

			}
		}
	}
	else if (tlist.at(itr).getClassPart() == "=")
	{
		icg_lineString += " = ";
		itr++;
		if ((T2 = init5()) != "false")
		{
			if (T1 == "false")           //if ID not pre-initialized-->T1=false
			{
				if (isFunc) insertScopeTable(N, T2);
				else insertFunctionTable(N, T2);
				return true;
			}
			else if (isCompatible(T1, T2, "") != "false")
				return true;
		}
	}
	else if (init2(T1) != "false" && T1!="false")
		return true;
	return false;
}

string SyntaxAndSementicAnalyzer::init2(string T)
{
	string T2, in;
	/*if (!isFunc)
		T1 = functionTableLookup(N, "");
	else if((T1=scopeTableLookup(N))=="false")
		if ((T1 = functionTableLookup(N, "")) == "false")
		{
			cout << "Variable " + N + " not declared";
			return "false";
		}*/
	if ((T2 = OE(T)) != "false")
	{
		icg_writer << icg_lineString + icg_tempOprand << endl;
		if ((in = In()) != "false")
		{
			if (in == "in") return "bool_const";
			else return T2;
		}
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::init3(string N)
{
	string am="";
	if (tlist.at(itr).getClassPart() == "accessModifier")
	{
		am = tlist.at(itr).getClassPart();
		itr++;
		if (init4(N, am))
			return true;
	}
	else if (init4(N, am))
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::init4(string name, string accMod)
{
	string N = name;
	string s_f, irel, n1, T;
	Am am=PUBLIC;
	bool Static = false, Final = false;
	if ((s_f = static_final()) != "false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			n1= tlist.at(itr).getValuePart();
			itr++;
			if ((irel=ID_rel(n1))!="false")
			{
				N += irel;
				if (tlist.at(itr).getClassPart() == "=")
				{
					itr++;
					if ((T=init5())!="false")
					{
						if (!isFunc)                                            //insert in function table
						{
							if (accMod == "private") am = PRIVATE;             //AM
							else if (accMod == "protected") am = PROTECTED;

							if (s_f == "s") Static = true;                     //static final
							else if (s_f == "f") Static = true;
							else if (s_f == "sf") { Static = true; Final = true; }

							if (!insertFunctionTable(N, T, am, Final, Static))
								return false;
						}
						else {                                                 //insert in scope table
							if (!insertScopeTable(N, T))
								return false;
						}
						return true;
					}
				}
			}
		}
	}
	//else if (tlist.at(itr).getClassPart() == "ID")
	//{
	//	itr++;
	//	if(ID_rel())
	//		if (tlist.at(itr).getClassPart() == "=")
	//		{
	//			itr++;
	//			if (init5())
	//				return true;
	//		}
	//}
	if (tlist.at(itr).getClassPart() == "ID")
	{
		n1= tlist.at(itr).getValuePart();
		itr++;
		if ((irel = ID_rel(n1)) != "false")
		{
			N += irel;
			if (tlist.at(itr).getClassPart() == "=")
			{
				itr++;
				if ((T = init5()) != "false")
				{
					if (!isFunc)                                           //insert in function table
					{
						if (accMod == "private") am = PRIVATE;             //AM
						else if (accMod == "protected") am = PROTECTED;

						if (!insertFunctionTable(N, T, am))
							return false;
					}
					else {                                                 //insert in scope table
						if (!insertScopeTable(N, T))
							return false;
					}
					return true;
				}
			}
		}
	}
	return false;
}

string SyntaxAndSementicAnalyzer::init5()
{
	string T1, N, this_, T2, T;
	if ((T1=Const())!="false")
	{
		if ((T1=OE(T1))!="false")
		{
			icg_writer << icg_lineString + icg_tempOprand << endl;
			return T1;
		}
	}
	else if ((this_=This())!="false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N= tlist.at(itr).getValuePart();
			icg_tempOprand += N;
			itr++;
			if ((N = ID_rel(N)) != "false")
			{
				T = split(N, ":").back();        //T and N will be same if variable and different if function call
				N = split(N, ":").front();
				if (T != N)
				{
					T2 = functionTableLookup(N, T);  //looking up for function
					T2 = split(T2, ">").back();
				}
				else T2 = functionTableLookup(N, "");  //looking up for variable
				if (this_ == "this" && T2 == "false")  //if looking up for class member it must be in functable
					return "false";
				if ((T1 = init6(N, this_)) != "false")
				{
					/*if (isFunc) insertScopeTable(N, T1);
					else insertFunctionTable(N, T1);*/
					return T1;
				}
			}
		}
	}
	else if (list())
	{
		icg_writer << icg_lineString + icg_tempOprand << endl;
		return "list";
	}
	else if (Dictionary())
	{
		icg_writer << icg_lineString + icg_tempOprand << endl;
		return "dictionary";
	}
	else if ((T1=Class_call())!="false")
		return T1;
	else if (lambda())
		return "lambda";
	return "false";
}
//*****
string SyntaxAndSementicAnalyzer::init6(string N, string this_)
{
	string T="", T1;
	if (this_ == "this")            //if this, searching for class member
		T = functionTableLookup(N, "");
	else if (isFunc && scopeTableLookup(N) != "false")   //if in function searching in scopeTable
		T = scopeTableLookup(N);
	else if(functionTableLookup(N, "")!="false")     //searching in class attributes
		T = functionTableLookup(N, "");
	T=split(T, ">").back();
	//if assignment comparing types (T!="")
	//if initialization not comparing types. (T="")

	if (tlist.at(itr).getClassPart() == "=")
	{
		icg_tempStack.push_back(icg_lineString);
		icg_lineString = icg_tempOprand + " = ";
		string temp = icg_tempOprand;
		icg_tempOprand = "";
		itr++;
		if ((T1 = init5()) != "false")
		{
			icg_lineString = icg_tempStack.back() + temp;
			icg_tempStack.pop_back();
			icg_writer << icg_lineString << endl;

			if (T != "" && isCompatible(T, T1, "")=="false")
				return "false";
			if (isFunc) insertScopeTable(N, T1);
			else insertFunctionTable(N, T1);
			return T1;
		}
	}
	else if (T!="" && (T1=init2(T))!="false")
		return T1;
	return "false";
}

bool SyntaxAndSementicAnalyzer::id_rel_(string N)
{
	string PL;
	if (id_rel1_(N))
		return true;
	else if (tlist.at(itr).getClassPart() == "(")
	{
		icg_paramTypeFlag = true;
		itr++;
		if ((PL=arg_list_call())!="false")
			if (tlist.at(itr).getClassPart() == ")")
			{
				icg_writer << icg_lineString << endl;
				itr++;
				if (AcOP(N, PL))
					return true;
			}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::id_rel1_(string N)
{
	if (tlist.at(itr).getClassPart() == "[")
	{
		itr++;
		if (exp()=="int_const")
			if (tlist.at(itr).getClassPart() == "]")
			{
				itr++;
				if (id_rel1_(N+"[]"))
					return true;
			}
	}
	else if (tlist.at(itr).getClassPart() == "AcOp")
	{
		string T;
		if ((isFunc && (T=scopeTableLookup(N)) != "false") || (T=functionTableLookup(N, "")) != "false")    //searching var name in scope and funcTable
		{
			T = split(T, ">").back();
			if (classTableLookup(T) == "false")             //class of the same name must be present in classTable
				return false;
			parentClass.push(currentClass);           //saving currentClass in another variable
			currentClass = T;                      //assigning variable name as currentClass for further calculations
			itr++;
			if (tlist.at(itr).getClassPart() == "ID")
			{
				N = tlist.at(itr).getValuePart();
				itr++;
				if (id_rel_(N))
				{
					currentClass = parentClass.top();
					parentClass.pop();
					return true;
				}
			}
		}
	}
	else if (init1(N))
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::AcOP(string N, string PL)
{
	if (tlist.at(itr).getClassPart() == "AcOp")
	{
		if (functionTableLookup(N, PL) == "false")
			return false;
		parentClass.push(currentClass);
		currentClass = N;
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			itr++;
			if (id_rel_(N))
			{
				currentClass = parentClass.top();
				parentClass.pop();
				return true;
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == "nl")
		return true;
	return false;
}

string SyntaxAndSementicAnalyzer::AM()
{
	if (tlist.at(itr).getClassPart() == "accessModifier")
	{
		return tlist.at(itr++).getValuePart();
	}
	else if (tlist.at(itr).getClassPart() == "static" || tlist.at(itr).getClassPart() == "final" || tlist.at(itr).getClassPart() == "ID")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::static_final()
{
	string s_f;
	string f;
	if (tlist.at(itr).getClassPart() == "static")
	{
		s_f = "s";
		itr++;
		if ((f = s_final()) != "false")
		{
			if (f == "final")
				s_f+="f";
		}
		return s_f;
	}
	else if (tlist.at(itr).getClassPart() == "final")
	{
		itr++;
		return "f";
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::s_final()
{
	if (tlist.at(itr).getClassPart() == "final")
	{
		itr++;
		return "final";
	}
	else if (tlist.at(itr).getClassPart() == "ID")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::id_const()
{
	string T = "", this_, N, N1;
	if ((this_=This())!="false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			icg_tempOprand += N;
			itr++;
			if (N == "false") N = "";
			if ((N = ID_rel(N)) != "false")
			{
				if (!isFunc && functionTableLookup(N, "") == "false")
					return "false";
				else {
					if (scopeTableLookup(N) == "false" && functionTableLookup(N, "") == "false")
						return "false";
				}
				return N;
				/*if (this_ == "this")
				{
					if (functionTableLookup(N, "") == "false")
						return "false";
				}
				else if (scopeTableLookup(N) != "false" || functionTableLookup(N, "") != "false")
					return N;*/
			}
		}
	}
	else if ((T=Const())!="false")
	{
		return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::Const()
{
	if (tlist.at(itr).getClassPart() == "int_const" || tlist.at(itr).getClassPart() == "float_const" || tlist.at(itr).getClassPart() == "string_const" || tlist.at(itr).getClassPart() == "char_const" || tlist.at(itr).getClassPart() == "bool_const")
	{
		if (icg_exp)
		{
			//icg_writer << "T" << icg_createTempVar() << " = " << icg_tempOprand << endl;
			//icg_tempOprand = "T" + to_string(icg_tempVarCount);
			icg_tempStack.push_back(icg_tempOprand);
			icg_tempOprand = tlist.at(itr).getValuePart();
			icg_exp = false;

		}
		else
			icg_tempOprand += tlist.at(itr).getValuePart();
		return tlist.at(itr++).getClassPart();
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::Global()
{
	if (tlist.at(itr).getClassPart() == "global")
	{
		global_ = true;
		itr++;
		if (initialize())
			return true;
	}
	return false;
}

string SyntaxAndSementicAnalyzer::pointer()
{
	if (tlist.at(itr).getClassPart() == "*")
	{
		itr++;
		return "*";
	}
	else if (tlist.at(itr).getClassPart() == "ID")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::OE(string T)
{
	string T1, T2, T3, T4, T5;
	if ((T1=exp_MDM(T))!="false")
		if ((T2 = exp_PM(T1)) != "false")
			if ((T3 = exp_RELOP(T2)) != "false")
				if ((T4 = exp_AND(T3)) != "false")
					if ((T5 = exp_OR(T4)) != "false")
						return T5;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp()
{
	string T1, T2;
	if ((T1=expAND())!="false")
		if ((T2=exp_OR(T1))!="false")
			return T2;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_OR(string T)
{
	string op, T1, T2, T3;
	if (tlist.at(itr).getClassPart() == "Or")
	{
		op = "Or";
		icg_exp = true;
		icg_tempOprand += op;
		itr++;
		if ((T1 = expAND()) != "false")
			if((T2=isCompatible(T, T1, op))!="false")
				if ((T3=exp_OR(T2))!="false")
					return T3;
	}
	else {
		for (int i = 0; i < exp_ORSet.size(); i++)
			if (tlist.at(itr).getClassPart() == exp_ORSet.at(i))
				return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::expAND()
{
	string T1, T2;
	if ((T1 = expRELOP()) != "false")
		if ((T2 = exp_AND(T1)) != "false")
			return T2;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_AND(string T)
{
	string op, T1, T2, T3;
	if (tlist.at(itr).getClassPart() == "And")
	{
		op = "And";
		icg_exp = true;
		icg_tempOprand += op;
		itr++;
		if ((T1 = expRELOP()) != "false")
			if ((T2 = isCompatible(T, T1, op)) != "false")
				if ((T3 = exp_AND(T2)) != "false")
					return T3;
	}
	else {
		for (int i = 0; i < exp_ANDSet.size(); i++)
			if (tlist.at(itr).getClassPart() == exp_ANDSet.at(i))
				return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::expRELOP()
{
	string T1, T2;
	if ((T1 = expPM()) != "false")
		if ((T2 = exp_RELOP(T1)) != "false")
			return T2;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_RELOP(string T)
{
	string op, T1, T2, T3;
	if (tlist.at(itr).getClassPart() == "RelOp")
	{
		op = "RelOp";
		icg_exp = true;
		icg_tempOprand += tlist.at(itr).getValuePart();
		itr++;
		if ((T1 = expPM()) != "false")
			if ((T2 = isCompatible(T, T1, op)) != "false")
			{
				if (!icg_tempStack.empty())
				{
					icg_tempOprand = icg_tempStack.back() + icg_tempOprand;
					icg_tempStack.pop_back();
					icg_writer << "T" << to_string(icg_createTempVar()) + " = " + icg_tempOprand << endl;
					icg_tempOprand = "T" + to_string(icg_tempVarCount);
				}
				if ((T3 = exp_RELOP(T2)) != "false")
					return T3;
			}
	}
	else {
		for (int i = 0; i < exp_RELOPSet.size(); i++)
			if (tlist.at(itr).getClassPart() == exp_RELOPSet.at(i))
				return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::expPM()
{
	string T1, T2;
	if ((T1 = expMDM()) != "false")
		if ((T2 = exp_PM(T1)) != "false")
			return T2;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_PM(string T)
{
	string op, T1, T2, T3;
	if (tlist.at(itr).getClassPart() == "PM")
	{
		icg_exp = true;
		op = tlist.at(itr).getValuePart();

		icg_tempOprand += op;
		icg_tempStack.push_back(icg_tempOprand);
		icg_tempOprand = "";
		icg_exp = false;

		itr++;
		if ((T1 = expMDM()) != "false")
			if ((T2 = isCompatible(T, T1, op)) != "false")
			{
				if (!icg_tempStack.empty())
				{
					icg_tempOprand = icg_tempStack.back() + icg_tempOprand;
					icg_tempStack.pop_back();
					icg_writer << "T" << to_string(icg_createTempVar()) + " = " + icg_tempOprand << endl;
					icg_tempOprand = "T" + to_string(icg_tempVarCount);
				}
				if ((T3 = exp_PM(T2)) != "false")
					return T3;
			}
	}
	else {
		for (int i = 0; i < exp_PMSet.size(); i++)
			if (tlist.at(itr).getClassPart() == exp_PMSet.at(i))
				return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::expMDM()
{
	string T1, T2;
	if ((T1 = exp_F()) != "false")
		if ((T2 = exp_MDM(T1)) != "false")
			return T2;
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_MDM(string T)
{
	string op, T1, T2, T3;
	if (tlist.at(itr).getClassPart() == "DM" || tlist.at(itr).getClassPart() == "*")
	{
		if (tlist.at(itr).getClassPart() == "*") op = "*";
		else op = tlist.at(itr).getValuePart();
		icg_tempOprand += op;
		icg_exp = true;
		itr++;
		if ((T1 = exp_F()) != "false")
			if ((T2 = isCompatible(T, T1, op)) != "false")
			{
				if (!icg_tempStack.empty())
				{
					icg_tempOprand = icg_tempStack.back() + icg_tempOprand;
					icg_tempStack.pop_back();
					icg_writer << "T" << to_string(icg_createTempVar()) + " = " + icg_tempOprand << endl;
					icg_tempOprand = "T" + to_string(icg_tempVarCount);
				}
				if ((T3 = exp_MDM(T2)) != "false")
					return T3;
			}
	}
	else {
		for (int i = 0; i < exp_MDMSet.size(); i++)
			if (tlist.at(itr).getClassPart() == exp_MDMSet.at(i))
				return T;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::exp_F()
{
	string this_, N, in, T;
	if ((this_=This())!="false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			if (icg_exp)
			{
				//icg_writer << "T" << icg_createTempVar() << " = " << icg_tempOprand << endl;
				//icg_tempOprand = "T" + to_string(icg_tempVarCount);
				icg_tempStack.push_back(icg_tempOprand);

				icg_tempOprand = tlist.at(itr).getValuePart();
				icg_exp = false;

			}
			else
				icg_tempOprand += tlist.at(itr).getValuePart();
			N = tlist.at(itr).getValuePart();
			itr++;
			if ((N = ID_rel(N)) != "false")
			{
				if (scopeTableLookup(N) == "false" && functionTableLookup(N, "") == "false")  //ID must be defined b4 the call
					cout << "Variable " + N + " not defined";
				else {
					if ((in = In()) != "false")
					{
						if (in == "in") T = "bool_const";  //in statements always return bool_const
						else {
							if (scopeTableLookup(N) != "false")
								T = scopeTableLookup(N);
							else if (functionTableLookup(N, "") != "false")
								T = functionTableLookup(N, "");
						}
						return T;
					}
				}
			}
		}
	}
	else if ((T=Const())!="false")
	{
		if ((in = In()) != "false")
		{
			if (in == "in") return "bool_const";
			else return T;
		}
	}
	else if (tlist.at(itr).getClassPart() == "(")
	{
		itr++;
		if ((T=exp())!="false")
			if (tlist.at(itr).getClassPart() == ")")
			{
				itr++;
				return T;
			}
	}
	else if (tlist.at(itr).getClassPart() == "Not")
	{
		itr++;
		if ((T=exp_F())!="false")
			return T;
	}
	else if (lambda())
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::ID_rel(string N1)
{
	string N = N1, PL, T;
	if (tlist.at(itr).getClassPart() == "[")
	{
		itr++;
		if(exp()=="int_const")
			if (tlist.at(itr).getClassPart() == "]")
			{
				N += "[]";
				itr++;
				if ((N=ID_rel1(N))!="false")
					return N;
			}
	}
	else if (tlist.at(itr).getClassPart() == "(")
	{
		itr++;
		if ((PL=arg_list_call())!="false")
			if (tlist.at(itr).getClassPart() == ")")
			{
				T = PL+">";	
				itr++;
				if ((N=ID_rel1(N))!="false")
					return N+":"+T;
			}
	}
	else if (tlist.at(itr).getClassPart() == "AcOp")
	{
		if ((isFunc && scopeTableLookup(N) != "false") || functionTableLookup(N, "") != "false")  //checking is var present in currentscope or current funcTable
			if (classTableLookup(N) != "false")      //checking if the class of same name exist
			{
				parentClass.push(currentClass);
				currentClass = N;
				itr++;
				if (tlist.at(itr).getClassPart() == "ID")
				{
					N = tlist.at(itr).getValuePart();
					itr++;
					if ((N = ID_rel(N)) != "false")
					{
						currentClass = parentClass.top();
						parentClass.pop();
						return N;
					}
				}
			}
	}
	else {
	/*	if((isFunc && scopeTableLookup(N)!="false") || functionTableLookup(N, "")!="false")*/
			for (int i = 0; i < id_relSet.size(); i++)
				if (tlist.at(itr).getClassPart() == id_relSet.at(i))
					return N;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::ID_rel1(string N1)
{
	string N = N1;
	if (tlist.at(itr).getClassPart() == "AcOp")
	{
		if (((isFunc && scopeTableLookup(N) != "false") || functionTableLookup(N, "") != "false") && (classTableLookup(N) != "false"))
		{
			parentClass.push(currentClass);
			currentClass = N;
			itr++;
			if (tlist.at(itr).getClassPart() == "ID")
			{
				N = tlist.at(itr).getValuePart();
				itr++;
				if ((N = ID_rel(N)) != "false")
				{
					currentClass = parentClass.top();
					parentClass.pop();
					return N;
				}
			}
		}
	}
	else if ((N=ID_rel2(N))!="false")
		return N;
	return "false";
}

string SyntaxAndSementicAnalyzer::ID_rel2(string N1)
{
	string N = N1;
	if (tlist.at(itr).getClassPart() == "[")
	{
		itr++;
		if (exp()=="int_const")
			if (tlist.at(itr).getClassPart() == "]")
			{
				N += "[]";
				itr++;
				if ((N=ID_rel1(N))!="false")
					return N;
			}
	}
	else {
		for (int i = 0; i < id_rel2Set.size(); i++)
			if (tlist.at(itr).getClassPart() == id_rel2Set.at(i))
				return N;
	}
	return "false";
}

string SyntaxAndSementicAnalyzer::This()
{
	if (tlist.at(itr).getClassPart() == "this")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "AcOp")
		{
			itr++;
			return "this";
		}
	}
	else if (tlist.at(itr).getClassPart() == "ID")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::In()
{
	if (tlist.at(itr).getClassPart() == "in")
	{
		icg_tempOprand += " in ";
		itr++;
		if (In_())
			return "in";
	}
	else
	{
		for (int i = 0; i < inSet.size(); i++)
			if (tlist.at(itr).getClassPart() == inSet.at(i))
				return "";
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::In_()
{
	string N;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		N = tlist.at(itr).getValuePart();
		icg_tempOprand += N;
		itr++;
		if(scopeTableLookup(N)!="false" || functionTableLookup(N, "")!="false")
			return true;
		else cout<<"Variable/function not defined.";
	}
	else if (list())
	{
		return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::body()
{
	if (S_St())
		return true;
	else if (tlist.at(itr).getClassPart() == "nl")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "IndentInit")
		{
			itr++;
			if(M_St())
				if (tlist.at(itr).getClassPart() == "IndentOut")
				{
					itr++;
					return true;
				}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::M_St()
{
	if (S_St())
	{
		//if (N())
		icg_lineString = icg_tempOprand = "";
		if (M_St_())
			return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::M_St_()
{
	if (M_St())
		return true;
	else if (tlist.at(itr).getClassPart() == "IndentOut")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::S_St()
{
	if (S_St1())
	{
		if (tlist.at(itr).getClassPart() == "nl")
		{
			itr++;
			return true;
		}
	}
	else if (S_St2())
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::S_St1()
{
	if (tlist.at(itr).getClassPart() == "break" && ifCondition)
	{
		icg_writer << "JMP " << icg_tempLabelStack.back() << endl;
		itr++;
		return true;
	}
	else if (tlist.at(itr).getClassPart() == "continue" && (ifCondition || isLoop))
	{
		icg_writer << "JMP " << icg_tempLabelStack.front() << endl;
		itr++;
		return true;
	}
	else if (tlist.at(itr).getClassPart() == "return" && isFunc)
	{
		itr++;
		if (sst1() == currentFuncType)
		{
			icg_writer << "RET " + icg_tempOprand << endl;
			return true;
		}
		else cout << "Return type and function type do not match.";
	}
	else if (Del())
		return true;
	else if (Global())
		return true;
	else if (initialize())
		return true;
	else if (tlist.at(itr).getClassPart() == "pass")
	{
		icg_writer << "halt" << endl;
		itr++;
		return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::S_St2()
{
	if (For())
		return true;
	else if (While())
		return true;
	else if (If())
		return true;
	else if (Try())
		return true;
	return false;
}

string SyntaxAndSementicAnalyzer::sst1()
{
	string T;
	if ((T=exp())!="false")
		return T;
	else if (list2())
		return "List";
	else if (Dictionary2())
		return "dict";
	return "false";
}

bool SyntaxAndSementicAnalyzer::NL()
{
	if (tlist.at(itr).getClassPart() == "nl")
	{
		itr++;
		return true;
	}
	else if (tlist.at(itr).getClassPart() == "IndentOut") {
		return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Class()
{
	icg_lineString = icg_tempOprand = "";
	string N;
	vector<string> v;
	if (tlist.at(itr).getClassPart() == "class")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			currentClass = N;
			itr++;
			if (classTableLookup(N)!="false")
				cout << "Class already defined";
			else
			{
				if (tlist.at(itr).getClassPart() == "(")
				{
					itr++;
					if ((v = inherit()).at(0) != "false")
					{
						if (tlist.at(itr).getClassPart() == ")")
						{
							itr++;
							insertClassTable(N, PUBLIC, false, false, v.at(1), v.at(0));
							if (tlist.at(itr).getClassPart() == ":")
							{
								itr++;
								if (tlist.at(itr).getClassPart() == "nl")
								{
									itr++;
									if (tlist.at(itr).getClassPart() == "IndentInit")
									{
										itr++;
										if (class_body())
											if (NL())
											{
												if (tlist.at(itr).getClassPart() == "IndentOut")
												{
													currentClass="global";
													itr++;
													return true;
												}
											}

									}
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

vector<string> SyntaxAndSementicAnalyzer::inherit()
{
	string N1, N2;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		N1 = tlist.at(itr).getValuePart();
		itr++;
		if (classTableLookup(N1) == "false")
			cout << "Inheritted class not defined";
		else if ((N2 = inherit_()) != "false")
			return {N1, N2};
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return {"", ""};
	return {"false"};
}

string SyntaxAndSementicAnalyzer::inherit_()
{
	string N;
	if (tlist.at(itr).getClassPart() == ",")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			itr++;
			if (classTableLookup(N) == "false")
				cout << "Inheritted class not defined";
			else return N;
		}
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return "";
	return "false";
}

bool SyntaxAndSementicAnalyzer::class_body()
{
	if (class_body1())
		return true;
	else if (tlist.at(itr).getClassPart() == "pass")
	{
		itr++;
		return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::class_body1()
{
	icg_tempOprand = "";
	icg_lineString = currentClass + "_";
	if (initialize())
	{
		if (tlist.at(itr).getClassPart() == "nl")
		{
			itr++;
			if (class_body3())
				return true;
		}
	}
	else if (tlist.at(itr).getClassPart() == "def")
	{
		itr++;
		if (class_body2())
			if(class_body3())
				return true;
	}

	return false;
}

bool SyntaxAndSementicAnalyzer::class_body2()
{
	if (constructor())
	{
		return true;
	}
	else if (function())
	{
		return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::class_body3()
{
	if (class_body1())
		return true;
	else if (tlist.at(itr).getClassPart() == "IndentOut")
		return true;
	return false;
}

string SyntaxAndSementicAnalyzer::Class_call()
{
	string PL, N, T;
	if (tlist.at(itr).getClassPart() == "new")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			itr++;
			if (tlist.at(itr).getClassPart() == "(")
			{
				itr++;
				if ((PL=arg_list_call())!="false")
				{
					if (tlist.at(itr).getClassPart() == ")")
					{
						itr++;
						return PL+">"+N;
					}
				}
			}
		}
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::constructor()
{
	string T, N, accMod;
	Am am = PUBLIC;
	if ((accMod=AM())!="false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			icg_lineString += N;
			if (N != currentClass)
				cout<<"Name could not be assigned to the constructor";
			else {
				itr++;
				if (tlist.at(itr).getClassPart() == "(")
				{
					itr++;
					if ((isFunc || ifCondition || isLoop || isTry) == true)
						previous_scope.push(currentScope);   //if already in scope saving the scope number in stack
					currentScope = ++scopeCount;
					if ((T=arg_list())!="false")
						if (tlist.at(itr).getClassPart() == ")")
						{
							icg_lineString += ", " + to_string(icg_argCount) + " proc";
							icg_writer << icg_lineString << endl;
							icg_lineString = "	";
							icg_argFlag = false;
							icg_argCount = 0;
							itr++;
							if (accMod == "private") am = PRIVATE;             //AM
							else if (accMod == "protected") am = PROTECTED;

							if (!insertFunctionTable(N, T+">", am))
								return false;
							
							if (tlist.at(itr).getClassPart() == ":")
							{
								itr++;
								isFunc_.push(isFunc);          //saving the previous state of isFunc variable in stack
								isFunc = true;                 //making isFunc true for current function
								if (body())
								{
									icg_writer << "endp" << endl << endl;
									if (isFunc_.empty()) isFunc = false;
									else {
										isFunc = isFunc_.top();            //restoring previous isFunc value
										isFunc_.pop();
									}
									if (!previous_scope.empty())
									{
										currentScope = previous_scope.top();   //restoring previous scope value
										previous_scope.pop();
									}
									return true;
								}
							}
						}
				}
			}

		}
	}
	return false;
}

string SyntaxAndSementicAnalyzer::arg_list()
{
	icg_argFlag = true;
	string T, T1, N;
	if ((T=data_type())!="false")
	{
		if ((N=pointer())!="false")
		{
			if (tlist.at(itr).getClassPart() == "ID")
			{
				N += tlist.at(itr).getValuePart();
				itr++;
				if(insertScopeTable(N, T))
					if ((T1=arg_list1())!="false")
					{
						if(T1!="") T += "," + T1;
						return T;
					}
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::arg_list1()
{
	string T, T1, N;
	if (tlist.at(itr).getClassPart() == ",")
	{
		itr++;
		if ((T=data_type())!="false")
		{
			if ((N=pointer())!="false")
			{
				if (tlist.at(itr).getClassPart() == "ID")
				{
					N += tlist.at(itr).getValuePart();
					itr++;
					if (insertScopeTable(N, T))
						if ((T1=arg_list1())!="false")
						{
							if (T1 != "") T += "," + T1;
							return T;
						}
				}
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::arg_list_call()
{
	deque<string> icg_paramStack;
	icg_tempStack.push_back(icg_lineString);
	icg_lineString = icg_tempOprand;
	icg_tempOprand = "";
	string T, T1;
	if ((T=exp())!="false")
	{
		icg_lineString += "_" + split(T, "_").front();
		icg_argCount++;
		icg_paramStack.push_front(icg_tempOprand);
		icg_tempOprand = "";

		if ((T1=arg_list_call1(icg_paramStack))!="false")
		{
			for each (string icg_param in icg_paramStack)
			{
				icg_writer << "Param " << icg_param << endl;
			}
			icg_tempOprand = "Call " + currentClass + "_" + icg_lineString + ", " + to_string(icg_argCount) + "";
			icg_lineString = icg_tempStack.back();
			icg_tempStack.pop_back();
			icg_argCount = 0;

			if (T1 != "") T += "," + T1;
			return T;
		}
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return "";
	return "false";
}

string SyntaxAndSementicAnalyzer::arg_list_call1(deque<string> &icg_paramStack)
{
	string T, T1;
	if (tlist.at(itr).getClassPart() == ",")
	{
		itr++;
		if ((T = exp()) != "false")
		{
			icg_lineString += "_" + split(T, "_").front();
			icg_argCount++;
			icg_paramStack.push_front(icg_tempOprand);
			icg_tempOprand = "";

			if ((T1 = arg_list_call1(icg_paramStack)) != "false")
			{
				if (T1 != "") T += "," + T1;
				return T;
			}
		}
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return "";
	return "false";
}

bool SyntaxAndSementicAnalyzer::function()
{
	icg_lineString = currentClass + "_";
	string T, am;
	if ((T=data_type())!="false")
	{
		currentFuncType = T;
		if ((am=AM())!="false")
		{
			if (function_(T, am))
				return true;
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::function_(string T1, string accMod)
{
	string s_f, N, T;
	bool Final=false, Static = false;
	Am am = PUBLIC;
	if ((s_f=static_final())!="false")
	{
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			icg_lineString += N;
			itr++;
			if (tlist.at(itr).getClassPart() == "(")
			{
				itr++;
				if ((isFunc || ifCondition || isLoop || isTry) == true)
					previous_scope.push(currentScope);
				currentScope = ++scopeCount;
				if ((T=arg_list())!="false")
					if (tlist.at(itr).getClassPart() == ")")
					{
						icg_argFlag = false;
						icg_lineString += ", " + to_string(icg_argCount) + " Proc";
						icg_argCount = 0;
						icg_writer << icg_lineString << endl;
						icg_lineString = "	";

						itr++;
						T += ">" + T1;

						if (accMod == "private") am = PRIVATE;
						else if (accMod == "protected") am = PROTECTED;
						if (s_f == "s") Static = true;
						else if (s_f == "f") Final = true;
						else if (s_f == "sf") { Static = true; Final = true; }

						if (!insertFunctionTable(N, T, am, Final, Static))
							return false;

						if (tlist.at(itr).getClassPart() == ":")
						{
							itr++;
							isFunc_.push(isFunc);
							isFunc = true;
							if (body())
							{
								icg_writer << "endp" << endl;
								if (isFunc_.empty()) isFunc = false;
								else {
									isFunc = isFunc_.top();
									isFunc_.pop();
								}
								if (!previous_scope.empty())
								{
									currentScope = previous_scope.top();
									previous_scope.pop();
								}
								return true;
							}
						}
					}
			}

		}
	}
	else if (tlist.at(itr).getClassPart() == "ID")
	{
		N = tlist.at(itr).getValuePart();
		icg_lineString += N;
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			itr++;
			if ((isFunc || ifCondition || isLoop || isTry) == true)
				previous_scope.push(currentScope);
			currentScope = ++scopeCount;
			if ((T = arg_list()) != "false")
				if (tlist.at(itr).getClassPart() == ")")
				{
					icg_argFlag = false;
					icg_lineString += ", " + to_string(icg_argCount) + " Proc";
					icg_argCount = 0;
					icg_writer << icg_lineString << endl;
					icg_lineString = "	";

					itr++;
					T += ">" + T1;

					if (accMod == "private") am = PRIVATE;
					else if (accMod == "protected") am = PROTECTED;

					if (!insertFunctionTable(N, T, am))
						return false;

					if (tlist.at(itr).getClassPart() == ":")
					{
						itr++;
						isFunc_.push(isFunc);
						isFunc = true;
						if (body())
						{
							icg_writer << "endp" << endl;
							if (isFunc_.empty()) isFunc = false;
							else {
								isFunc = isFunc_.top();
								isFunc_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							return true;
						}
					}
				}
		}

	}
	return false;
}

string SyntaxAndSementicAnalyzer::data_type()
{
	string T;
	if (tlist.at(itr).getClassPart() == "DT" || tlist.at(itr).getClassPart() == "string" || tlist.at(itr).getClassPart() == "ID" || tlist.at(itr).getClassPart() == "List" || tlist.at(itr).getClassPart() == "dict")
	{
		T= tlist.at(itr).getClassPart();
		if (icg_argFlag)
		{
			icg_lineString += "_" + tlist.at(itr).getValuePart();
			icg_argCount++;
		}
		if(T !="List" && T !="dictionary" && T!="ID")
			T = tlist.at(itr).getValuePart()+"_const";
		if (tlist.at(itr).getClassPart() == "ID")        //checking if class is present or not
			if (classTableLookup(tlist.at(itr).getValuePart())=="false")
				return "false";
		itr++;
		return T;
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::For()
{
	vector<string> icg_labelStack;
	string N;
	if (tlist.at(itr).getClassPart() == "for")
	{
		icg_labelStack.push_back("L" + to_string(icg_createLabel()));
		icg_writer << endl << icg_labelStack.back() << ":" << endl;
		itr++;
		if ((isFunc || ifCondition || isLoop || isTry) == true)
			previous_scope.push(currentScope);
		currentScope = ++scopeCount;

		if (tlist.at(itr).getClassPart() == "ID")
		{
			icg_lineString = "T" + to_string(icg_createTempVar());
			icg_tempOprand += " = " + tlist.at(itr).getValuePart();
			/*N = tlist.at(itr).getValuePart();
			if ((functionTableLookup(N, "") == "false") && scopeTableLookup(N) == "false")
			{
				cout << "Variable " + N + " not defined";
				return false;
			}*/
			itr++;
			if (tlist.at(itr).getClassPart() == "in")
			{
				icg_tempOprand += " in ";
				itr++;
				if (For_())
				{
					icg_writer << icg_lineString + icg_tempOprand << endl;
					icg_labelStack.push_back("L" + to_string(icg_createLabel()));
					icg_writer << "if(" << icg_lineString << " == False) JMP " << icg_labelStack.back() << endl;
					icg_lineString = icg_tempOprand = "";
					icg_tempLabelStack = icg_labelStack;
					if (tlist.at(itr).getClassPart() == ":")
					{
						itr++;
						isLoop_.push(isFunc);
						isLoop = true;
						if (body())
						{
							icg_writer << "JMP " + icg_labelStack.front() << endl;
							icg_writer << icg_labelStack.back() << ":" << endl;
							icg_labelStack.clear();
							if (isLoop_.empty()) isLoop = false;
							else {
								isLoop = isLoop_.top();
								isLoop_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::For_()
{
	string N, T;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		icg_tempOprand += tlist.at(itr).getValuePart();
		N = tlist.at(itr).getValuePart();
		itr++;
		if ((N = ID_rel(N)) != "false")
		{
			T=split(N, ":").back();
			N= split(N, ":").front();
			if (!isFunc)
			{
				if (T != N)
				{
					if (functionTableLookup(N, T) == "false")
						return false;
				}
				else {
					if (functionTableLookup(N, "") == "false")
						return false;
				}
			}
			else {
				if (scopeTableLookup(N) == "false")
					if (functionTableLookup(N, T) == "false")
						return false;
			}
			return true;
		}
	}
	else if (list2())
		return true;
	else if (Dictionary2())
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::While()
{
	vector<string> icg_labelstack;
	if (tlist.at(itr).getClassPart() == "while")
	{
		icg_labelstack.push_back("L" + to_string(icg_createLabel()));
		icg_writer << endl << icg_labelstack.back() << ":" << endl;
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			itr++;
			if (exp()!="false")
			{
				icg_labelstack.push_back("L" + to_string(icg_createLabel()));
				icg_writer << "if(" + icg_tempOprand + " == False) JMP " << icg_labelstack.back() << endl;
				icg_tempOprand = "";
				if (tlist.at(itr).getClassPart() == ")")
				{
					itr++;
					if (tlist.at(itr).getClassPart() == ":")
					{
						itr++;
						if ((isFunc || ifCondition || isLoop || isTry) == true)
							previous_scope.push(currentScope);
						currentScope = ++scopeCount;
						isLoop_.push(isFunc);
						isLoop = true;
						if (body())
						{
							icg_writer << "JMP " << icg_labelstack.front() << endl;
							icg_writer << icg_labelstack.back() << ":" << endl;
							icg_labelstack.clear();
							if (isLoop_.empty()) isLoop = false;
							else {
								isLoop = isLoop_.top();
								isLoop_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::If()
{
	if (tlist.at(itr).getClassPart() == "if")
	{
		vector<string> icg_labelStack;
		bool icg_IfElseFlag = true;
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			itr++;
			if (exp()!="false")
			{
				if (tlist.at(itr).getClassPart() == ")")
				{
					itr++;
					if (tlist.at(itr).getClassPart() == ":")
					{
						icg_lineString += "if(" + icg_tempOprand + " == False) JMP ";
						icg_tempOprand = "L" + to_string(icg_createLabel());
						icg_lineString += icg_tempOprand;
						icg_writer << icg_lineString << endl;
						icg_lineString = "";
						string temp = icg_tempOprand;
						icg_tempOprand = "";

						itr++;
						if ((isFunc || ifCondition || isLoop || isTry) == true)
							previous_scope.push(currentScope);
						currentScope = ++scopeCount;
						ifCondition_.push(isFunc);
						ifCondition = true;
						if (body())
						{
							icg_tempOprand = temp;
							if (isLoop_.empty()) isLoop = false;
							else {
								ifCondition = ifCondition_.top();
								ifCondition_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							if (Elif(icg_labelStack, icg_IfElseFlag))
								return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Elif(vector<string> icg_labelStack, bool icg_IfElseFlag)
{
	if (tlist.at(itr).getClassPart() == "elif")
	{
		if (icg_IfElseFlag)
		{
			icg_labelStack.push_back("L" + to_string(icg_createLabel()));
			icg_IfElseFlag = false;
		}
		icg_writer << "JMP " + icg_labelStack.back() << endl;
		icg_writer << icg_tempOprand << ":" << endl;
		icg_tempOprand = "";
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			itr++;
			if (exp()!="false")
			{
				if (tlist.at(itr).getClassPart() == ")")
				{
					itr++;
					if (tlist.at(itr).getClassPart() == ":")
					{
						icg_lineString += "if(" + icg_tempOprand + " == False) JMP ";
						icg_tempOprand = "L" + to_string(icg_createLabel());
						icg_lineString += icg_tempOprand;
						icg_writer << icg_lineString << endl;
						icg_lineString = "";
						string temp = icg_tempOprand;
						icg_tempOprand = "";

						itr++;
						if ((isFunc || ifCondition || isLoop || isTry) == true)
							previous_scope.push(currentScope);
						currentScope = ++scopeCount;
						ifCondition_.push(isFunc);
						ifCondition = true;
						if (body())
						{
							icg_tempOprand = temp;
							if (isLoop_.empty()) isLoop = false;
							else {
								ifCondition = ifCondition_.top();
								ifCondition_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							if (Elif(icg_labelStack, icg_IfElseFlag))
								return true;
						}
					}
				}
			}
		}
	}
	else if (Else(icg_labelStack, icg_IfElseFlag))
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::Else(vector<string> icg_labelStack, bool icg_IfElseFlag)
{
	if (tlist.at(itr).getClassPart() == "else")
	{
		if (icg_IfElseFlag)
		{
			icg_labelStack.push_back("L" + to_string(icg_createLabel()));
			icg_IfElseFlag = false;
		}

		icg_lineString += "JMP " + icg_labelStack.back();
		icg_writer << icg_lineString << endl;

		icg_writer << icg_tempOprand << ":" << endl;


		icg_lineString = "";
		icg_tempOprand = "";

		itr++;
		if (tlist.at(itr).getClassPart() == ":")
		{
			itr++;
			if ((isFunc || ifCondition || isLoop || isTry) == true)
				previous_scope.push(currentScope);
			currentScope = ++scopeCount;
			ifCondition_.push(isFunc);
			ifCondition = true;
			if (body())
			{
				icg_writer << icg_labelStack.back() << ":" << endl;
				icg_labelStack.pop_back();
				if (isLoop_.empty()) isLoop = false;
				else {
					ifCondition = ifCondition_.top();
					ifCondition_.pop();
				}
				if (!previous_scope.empty())
				{
					currentScope = previous_scope.top();
					previous_scope.pop();
				}
				return true;
			}
		}
	}
	else {
		if (!icg_labelStack.empty())
		{
			icg_writer << icg_tempOprand << ":" << endl;
			icg_writer << icg_labelStack.back() << ":" << endl;
			icg_labelStack.pop_back();
		}
		else
			icg_writer << icg_tempOprand << ":" << endl;

		for (int i = 0; i < elseSet.size(); i++)
			if (tlist.at(itr).getClassPart() == elseSet.at(i))
				return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::list()
{
	if (list2())
		return true;
	else if (tlist.at(itr).getClassPart() == "List")
	{
		icg_tempOprand += "List";
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			icg_tempOprand += "(";
			itr++;
			if (list1())
			{
				if (tlist.at(itr).getClassPart() == ")")
				{
					icg_tempOprand += ")";
					itr++;
					return true;
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::list1()
{
	string N;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		icg_tempOprand += tlist.at(itr).getClassPart();
		N = tlist.at(itr).getValuePart();
		if (!isFunc && functionTableLookup(N, "") == "false")
			return false;
		else {
			if (scopeTableLookup(N) == "false" && functionTableLookup(N, "") == "false")
				return false;
		}
		itr++;
		return true;
	}
	else if (list2())
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::list2()
{
	if (tlist.at(itr).getClassPart() == "[")
	{
		icg_tempOprand += "[";
		itr++;
		if (list3())
		{
			if (tlist.at(itr).getClassPart() == "]")
			{
				icg_tempOprand += "]";
				itr++;
				return true;
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::list3()
{
	if (id_const()!="false")
	{
		if (list4())
			return true;
	}
	else if (list2())
	{
		if (list4())
			return true;
	}
	else if (Dictionary2())
	{
		if (list4())
			return true;
	}
	else if (tlist.at(itr).getClassPart() == "]")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::list4()
{
	if (tlist.at(itr).getClassPart() == ",")
	{
		icg_tempOprand += ", ";
		itr++;
		if (list5())
		{
			return true;
		}
	}
	else if (tlist.at(itr).getClassPart() == "]")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::list5()
{
	if (id_const()!="false")
	{
		if (list4())
			return true;
	}
	else if (list2())
	{
		if (list4())
			return true;
	}
	else if (Dictionary2())
	{
		if (list4())
			return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary()
{
	if (Dictionary2())
	{
		return true;
	}
	else if (tlist.at(itr).getClassPart() == "dict")
	{
		icg_tempOprand += "dict";
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			icg_tempOprand += "(";
			itr++;
			if (Dictionary1())
			{
				if (tlist.at(itr).getClassPart() == ")")
				{
					icg_tempOprand += ")";
					itr++;
					return true;
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary1()
{
	string N;
	if (tlist.at(itr).getClassPart() == "ID")
	{
		icg_tempOprand += tlist.at(itr).getValuePart();
		N = tlist.at(itr).getValuePart();
		if (!isFunc && functionTableLookup(N, "") == "false")
			return false;
		else {
			if (scopeTableLookup(N) == "false" && functionTableLookup(N, "") == "false")
				return false;
		}
		itr++;
		return true;
	}
	else if (Dictionary2())
	{
		return true;
	}
	else if (tlist.at(itr).getClassPart() == ")")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary2()
{
	if (tlist.at(itr).getClassPart() == "{")
	{
		icg_tempOprand += "{";
		itr++;
		if (Dictionary3())
		{
			if (tlist.at(itr).getClassPart() == "}")
			{
				icg_tempOprand += "}";
				itr++;
				return true;
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary3()
{
	if (id_const()!="false")
	{
		if (tlist.at(itr).getClassPart() == ":")
		{
			icg_tempOprand += ":";
			itr++;
			if (Dictionary4())
				return true;
		}
	}
	else if (tlist.at(itr).getClassPart() == "}")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary4()
{
	if (id_const()!="false")
	{
		if (Dictionary6())
		{
			return true;
		}
	}
	else if (Dictionary2())
	{
		if (Dictionary6())
		{
			return true;
		}
	}
	else if (list2())
	{
		if (Dictionary6())
			return true;
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary6()
{
	if (tlist.at(itr).getClassPart() == ",")
	{
		icg_tempOprand += ", ";
		itr++;
		if (Dictionary7())
		{
			return true;
		}
	}
	else if (tlist.at(itr).getClassPart() == "}")
		return true;
	return false;
}

bool SyntaxAndSementicAnalyzer::Dictionary7()
{
	if (id_const()!="false")
	{
		if (tlist.at(itr).getClassPart() == ":")
		{
			icg_tempOprand += ":";
			itr++;
			if (Dictionary4())
				return true;
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Del()
{
	string N;
	if (tlist.at(itr).getClassPart() == "del")
	{
		icg_lineString = "del";
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			icg_tempOprand += N;
			itr++;
			if ((N=Del_(N))!="false")
			{
				icg_writer << icg_lineString + icg_tempOprand << endl;
				if (isFunc)
				{
					if (scopeTableLookup(N) == "false")
						if (functionTableLookup(N, "")=="false")
							return false;
				}
				else if (functionTableLookup(N, "") == "false")
					return false;
				return true;
			}
		}
	}
	return false;
}

string SyntaxAndSementicAnalyzer::Del_(string N)
{
	if (tlist.at(itr).getClassPart() == "[")
	{
		icg_tempOprand += " + ";
		itr++;
		if (exp()=="int")
			if (tlist.at(itr).getClassPart() == "]")
			{
				icg_writer << icg_lineString + icg_tempOprand << endl;
				itr++;
				return N+"[]";
			}
	}
	else {
		if (tlist.at(itr).getClassPart() == "nl")
			return N;
	}
	return "false";
}

bool SyntaxAndSementicAnalyzer::Try()
{
	if (tlist.at(itr).getClassPart() == "try")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == ":")
		{
			itr++;
			if ((isFunc || ifCondition || isLoop || isTry) == true)
				previous_scope.push(currentScope);
			currentScope = ++scopeCount;
			isTry_.push(isFunc);
			isTry = true;
			if (body())
			{
				if (isLoop_.empty()) isLoop = false;
				else {
					isTry = isTry_.top();
					isTry_.pop();
				}
				if (!previous_scope.empty())
				{
					currentScope = previous_scope.top();
					previous_scope.pop();
				}
				if (Except())
					return true;
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Except()
{
	if (Finally())
		return true;
	else if (tlist.at(itr).getClassPart() == "except")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "(")
		{
			itr++;
			if ((isFunc || ifCondition || isLoop || isTry) == true)
				previous_scope.push(currentScope);
			currentScope = ++scopeCount;
			if (Exception())
			{
				if (tlist.at(itr).getClassPart() == ")")
				{
					itr++;
					if (tlist.at(itr).getClassPart() == ":")
					{
						itr++;
						isTry_.push(isFunc);
						isTry = true;
						if (body())
						{
							if (isLoop_.empty()) isLoop = false;
							else {
								isTry = isTry_.top();
								isTry_.pop();
							}
							if (!previous_scope.empty())
							{
								currentScope = previous_scope.top();
								previous_scope.pop();
							}
							if (Except())
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Finally()
{
	if (tlist.at(itr).getClassPart() == "finally")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == ":")
		{
			itr++;
			if ((isFunc || ifCondition || isLoop || isTry) == true)
				previous_scope.push(currentScope);
			currentScope = ++scopeCount;
			isTry_.push(isFunc);
			isTry = true;
			if (body())
			{
				if (isLoop_.empty()) isLoop = false;
				else {
					isTry = isTry_.top();
					isTry_.pop();
				}
				if (!previous_scope.empty())
				{
					currentScope = previous_scope.top();
					previous_scope.pop();
				}
				return true;
			}
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::Exception()
{
	string N;
	if (tlist.at(itr).getClassPart() == "exception")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			insertScopeTable(N, "exception");
			itr++;
			return true;
		}
	}
	return false;
}

bool SyntaxAndSementicAnalyzer::lambda()
{
	string N;
	if (tlist.at(itr).getClassPart() == "lambda")
	{
		itr++;
		if (tlist.at(itr).getClassPart() == "ID")
		{
			N = tlist.at(itr).getValuePart();
			itr++;
			if((isFunc && scopeTableLookup(N)!="false") || (!isFunc && functionTableLookup(N, "") != "false"))
			{
				cout << "identifier already defined";
				return false;
			}
			if (tlist.at(itr).getClassPart() == ":")
			{
				itr++;
				if (exp()!="false")
					return true;
			}
		}
	}
	return false;
}