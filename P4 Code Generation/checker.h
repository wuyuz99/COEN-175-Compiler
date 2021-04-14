/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

# ifndef CHECKER_H
# define CHECKER_H
# include "Scope.h"

Scope *openScope();
Scope *closeScope();

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Type checkLogicalOR(const Type &left, const Type &right);
Type checkLogicalAND(const Type &left, const Type &right);
Type checkEqualityEQL(const Type &left, const Type &right);
Type checkEqualityNEQ(const Type &left, const Type &right);
Type checkRelationalLE(const Type &left, const Type &right);
Type checkRelationalGE(const Type &left, const Type &right);
Type checkRelationalLT(const Type &left, const Type &right);
Type checkRelationalGT(const Type &left, const Type &right);
Type checkAdditiveADD(const Type&left, const Type&right);
Type checkAdditiveSUB(const Type&left, const Type&right);
Type checkMultiplicativeMUL(const Type &left, const Type &right);
Type checkMultiplicativeDIV(const Type &left, const Type &right);
Type checkMultiplicativeMOD(const Type &left, const Type &right);
Type checkPostfix(const Type &left, const Type &right);
Type checkPrefixDEREF(const Type &op);
Type checkPrefixADDR(const Type &op, const bool lvalue);
Type checkPrefixNOT(const Type &op);
Type checkPrefixNEG(const Type &op);
Type checkPrefixSIZEOF(const Type &op);

void checkExpression(const Type &op);
void checkReturn(const Type &op, const Type &currentReturn);
void checkAssignment(const Type &left, const Type &right, const bool lvalue);

Type checkFunction(const Type &func, Parameters *params);
# endif /* CHECKER_H */
