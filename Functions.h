struct DFunction : public Function {
	typedef shared_ptr<DFunction> ptr;
	DFunction(string name):Function(name){}
	vector<string> params;
	X::ptr body;

	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), params.size());
		map<string, X::ptr> m;
		auto a = args;
		for (auto i=params.begin(); i != params.end(); ++i,a=$(SEX)(a->next))
			m[*i] = eval(a->x);
		variables.push(m);
		auto x = eval(body);
		variables.pop();
		return x;
	}
};

//built in funcs
struct Defun : public Function {
	Defun():Function("DEFUN"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 3);
		ID::ptr id;
		X::ptr body = (*args)[2];
		checkArgs(id=$(ID)((*args)[0]));

		vector<string> noRedefine = {"DEFUN","COND","QUOTE","PLUS","MINUS","TIMES","QUOTIENT","REMAINDER","LESS","GREATER","EQ","NULL","ATOM","INT","CAR","CDR","CONS","NIL","T"};
		if (contains(noRedefine,id->name))
			error("can't redefine built in function");

		if (!SEX::list((*args)[1]))
			error("DEFUN arg 1 must be a list of params");		
		SEX::ptr params = $(SEX)((*args)[1]);
			
		auto df = DFunction::ptr(new DFunction(id->name));
		int size = SEX::size(params);
		for (int i=0; i < size; i++){
			ID::ptr p;
			if (p = $(ID)((*params)[i]))
				df->params.push_back(p->name);
			else
				error("param " + itos(i) + " in definition of " + id->name + " must be an id atom");
		}

		df->body = body;
		functions.add(df->name, df);
		return id;
	}
};

struct Cond : public Function {
	Cond():Function("COND"){}
	X::ptr call(const SEX::ptr& args){
		if (!args)
			error(name + " requires at least 1 arg");
		int n = args->size();
		for (int i=0; i < n; i++){
			SEX::ptr pair = $(SEX)((*args)[i]);
			if (!pair || !pair->list() || pair->size() != 2)
				error(name + " arg " + itos(i) + " must be a list of size 2");

			X::ptr b = eval((*pair)[0]);
			if (b)
				return eval((*pair)[1]);
		}
		error("no cond condition evaluated to true");
		return X::ptr();
	}
};

struct Quote : public Function {
	Quote():Function("QUOTE"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		return (*args)[0];
	}
};

struct Plus : public Function {
	Plus():Function("PLUS"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(new IntL(n1->value + n2->value));
	}
};

struct Minus : public Function {
	Minus():Function("MINUS"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(new IntL(n1->value - n2->value));
	}
};

struct Times : public Function {
	Times():Function("TIMES"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(new IntL(n1->value * n2->value));
	}
};

struct Quotient : public Function {
	Quotient():Function("QUOTIENT"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		if (n2->value == 0) error("divide by 0");
		return X::ptr(new IntL(n1->value / n2->value));
	}
};

struct Remainder : public Function {
	Remainder():Function("REMAINDER"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(new IntL(n1->value % n2->value));
	}
};

struct Less : public Function {
	Less():Function("LESS"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(n1->value < n2->value ? new T : NULL);
	}
};

struct Greater : public Function {
	Greater():Function("GREATER"){}
	X::ptr call(const SEX::ptr& args){
		IntL::ptr n1, n2;
		checkArgs(SEX::size(args), 2);
		checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		return X::ptr(n1->value > n2->value ? new T : NULL);
	}
};

struct Eq : public Function {
	Eq():Function("EQ"){}
	X::ptr call(const SEX::ptr& args){
		X::ptr x1, x2;
		checkArgs(SEX::size(args), 2);
		//checkArgs(n1 = $(IntL)(eval((*args)[0])), n2 = $(IntL)(eval((*args)[1])));
		x1 = eval((*args)[0]);
		x2 = eval((*args)[1]);
		if ($(SEX)(x1) || $(SEX)(x2))
			error("Arguments to EQ must be atoms");

		IntL::ptr n1, n2;
		if (n1 = $(IntL)(x1)){//both numbers?
			if (n2 = $(IntL)(x2))
				return X::ptr(n1->value == n2->value ? new T : NULL);
			else
				return X::ptr();
		}
		if (!x1)//both NULL?
			return X::ptr(!x2 ? new T : NULL);
		if ($(T)(x1))//both T?
			return X::ptr($(T)(x2) ? new T : NULL);
		ID::ptr id1, id2;
		if (id1 = $(ID)(x1)){
			if (!(id2 = $(ID)(x2)))
				return X::ptr();
			return X::ptr(id1->name == id2->name ? new T : NULL);
		}

		error("unimplemented EQ case");
		return X::ptr();//shouldn't happen
	}
};

struct NIL : public Function {
	NIL():Function("NULL"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		return X::ptr(eval((*args)[0]) ? NULL : new T);
	}
};

struct Atom : public Function {
	Atom():Function("ATOM"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		auto a = eval((*args)[0]);
		return X::ptr(!a || $(ATOM)(a) ? new T : NULL);
	}
};

struct Int : public Function {
	Int():Function("INT"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		return X::ptr($(IntL)(eval((*args)[0])) ? new T : NULL);
	}
};

struct Car : public Function {
	Car():Function("CAR"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		SEX::ptr s;
		checkArgs(s = $(SEX)(eval((*args)[0])));
		return s->x;
	}
};

struct Cdr : public Function {
	Cdr():Function("CDR"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 1);
		SEX::ptr s;
		checkArgs(s = $(SEX)(eval((*args)[0])));
		return s->next;
	}
};

struct Cons : public Function {
	Cons():Function("CONS"){}
	X::ptr call(const SEX::ptr& args){
		checkArgs(SEX::size(args), 2);
		return X::ptr(new SEX(eval((*args)[0]),eval((*args)[1])));
	}
};

void init(){
	functions.add("DEFUN", X::ptr(new Defun));
	functions.add("COND", X::ptr(new Cond));
	functions.add("QUOTE", X::ptr(new Quote));
	functions.add("PLUS", X::ptr(new Plus));
	functions.add("MINUS", X::ptr(new Minus));
	functions.add("TIMES", X::ptr(new Times));
	functions.add("QUOTIENT", X::ptr(new Quotient));
	functions.add("REMAINDER", X::ptr(new Remainder));
	functions.add("LESS", X::ptr(new Less));
	functions.add("GREATER", X::ptr(new Greater));
	functions.add("EQ", X::ptr(new Eq));
	functions.add("NULL", X::ptr(new NIL));
	functions.add("ATOM", X::ptr(new Atom));
	functions.add("INT", X::ptr(new Int));
	functions.add("CAR", X::ptr(new Car));
	functions.add("CDR", X::ptr(new Cdr));
	functions.add("CONS", X::ptr(new Cons));
}