Micah Lamb - CSE 6341: Lisp Interpreter Project

to compile:
make

to run:
Interpreter < input > output

#DESIGN
The project has been broken into 4 main parts which can be found in the 4 .h files: Scanner.h, Parser.h, Core.h and Functions.h.

#Scanner.h
Instead of writing a general purpose regular expression engine I was able to simply hardcode some simple code to walk through the input string char by char to find tokens.  This was very simple since all tokens could be uniquely identified by their first character.  ( => LPAREN, ) => RPAREN =>, . => DOT, digit => int literal, letter => id.  This is implemented in the function next() which returns the next token found.  Another useful helper function is peek() which allows the parser to see the next token without actually consuming it.

#Parser.h
To implement the parser I didn't think to create a grammar but instead implemented it completely with one simple function val().  The return of val() is the next complete expression found from the token stream.  It is very simple: ) & NIL return NULL, ID, INTL & T return themselves.  Whenever an ( is found I begin a linked list of S-Expressions.  Each new token is added to the list by recursively calling val() until a ) or DOT is found.  If a ) is found then the expression is considered a list and ends with NULL.  A DOT means the linked list ends with the next expression (which could make it continue as a linked list or end as an ATOM).  I prefer to think of the underlying representation as a linked list of linked lists that can end in an ATOM, but this should be equivalent to a binary tree.

#Core.h
This file implements the underlying format for the project.  To allow dynamic typing I created the base class X which all types inherit from.  All functions accept X pointers and must use dynamic casting to determine their actual type.  The following types were defined: SEX to store S-Expressions, ID to hold named variables, Function to store functions, IntL to store ints, and T.  NIL was represented as NULL instead of an actual type.  SEX is very important as it is the type that links the entire expression together by implementing the linked list of X's.  To handle the dot I distinguish between two kinds of lists.  A list that ends in NULL is the normal lisp list which all the operational semantics require (for function calls, cond etc).  When printed it is shown in list notation.  The alternative to a list is a linked list ending in an ATOM.  When printed it is shown in dot notation.

The function eval evaluates an expression.  NIL, T and IntL just evaluate to themselves.  ID's are evaluated by looking up their value in the symbol table.  SEX's are the interesting case.  They must be lists with the first element being being a Function (after looking the ID up in the symbol table).  The return is the result of calling the function with the remaining list items as arguments.

All functions and variable values are stored in 2 global symbol tables.  The tables consist of a stack of maps associating strings to expressions.  As mentioned when ID's are evaluated the returned value is just the associated expression.  Functions which are also represented in expressions as ID's will return a reference to the Function object implementing them.  When looking up an ID, the symbol table always starts at the top of the stack and works its way to the bottom.  This allows a function to access variables available in the calling function.

#Functions.h
Functions all implement the virtual function X* call(args) and come in two flavors, built in (each has its own class) and user defined (see DFunction).  Built in functions are implemented in c++.  User defined functions are created by the built in function defun.  They store a list of param names and a body expression.  To call a user defined function the class DFunction basically pushes a map storing all param and passed value pairs into the symbol table and evaluates the body expression.

#Notes
> I don't allow functions to be defined as T or NIL.  This doesn't really make sense and I didn't find out till last minute.  My whole program beginning with the scanner considers them as their own tokens so it would be too messy to fix.
> $() & $$() are short hand for dynamic_cast<> since it is used so often.
> memory management is handled by unique and shared smart pointers, I couldn't figure out how to do it with new and delete.
> the code can be easily modified to allow functions to be passed as args and obtained as the result of evaluating the first expression in a SEX.  Look in eval.