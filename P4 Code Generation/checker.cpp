/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		If a symbol is redeclared, the redeclaration is discarded
 *		and the original declaration is retained.
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

# include <iostream>
# include "lexer.h"
# include "checker.h"
# include "tokens.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"


using namespace std;

static Scope *outermost, *toplevel;
static const Type error, integer(INT), voidptr(VOID, 1);

static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string undeclared = "'%s' undeclared";
static string void_object = "'%s' has type void";

static string invalid_return = "invalid return type";
static string invalid_test_expression = "invalid type for test expression";
static string lvalue_required = "lvalue required in expression";
static string invalid_operands = "invalid operands to binary %s";
static string invalid_operand = "invalid operand to unary %s";
static string not_a_function = "called object is not a function";
static string invalid_arguements = "invalid arguments to called function";


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
	outermost = toplevel;

    return toplevel;
}


/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;
    toplevel = toplevel->enclosing();
    return old;
}


/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.  This
 *		definition always replaces any previous definition or
 *		declaration.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol != nullptr) {
	if (symbol->type().isFunction() && symbol->type().parameters()) {
	    report(redefined, name);
	    delete symbol->type().parameters();

	} else if (type != symbol->type())
	    report(conflicting, name);

	outermost->remove(name);
	delete symbol;
    }

    symbol = new Symbol(name, type);
    outermost->insert(symbol);
    return symbol;
}


/*
 * Function:	declareFunction
 *
 * Description:	Declare a function with the specified NAME and TYPE.  A
 *		function is always declared in the outermost scope.  Any
 *		redeclaration is discarded.
 */

Symbol *declareFunction(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, type);
	outermost->insert(symbol);

    } else if (type != symbol->type()) {
	report(conflicting, name);
	delete type.parameters();

    } else
	delete type.parameters();

    return symbol;
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) {
	if (type.specifier() == VOID && type.indirection() == 0)
	    report(void_object, name);

	symbol = new Symbol(name, type);
	toplevel->insert(symbol);

    } else if (outermost != toplevel)
	report(redeclared, name);

    else if (type != symbol->type())
	report(conflicting, name);

    return symbol;
}


/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) {
    	report(undeclared, name);
    	symbol = new Symbol(name, error);
    	toplevel->insert(symbol);
    }

    return symbol;
}

/*
 * Function:	checkLogical
 *
 * Description:	Helper function for ||, &&. Check if both sides are
 *		integer
 */

Type checkLogical(const Type &left, const Type &right, const string &op) {
    if (left == error || right == error) {
        return error;
    }
    if (left.isValue() && right.isValue()) {
        return integer;
    }
    report(invalid_operands, op);
    return error;
}

/*
 * Function:	checkLogicalOr/checkLogicalAnd
 *
 * Description:	Calls checkLogical.
 */

Type checkLogicalOR(const Type &left, const Type &right) {
    return checkLogical(left, right, "||");
}


Type checkLogicalAND(const Type &left, const Type &right) {
    return checkLogical(left, right, "&&");
}

/*
 * Function:	checkEquality
 *
 * Description:	Helper function that Check if operands are compatible,
 *      returns integer type.
 */

Type checkEquality(const Type &left, const Type &right, const string &op) {
    if (left == error || right == error) {
        return error;
    }
    if(left.isCompatibleWith(right)) {
        return integer;
    }
    report(invalid_operands, op);
    return error;
}

/*
 * Function:	checkEqualityEqual/checkEqualityNotEqual
 *
 * Description:	Calls checkEquality.
 */

Type checkEqualityEQL(const Type &left, const Type &right) {
    return checkEquality(left, right, "==");
}


Type checkEqualityNEQ(const Type &left, const Type &right) {
    return checkEquality(left, right, "!=");
}

/*
 * Function:	checkRelational
 *
 * Description:	Helper function that Check if operands are identical value
 *      types, returns integer type.
 */

Type checkRelational(const Type &left, const Type &right, const string &op) {
    if (left == error || right == error) {
        return error;
    }
    if (left.isValue() && right.isValue() && left.promote() == right.promote()) {
        return integer;
    }
    report(invalid_operands, op);
    return error;
}

/*
 * Function:	checkRelationalLE/checkRelationalGE/
 *              checkRelationalLT/checkRelationalGT
 *
 * Description:	Calls checkRelational.
 */


Type checkRelationalLE(const Type &left, const Type &right) {
    return checkRelational(left, right, "<=");
}

Type checkRelationalGE(const Type &left, const Type &right) {
    return checkRelational(left, right, ">=");
}

Type checkRelationalLT(const Type &left, const Type &right) {
    return checkRelational(left, right, "<");
}

Type checkRelationalGT(const Type &left, const Type &right) {
    return checkRelational(left, right, ">");
}

/*
 * Function:	checkAdditive
 *
 * Description:	check if both integer or left is non-void poiunter and
 *      right is integer.
 */

Type checkAdditive(const Type &left, const Type &right) {
    if (left == integer && right == integer) {
        return integer;
    }
    if (left.isPointer() && left != voidptr && right == integer) {
        return left;
    }
    return error;
}

/*
 * Function:	checkAdditiveADD
 *
 * Description:	call checkAdditive, then check if left is int and right is
 *      non-void pointer
 */

