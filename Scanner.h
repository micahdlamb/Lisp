#pragma once

struct Token {
	enum Type {END, LPAREN, RPAREN, DOT, NIL, T, ID, INTL};
	Type type;
	Token(Type type):type(type){}
	virtual X::ptr get(){return X::ptr();}

	//delete my Tokens automatically after use
	typedef unique_ptr<Token> ptr;
};

struct TToken : public Token {
	TToken():Token(T){}
	X::ptr get(){return X::ptr(new ::T());}
};

struct INTLToken : public Token {
	int v;
	INTLToken(int v):Token(INTL),v(v){}
	X::ptr get(){return X::ptr(new IntL(v));}
};

struct IDToken : public Token {
	string v;
	IDToken(string v):Token(ID),v(v){}
	X::ptr get(){return X::ptr(new ::ID(v));}
};

struct Scanner {
	string s;
	U i, ii, c;

	void operator()(string line){
		i = 0;
		s = line + " ";
	}

	bool digit(char c){
		return c >= '0' && c <= '9';
	}

	bool letter(char c){
		return c >= 'A' && c <= 'z';
	}

	bool whitespace(char c){
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	Token::ptr peek(){
		ii = i;
		auto t = next();
		c = i;
		i = ii;
		return t;
	}

	void commit(){
		i = c;
	}

	//unused
	Token::ptr error(string msg){
		cout << msg << " ... attempting recovery" << endl;
		i++;ii++;
		return next();//keep trying till valid token found
	}

	//handle stupid . errors
	set<string> errors;
	bool warn(){
		if (errors.size()){
			//cout << "scanner warnings:" << endl;
			for (auto i = errors.begin(); i != errors.end(); i++)
				quit(*i);
				//cout << *i << endl;
			errors.clear();
			return true;
		}
		return false;
	}
	void noAtom(){
		if (letter(s[i]) || digit(s[i]))
			errors.insert("space required between . and atom");
	}

	void noDot(){
		if (s[i] == '.')
			errors.insert("space required between . and atom");
	}

	Token::ptr next(){
		while (i < s.length() && whitespace(s[i]))
			i++;

		if (i >= s.length())
			return Token::ptr(new Token(Token::END));

		if (s[i] == '('){
			i++;
			return Token::ptr(new Token(Token::LPAREN));
		}
		if (s[i] == ')'){
			i++;
			return Token::ptr(new Token(Token::RPAREN));
		}
		if (s[i] == '.'){
			i++;
			noAtom();
			return Token::ptr(new Token(Token::DOT));
		}
		if (s[i] == '+' || s[i] == '-' || digit(s[i])){
			string v;
			if (s[i] == '+'){
				++i;
				if (!digit(s[i])) quit("+ must be followed by a number");
			}
			if (s[i] == '-'){
				v += s[i++];
				if (!digit(s[i])) quit("- must be followed by a number");
			}
			while (digit(s[i]))
				v += s[i++];
			noDot();
			return Token::ptr(new INTLToken(atoi(v.c_str())));
		}
		if (letter(s[i])){
			string v;
			v += s[i++];
			while (letter(s[i]) || digit(s[i]))
				v += s[i++];
			noDot();
			transform(v.begin(), v.end(),v.begin(), ::toupper);//uppercase it
			if (v == "NIL")
				return Token::ptr(new Token(Token::NIL));
			else if (v == "T")
				return Token::ptr(new TToken());
			else
				return Token::ptr(new IDToken(v));
		}
		
		quit("scanner fail, unknown char " + itos(s[i]));
		return Token::ptr();
	}

};