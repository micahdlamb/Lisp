#pragma once
#include "Scanner.h"

struct Parser {
	Scanner scanner;

	//for line by line
	X::ptr parse(string s){
		scanner(s);
		return val();
	}

	X::ptr val(){
		Token::ptr t = scanner.next();
		//NIL
		if (t->type == Token::NIL)
			return X::ptr();
		//ID or INTL or T
		if (t->type == Token::ID || t->type == Token::INTL || t->type == Token::T)
			return t->get();

		//LPAREN
		if (t->type != Token::LPAREN)
			quit("( ID, INTL or NIL expected");

		//()
		if (scanner.peek()->type == Token::RPAREN){
			scanner.commit();
			return X::ptr();
		}

		SEX::ptr head(new SEX)
			,mover = head;

		while (true){
			mover->x = val();

			if (scanner.peek()->type != Token::RPAREN){
				if (scanner.peek()->type == Token::DOT){//dot syntax
					scanner.commit();
					mover->next = val();
					if (scanner.next()->type != Token::RPAREN)
						quit(") expected");
					break;
				} else {//list syntax
					mover->next = X::ptr(new SEX);
					mover = $(SEX)(mover->next);
				}
			} else {
				scanner.commit();
				break;
			}
		}
		return head;
	}
};