Type checkAdditiveADD(const Type &left, const Type &right) {
    if (left == error || right == error) {
        return error;
    }
    Type l = left.promote();
    Type r = right.promote();
    Type tmp = checkAdditive(l, r);
    if (tmp != error) {
        return tmp;
    }
    if (l == integer && r.isPointer() && r!= voidptr) {
        return r;
    }
    report(invalid_operands, "+");
    return error;
}

/*
 * Function:	checkAdditiveSUB
 *
 * Description:	call checkAdditive, then check if both sides are non-void
 *      pointers
 */

Type checkAdditiveSUB(const Type &left, const Type &right) {
    if (left == error || right == error) {
        return error;
    }
    Type l = left.promote();
    Type r = right.promote();
    Type tmp = checkAdditive(l, r);
    if (tmp != error) {
        return tmp;
    }
    if (l.isPointer() && l!= voidptr && r.isPointer() && r != voidptr) {
        if (left.isCompatibleWith(right)) {
            return integer;
        }
    }
    report(invalid_operands, "-");
    return error;
}

/*
 * Function:	checkMultiplicative
 *
 * Description:	Helper function that Check if operands are integer,
 *      returns integer type.
 */

Type checkMultiplicative (const Type &left, const Type &right, const string &op) {
    if (left == error || right == error) {
        return error;
    }
    if (left.isInteger() && right.isInteger()) {
        return integer;
    }
    report(invalid_operands, op);
    return error;
}

/*
 * Function:	checkMultiplicativeMUL/checkMultiplicativeDIV/
 *              checkMultiplicativeMOD
 *
 * Description:	Calls checkMultiplicative.
 */

 Type checkMultiplicativeMUL(const Type &left, const Type &right) {
     return checkMultiplicative(left, right, "*");
 }

 Type checkMultiplicativeDIV(const Type &left, const Type &right) {
     return checkMultiplicative(left, right, "/");
 }

 Type checkMultiplicativeMOD(const Type &left, const Type &right) {
     return checkMultiplicative(left, right, "%");
 }

 /*
  * Function:	checkPostfix
  *
  * Description: Check if left is pointer to non-VOID type T and right is
  *     integer, return T.
  */

 Type checkPostfix(const Type &left, const Type &right) {
     if (left == error || right == error) {
         return error;
     }
     Type l = left.promote();
     if(right != integer){
         report(invalid_operands, "[]");
         return error;
     }

     if (l.isPointer() && l != voidptr) {
         return Type(l.specifier(), l.indirection() - 1);
     }
     report(invalid_operands, "[]");
     return error;
 }

Type checkPrefixDEREF(const Type &op) {
    if (op == error) {
        return error;
    }
    Type tmp = op.promote();

    if(tmp.isPointer() && tmp != voidptr) {
        return Type(tmp.specifier(), tmp.indirection() - 1);
    }
    report(invalid_operand, "*");
    return error;
}

Type checkPrefixADDR(const Type &op, const bool lvalue) {
    if(!lvalue) {
        report(lvalue_required);
        return error;
    }
    if (op == error) {
        return error;
    }
    return Type(op.specifier(), op.indirection() + 1);
}

Type checkPrefixNOT(const Type &op) {
    if (op == error) {
        return error;
    }
    if(!op.isValue()) {
        report(invalid_operand, "!");
        return error;
    }
    return integer;
}

Type checkPrefixNEG(const Type &op) {
    if (op == error) {
        return error;
    }
    Type tmp = op.promote();
    if(tmp.isInteger()) {
        return integer;
    }
    report(invalid_operand, "-");
    return error;
}

Type checkPrefixSIZEOF(const Type &op) {
    if (op == error) {
        return error;
    }
    if(op.isValue()) {
        return integer;
    }
    report(invalid_operand, "sizeof");
    return error;
}

void checkExpression(const Type &expr) {
    if (expr == error) {
        return;
    }
    if(expr.isValue()) {
        return;
    }
    report(invalid_test_expression);
    return;
}

void checkReturn(const Type &op, const Type &currentReturn) {
    if (op == error) {
        return;
    }
    if (op.isCompatibleWith(currentReturn) && op.isScalar()) {
        return;
    }
    report(invalid_return);
    return;
}

void checkAssignment(const Type &left, const Type &right, const bool lvalue) {
    if (!lvalue) {
        report(lvalue_required);
        return;
    }
    if (left == error || right == error) {
        return;
    }
    if (left.isCompatibleWith(right)) {
        return;
    }

    report(invalid_operands, "=");
}

Type checkFunction(const Type &func, Parameters *params) {
    if (func == error) {
        return error;
    }
    if (!func.isFunction()) {
        report(not_a_function);
        return error;
    }
    if (func.parameters() == nullptr) {
        return Type(func.specifier(), func.indirection());
    }
    if (params == nullptr) {
        if (func.parameters()->size() == 0) {
            return Type(func.specifier(), func.indirection());
        } else {
            report(invalid_arguements);
            return error;
        }
    }
    if(func.parameters()->size() != params->size()) {
        report(invalid_arguements);
        return error;
    }
    size_t i = 0;
    for (i = 0; i < params->size(); ++i) {
        if(!(*func.parameters())[i].isCompatibleWith((*params)[i])) {
            report(invalid_arguements);
            return error;
        }
    }

    return Type(func.specifier(), func.indirection());
}
