typedef size_t U;
typedef shared_ptr<void> voidPtr;
#define $(t) dynamic_pointer_cast<t>
#define $$(t) dynamic_cast<t>

template <class T>
string itos(T i){
	stringstream ss;
	ss << i;
	return ss.str();
}

template <class A, class V>
bool contains(A a, V value){
	for (auto i=a.begin(); i != a.end(); i++)
		if (*i == value)
			return true;
	return false;
}

void error(string msg){
	throw "ERROR: " + msg;
}

void quit(string msg){
	cout << "ERROR: " << msg << endl;// << "exiting..." << endl;
	exit(1);
}

struct X {
	typedef shared_ptr<X> ptr;
	virtual string str(){return "X";}
	static string str(const X::ptr& x){
		return x ? x->str() : "NIL";
	}
	virtual ~X(){/*smart pointers take care of everything*/}
};

struct SEX : public X {
	typedef shared_ptr<SEX> ptr;
	X::ptr x;
	X::ptr next;

	SEX():x(X::ptr()), next(X::ptr()){}
	SEX(X::ptr x, X::ptr next):x(x),next(next){}

	X::ptr head(){
		return x;
	}

	X::ptr rest(){
		return next;
	}

	//cuz NIL is a list
	static bool list(const X::ptr& x){
		return !x || ($(SEX)(x) && $(SEX)(x)->list());
	}
	
	bool list(){
		if (next == NULL) return true;
		if (SEX::ptr l = $(SEX)(next))
			return l->list();
		return false;
	}

	//cuz NIL is a list of size 0
	static int size(const ptr& sex){
		return sex ? sex->size() : 0; 
	}
	
	int size(){
		int i=1;
		SEX* mover = this;
		while (mover = $$(SEX*)(mover->next.get()))
			i++;
		return i;
	}

	//innefficient but I like this syntax better :(
	X::ptr operator[](int i){
		SEX* mover = this;
		for (int j=0; j < i; j++)
			mover = $$(SEX*)(mover->next.get());
		return mover->x;
	}

	virtual string str(){
		string s("(");
		auto mover = this;
		s += X::str(mover->x);
		if (list()){
			while(mover = $$(SEX*)(mover->next.get()))
				s += " " + X::str(mover->x);
		} else {
			s += " . " + X::str(next);
		}
		s += ")";
		return s;
	}

	~SEX(){/*smart pointers take care of everything*/}
};

struct ATOM : public X {
	typedef shared_ptr<ATOM> ptr;
	virtual string str(){return "ATOM";}
	virtual ~ATOM(){}
};

struct T : public X {
	typedef shared_ptr<T> ptr;
	virtual string str(){return "T";}
	~T(){/*cout <<"T destructed"<<endl;*/}
};

struct ID : public ATOM {
	typedef shared_ptr<ID> ptr;
	ID(string name):name(name){}
	string name;
	virtual string str(){return name;}
	virtual ~ID(){/*cout << name << " destructed" << endl;*/}
};

struct NumberL : public ATOM {
	virtual string str(){return "NumberL";}
};

struct IntL : public NumberL {
	typedef shared_ptr<IntL> ptr;
	IntL(int v):value(v){}
	int value;
	virtual string str(){return itos(value);}
	~IntL(){/*cout << value << " destructed" << endl;*/}
};

struct Function : public ID {
	typedef shared_ptr<Function> ptr;
	Function(string name):ID(name){}
	virtual X::ptr call(const SEX::ptr& args)=0;
	void checkArgs(int actual, int expected){
		if (actual != expected)
			error(name + " called with " + itos(actual) + " args instead of " + itos(expected));
	}

	void checkArgs(const voidPtr& a1){
		if (!a1) error(name + " called with arg of invalid type");
	}

	void checkArgs(const voidPtr& a1, const voidPtr& a2){
		if (!a1 || !a2) error(name + " called with args of invalid type");
	}
};

struct SymbolTable {
	typedef map<string, X::ptr> M;
	vector<M> ids;
	string type;
	
	SymbolTable(string type="symbol"):type(type){push(M());}

	X::ptr lookup(string name){
		for (auto i=ids.rbegin(); i != ids.rend(); ++i){
			auto j = i->find(name);
			if (j != i->end())
				return j->second;
		}
		error("undefined " + type + ": " + name);
	}

	void add(string name, const X::ptr& x){
		ids.back()[name] = x;
	}

	void push(M m){
		ids.push_back(m);
	}
	void pop(){
		ids.pop_back();
	}
};

SymbolTable variables("variable");
SymbolTable functions("function");

X::ptr eval(const X::ptr& x){
	//null
	if (!x)
		return X::ptr();
	
	//S-EXPR
	if (auto sex = $(SEX)(x)){
		auto first = sex->head();
		//uncomment to allow first item in SEX to be expression returning func
		//if (auto i = $(ID)(eval(first))){
		if (auto i = $(ID)(first)){
			auto s = functions.lookup(i->name);
			if (auto f = $(Function)(s)){
				if (sex->list())
					return f->call($(SEX)(sex->rest()));
				else
					error("S-EXPR beginning with ID is function call and must be a list");
			}
			else
				error("ID " + X::str(s) + " used where function expected");
		} else
			error("first item of S-EXPR must be function: " + X::str(first) + " found");
	}

	//ID
	if (auto i = $(ID)(x)){
		auto s = variables.lookup(i->name);
		//comment out to allow functions to be passed as args
		//return from a quote still won't work though since its never evaluated
		if ($(Function)(s))
			error("functions can't be evaluated :(");
		return s;
	}

	//Int Literal
	if (auto i = $(IntL)(x))
		return x;

	//T
	if (auto t = $(T)(x))
		return x;

	error("Unexpected type");
	return X::ptr();
}

void print (const X::ptr& x){
	cout << X::str(x) << endl;
}
