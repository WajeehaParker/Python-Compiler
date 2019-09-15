#pragma once
#ifndef SYNTAXANDSEMENTICANALYZER_H
#define SYNTAXANDSEMENTICANALYZER_H
#include "Token.h"
#include "classTable.h"
#include "scopeTable.h"
#include <vector>
using namespace std;


class SyntaxAndSementicAnalyzer {
public:
	SyntaxAndSementicAnalyzer(vector<Token> tl);
	~SyntaxAndSementicAnalyzer();

	/*string classTableLookup(string name);
	string functionTableLookup(string name, string type);
	string scopeTableLookup(string name);

	bool insertClassTable(string name, Am am=PUBLIC, bool Static = false, bool final = false, string extd1 = "", string extd2 = "");
	bool insertFunctionTable(string name, string type, Am am = PUBLIC, bool final = false, bool Static = false);
	bool insertScopeTable(string n, string t);

	string isCompatible(string operand1, string operand2, string op);
*/

	//start
	bool start();
	
	//initialization
	bool initialize();
	bool init1(string N);
	string init2(string T1);
	bool init3(string N);
	bool init4(string N, string accMod);
	string init5();
	string init6(string N, string this_);
	bool id_rel_(string N);
	bool id_rel1_(string N);
	bool AcOP(string N, string PL);
	string AM();
	string static_final();
	string s_final();
	string id_const();
	string Const();
	bool Global();
	string pointer();

	//Expression
	string OE(string T);
	string exp();
	string exp_OR(string T);
	string expAND();
	string exp_AND(string T);
	string expRELOP();
	string exp_RELOP(string T);
	string expPM();
	string exp_PM(string T);
	string expMDM();
	string exp_MDM(string T);
	string exp_F();
	string ID_rel(string N);
	string ID_rel1(string N);
	string ID_rel2(string N);
	string This();
	string In();
	bool In_();

	//body
	bool body();
	bool M_St();
	bool M_St_();
	bool S_St();
	bool S_St1();
	bool S_St2();
	string sst1();
	bool NL();

	//class
	bool Class();
	vector<string> inherit();
	string inherit_();
	bool class_body();
	bool class_body1();
	bool class_body2();
	bool class_body3();

	//class call
	string Class_call();

	//constructor
	bool constructor();
	string arg_list();
	string arg_list1();
	string arg_list_call();
	string arg_list_call1(deque<string> &icg_paramStack);

	//function
	bool function();
	bool function_(string T, string accMod);
	string data_type();

	//loops
	bool For();
	bool For_();
	bool While();

	//if_elif_else
	bool If();
	bool Elif(vector<string> icg_labelStack, bool icg_IfElseFlag);
	bool Else(vector<string> icg_labelStack, bool icg_IfElseFlag);

	//list
	bool list();
	bool list1();
	bool list2();
	bool list3();
	bool list4();
	bool list5();

	//dictionary
	bool Dictionary();
	bool Dictionary1();
	bool Dictionary2();
	bool Dictionary3();
	bool Dictionary4();
	bool Dictionary6();
	bool Dictionary7();
	
	//del
	bool Del();
	string Del_(string N);

	//try_Except_finally
	bool Try();
	bool Except();
	bool Finally();
	bool Exception();
	
	//lambda
	bool lambda();


private:
	vector<Token> tlist;
	int itr;
};


#endif // !SYNTAXANDSEMENTICANALYZER_H

