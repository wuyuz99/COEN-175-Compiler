/* skeleton code for parser.cpp */
# include <iostream>
# include "lexer.h"
# include "tokens.h"

using namespace std;

int lookahead;
string lexbuf;

static void print(string str){
	cout << str << endl;
}

static void error()
{
	if (lookahead == DONE)
		report("syntax error at end of file");
	else
		report("syntax error at '%s'", lexbuf);

	exit(EXIT_FAILURE);
}

static void match(int t)
{
	if (lookahead != t)
		error();

	lookahead = lexan(lexbuf);
}

void expression();

void expressionList() {
	expression();
	while (lookahead == COM) {
		match(COM);
		expression();
	}
}

void primaryExpression() {
	if (lookahead == LPAR) {
		match(LPAR);
		expression();
		match(RPAR);
	} else if (lookahead == ID){
		match(ID);
		if(lookahead == LPAR){
			match(LPAR);
			if(lookahead != RPAR){
				expressionList();
			}
			match(RPAR);
		}
	} else if (lookahead == NUM) {
		match(NUM);
	} else if (lookahead == STRING) {
		match (STRING);
	}

}

void postfixExpression() {
	primaryExpression();

	while (lookahead == LBRACK) {
		match(LBRACK);
		expression();
		match(RBRACK);
		print("index");
	}
}

void prefixExpression() {
	if (lookahead == ADDR) {
		match(ADDR);
		prefixExpression();
		print("addr");
	} else if (lookahead == DEREF) {
		match(DEREF);
		prefixExpression();
		print("deref");
	} else if (lookahead == NOT) {
		match(NOT);
		prefixExpression();
		print("not");
	} else if (lookahead == NEG) {
		match(NEG);
		prefixExpression();
		print("neg");
	} else if (lookahead == SIZEOF) {
		match(SIZEOF);
		prefixExpression();
		print("sizeof");
	}
	postfixExpression();
}

void multiplicativeExpression() {
	prefixExpression();

	while (lookahead == MUL || lookahead == DIV || lookahead == REM) {
		if (lookahead == MUL) {
			match(MUL);
			prefixExpression();
			print("mul");
		} else if (lookahead == DIV) {
			match(DIV);
			prefixExpression();
			print("div");
		} else if (lookahead == REM) {
			match(REM);
			prefixExpression();
			print("rem");
		}
	}
}

void additiveExpression(){
	multiplicativeExpression();

	while (lookahead == ADD || lookahead == SUB) {
		if (lookahead == ADD) {
			match(ADD);
			multiplicativeExpression();
			print("add");
		} else if (lookahead == SUB) {
			match(SUB);
			multiplicativeExpression();
			print("sub");
		}
	}
}

void relationalExpression() {
	additiveExpression();

	while (lookahead == LTN || lookahead == GTN || lookahead == LEQ || lookahead == GEQ){
		if (lookahead == LTN) {
			match(LTN);
			additiveExpression();
			print("ltn");
		} else if (lookahead == GTN) {
			match(GTN);
			additiveExpression();
			print("gtn");
		} else if (lookahead == LEQ) {
			match(LEQ);
			additiveExpression();
			print("leq");
		} else if (lookahead == GEQ) {
			match(GEQ);
			additiveExpression();
			print("geq");
		}
	}
}

void equalityExpression() {
	relationalExpression();

	while (lookahead == EQL || lookahead == NEQ) {
		if (lookahead == EQL) {
			match(EQL);
			relationalExpression();
			print("eql");
		} else if (lookahead == NEQ) {
			match(NEQ);
			relationalExpression();
			print("neq");
		}
	}
}


void logicalAndExpression() {
	equalityExpression();

	while (lookahead == AND) {
		match(AND);
		equalityExpression();
		print("and");
	}
}

void expression() {
	logicalAndExpression();

	while (lookahead == OR) {
		match(OR);
		logicalAndExpression();
		print("or");
	}
	return;
}

void assignment() {
	expression();
	while (lookahead == ASSIGN) {
		match(ASSIGN);
		expression();
	}
}

void specifier() {
	if (lookahead == INT) {
		match(INT);
	} else if (lookahead == CHAR) {
		match(CHAR);
	} else if (lookahead == VOID) {
		match(VOID);
	}
}

void pointers() {
	while (lookahead == DEREF) {
		match(DEREF);
	}
}

void declarator() {
	pointers();
	match(ID);
	if (lookahead == LBRACK) {
		match(LBRACK);
		match(NUM);
		match(RBRACK);
	}
}

void declarator_list() {
	declarator();
	while (lookahead == COM) {
		match(COM);
		declarator();
	}
}

void declaration () {
	specifier();
	declarator_list();
	match(SEMCOL);
}

void declarations() {
	while(lookahead == INT || lookahead == CHAR || lookahead == VOID) {
		declaration();
	}
}

void statements();

void statement() {
	if (lookahead == LBRACE) {
		match(LBRACE);
		declarations();
		statements();
		match(RBRACE);
	} else if (lookahead == RETURN) {
		match(RETURN);
		expression();
		match(SEMCOL);
	} else if (lookahead == WHILE) {
		match(WHILE);
		match(LPAR);
		expression();
		match(RPAR);
		statement();
	} else if (lookahead == FOR) {
		match(FOR);
		match(LPAR);
		assignment();
		match(SEMCOL);
		expression();
		match(SEMCOL);
		assignment();
		match(RPAR);
		statement();
	} else if (lookahead == IF) {
		match(IF);
		match(LPAR);
		expression();
		match(RPAR);
		statement();
		if(lookahead == ELSE) {
			match(ELSE);
			statement();
		}
	} else {
		assignment();
		match(SEMCOL);
	}
}

void statements() {
	while (lookahead != RBRACE) {
		statement();
	}
}

void global_declarator() {
	pointers();
	match(ID);
	if (lookahead == LPAR) {
		match(LPAR);
		match(RPAR);
	} else if (lookahead == LBRACK) {
		match(LBRACK);
		match(NUM);
		match(RBRACK);
	}
}

void remaining_declarations() {
	while (lookahead == COM) {
		match(COM);
		global_declarator();
	}
	match(SEMCOL);
}

void remaining_parameters();

void parameters() {
	if (lookahead == VOID) {
		match(VOID);
		if (lookahead != RPAR) {
			pointers();
			match(ID);
			remaining_parameters();
		}
	} else if (lookahead == CHAR) {
		match(CHAR);
		pointers();
		match(ID);
		remaining_parameters();
	} else if (lookahead == INT) {
		match(INT);
		pointers();
		match(ID);
		remaining_parameters();
	}
}

void parameter() {
	specifier();
	pointers();
	match(ID);
}

void remaining_parameters() {
	while(lookahead == COM) {
		match(COM);
		parameter();
	}
}

void fog() {
	specifier();
	pointers();
	match(ID);

	if (lookahead == LBRACK) {
		match(LBRACK);
		match(NUM);
		match(RBRACK);
		remaining_declarations();
	} else if (lookahead == LPAR) {
		match(LPAR);
		if (lookahead == RPAR) {
			match(RPAR);
			remaining_declarations();
		} else {
			parameters();
			match(RPAR);
			match(LBRACE);
			declarations();
			statements();
			match(RBRACE);
		}
	} else {
		remaining_declarations();
	}
}

int main() {
	lookahead = lexan(lexbuf);

	while (lookahead != DONE){
		fog();
	}

	exit(EXIT_SUCCESS);
}
