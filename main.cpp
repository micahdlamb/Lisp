#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <map>
#include <stack>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <memory>
#include <array>
#include <set>
using namespace std;

#include "Core.h"
#include "Parser.h"
#include "Functions.h"

struct LispInterpreter {
	Parser p;
	void operator()(string s){
		p.scanner(s);
	}

	bool next(){
		try {
			auto v = p.val();
			p.scanner.warn();
			print(eval(v));
		} catch (string e) {
			cout << e << endl;
		}
		return p.scanner.peek()->type != Token::END;
	}
};

int main(int argc, char* argv[]){
	init();

	//read file then quit
	stringstream ss;
	ss << cin.rdbuf();
	LispInterpreter li;
	li(ss.str());
	while (li.next());
	
	//to read 1 line at a time
	/*
	Parser p;
	while (true){
		string line;
		getline(cin,line);
		try {
			auto x = eval(p.parse(line));
			print(x);
		}
		catch (string e){
			cout << e << endl;
		}
	}
	*/

	string pause;
	cin >> pause;
	return 0;
}


