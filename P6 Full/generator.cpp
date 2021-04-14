/*
 * File:	generator.cpp
 *
 * Description:	This file contains the public and member function
 *		definitions for the code generator for Simple C.
 *
 *		Extra functionality:
 *		- putting all the global declarations at the end
 */

# include <cassert>
# include <iostream>
# include <unordered_map>
# include "generator.h"
# include "machine.h"
# include "Tree.h"
# include "Label.h"

using namespace std;

static int offset;
static string funcname;
static unordered_map<string, Label*> strings;
static ostream &operator <<(ostream &ostr, Expression *expr);

static Register *eax = new Register("%eax", "%al");
static Register *ecx = new Register("%ecx", "%cl");
static Register *edx = new Register("%edx", "%dl");

static vector<Register *> registers = {eax, ecx, edx};


void assign(Expression *expr, Register *reg) {
   if (expr != nullptr) {
       if (expr->_register != nullptr){
           expr->_register->_node = nullptr;
       }
       expr->_register = reg;
   }
   if (reg != nullptr) {
       if (reg->_node != nullptr) {
           reg->_node->_register = nullptr;
       }
       reg->_node = expr;
   }
}

void load(Expression *expr, Register *reg) {
    cerr << "load" << endl;
    if (reg->_node != expr) {
        if (reg->_node != nullptr) {
            offset -= reg->_node->type().size();
            cerr << "offset: " << offset;
            reg->_node->_offset = offset;
            cout << "\tmovl\t" << reg << ", ";
            cout << offset << "(%ebp)" << endl;
        }

    if (expr != nullptr) {
        cout << (expr->type().size() == 1?
            "\tmovsbl\t" : "\tmovl\t");
        cout << expr << ", " << reg << endl;
    }
        assign(expr, reg);
    }

}



Register *getreg() {
   for (auto reg: registers)
       if (reg->_node == nullptr)
           return reg;

   load(nullptr, registers[0]);
   return registers[0];
}
/*
 * Function:	align (private)
 *
 * Description:	Return the number of bytes necessary to align the given
 *		offset on the stack.
 */

static int align(int offset)
{
    if (offset % STACK_ALIGNMENT == 0)
	return 0;

    return STACK_ALIGNMENT - (abs(offset) % STACK_ALIGNMENT);
}


/*
 * Function:	operator << (private)
 *
 * Description:	Convenience function for writing the operand of an
 *		expression using the output stream operator.
 */

static ostream &operator <<(ostream &ostr, Expression *expr)
{
    if (expr->_register != nullptr)
	return ostr << expr->_register;

    expr->operand(ostr);
    return ostr;
}


/*
 * Function:	Expression::operand
 *
 * Description:	Write an expression as an operand to the specified stream.
 */

void Expression::operand(ostream &ostr) const
{
    assert(_offset != 0);
    ostr << _offset << "(%ebp)";
}


/*
 * Function:	Identifier::operand
 *
 * Description:	Write an identifier as an operand to the specified stream.
 */

void Identifier::operand(ostream &ostr) const
{
    if (_symbol->_offset == 0)
	ostr << global_prefix << _symbol->name();
    else
	ostr << _symbol->_offset << "(%ebp)";
}


/*
 * Function:	Number::operand
 *
 * Description:	Write a number as an operand to the specified stream.
 */

void Number::operand(ostream &ostr) const
{
    ostr << "$" << _value;
}

void String::operand(ostream &ostr) const
{
    if (strings.count(_value) <= 0) {
        strings[_value] = new Label();
    }
    ostr << *(strings[_value]);
}

/*
 * Function:	Call::generate
 *
 * Description:	Generate code for a function call expression.
 *
 * 		On a 32-bit Linux platform, the stack needs to be aligned
 * 		on a 4-byte boundary.  (Online documentation seems to
 * 		suggest that 16 bytes are required, but 4 bytes seems to
 * 		work and is much easier.)  Since all arguments are 4-bytes
 *		wide, the stack will always be aligned.
 *
 *		On a 32-bit OS X platform, the stack needs to aligned on a
 *		16-byte boundary.  So, if the stack will not be aligned
 *		after pushing the arguments, we first adjust the stack
 *		pointer.  However, this trick only works if none of the
 *		arguments are themselves function calls.
 *
 *		To handle nested function calls, we need to generate code
 *		for the nested calls first, which requires us to save their
 *		results and then push them on the stack later.  For
 *		efficiency, we only first generate code for the nested
 *		calls, but generate code for ordinary arguments in place.
 */

void Call::generate()
{
    cerr << "Call::generate" << endl;
    unsigned numBytes;


    /* Generate code for any nested function calls first. */

    numBytes = 0;

    for (int i = _args.size() - 1; i >= 0; i --) {
	numBytes += _args[i]->type().size();

	if (STACK_ALIGNMENT != SIZEOF_ARG && _args[i]->_hasCall)
	    _args[i]->generate();
    }


    /* Align the stack if necessary. */

    if (align(numBytes) != 0) {
	cout << "\tsubl\t$" << align(numBytes) << ", %esp" << endl;
	numBytes += align(numBytes);
    }


    /* Generate code for any remaining arguments and push them on the stack. */

    for (int i = _args.size() - 1; i >= 0; i --) {
	if (STACK_ALIGNMENT == SIZEOF_ARG || !_args[i]->_hasCall)
	    _args[i]->generate();

	cout << "\tpushl\t" << _args[i] << endl;
	assign(_args[i], nullptr);

    }


    /* Call the function and then reclaim the stack space. */

    load(nullptr, eax);
    load(nullptr, ecx);
    load(nullptr, edx);

    cout << "\tcall\t" << global_prefix << _id->name() << endl;

    if (numBytes > 0)
	cout << "\taddl\t$" << numBytes << ", %esp" << endl;

    assign(this, eax);
    cerr << "Call::generate done" << endl;

}


/*
 * Function:	Block::generate
 *
 * Description:	Generate code for this block, which simply means we
 *		generate code for each statement within the block.
 */

void Block::generate()
{
    cerr << "Block::generate" << endl;
    for (auto stmt : _stmts) {
	stmt->generate();

	for (auto reg : registers)
	    assert(reg->_node == nullptr);
    }
    cerr << "Block::generate done" << endl;

}


/*
 * Function:	Simple::generate
 *
 * Description:	Generate code for a simple (expression) statement, which
 *		means simply generating code for the expression.
 */

void Simple::generate()
{
    cerr << "Simple::generate" << endl;
    _expr->generate();
    assign(_expr, nullptr);
    cerr << "Simple::generate done" << endl;

}


/*
 * Function:	Function::generate
 *
 * Description:	Generate code for this function, which entails allocating
 *		space for local variables, then emitting our prologue, the
 *		body of the function, and the epilogue.
 */

void Function::generate()
{
    cerr << "Function::generate" << endl;
    int param_offset;


    /* Assign offsets to the parameters and local variables. */

    param_offset = 2 * SIZEOF_REG;
    offset = param_offset;
    allocate(offset);


    /* Generate our prologue. */

    funcname = _id->name();
    cout << global_prefix << funcname << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << funcname << ".size, %esp" << endl;


    /* Generate the body of this function. */

    _body->generate();


    /* Generate our epilogue. */

    cout << endl << global_prefix << funcname << ".exit:" << endl;
    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl << endl;

    offset -= align(offset - param_offset);
    cout << "\t.set\t" << funcname << ".size, " << -offset << endl;
    cout << "\t.globl\t" << global_prefix << funcname << endl << endl;
    cerr << "Function::generate done" << endl;

}


/*
 * Function:	generateGlobals
 *
 * Description:	Generate code for any global variable declarations.
 */

void generateGlobals(Scope *scope)
{
    const Symbols &symbols = scope->symbols();

    for (auto symbol : symbols)
	if (!symbol->type().isFunction()) {
	    cout << "\t.comm\t" << global_prefix << symbol->name() << ", ";
	    cout << symbol->type().size() << endl;
	}
    if(!strings.empty()) {
        cout << "\t.data" << endl;
        for (auto it = strings.begin(); it != strings.end(); ++it) {
            cout << *(it->second) << ":\t.asciz\t\"" << it->first << "\"" << endl;
        }
    }
}


/*
 * Function:	Assignment::generate
 *
 * Description:	Generate code for an assignment statement.
 *
 *		NOT FINISHED: Only works if the right-hand side is an
 *		integer literal and the left-hand side is an integer
 *		scalar.
 */

void Assignment::generate()
{
    cerr << "Assignment::generate" << endl;

    Expression *pointer;

    _right->generate();

    if (_left->isDereference(pointer)) {

        pointer->generate();
        if (pointer->_register == nullptr) {
            load(pointer, getreg());
        }

        if (_right->_register == nullptr) {
            load(_right, getreg());
        }

        if (_left->type().size() == 4) {
            cout << "\tmovl\t" << _right << ", (" << pointer << ")" << endl;
        } else if (_left->type().size() == 1) {
            cout << "\tmovb\t" << _right->_register->byte() << ", (" << pointer << ")" << endl;
        }
        assign(pointer, nullptr);


    } else {
        if (_right->_register == nullptr) {
            load(_right, getreg());
        }
        if (_left->type().size() == 4) {
            cout << "\tmovl\t" << _right << ", " << _left << endl;
        } else if (_left->type().size() == 1) {
            cout << "\tmovb\t" << _right->_register->byte() << ", " << _left << endl;
        }
    }

    assign(_right, nullptr);

    cerr << "Assignment::generate done" << endl;

}

static void compute(Expression *result, Expression *left, Expression *right, const string &opcode){
    left->generate();
    right->generate();

    if (left->_register == nullptr) {
        load(left, getreg());
    }
    cout << "\t" << opcode << "\t" << right << ", " << left << endl;

    assign(right, nullptr);
    assign(result, left->_register);
}

void Add::generate() {
    cerr << "Add::generate" << endl;
    compute(this, _left, _right, "addl");
    cerr << "Add::generate done" << endl;

}

void Subtract::generate() {
    cerr << "Subtract::generate" << endl;
    compute(this, _left, _right, "subl");
    cerr << "Subtract::generate done" << endl;

}

void Multiply::generate() {
    cerr << "Multiply::generate" << endl;
    compute(this, _left, _right, "imull");
    cerr << "Multiply::generate done" << endl;

}

void Cast::generate() {
    cerr << "Cast::generate" << endl;
    _expr->generate();
    if (_expr->_register == nullptr) {
        load(_expr, getreg());
    }
    assign(this, _expr->_register);
    cerr << "Cast::generate done" << endl;

}

void Divide::generate() {
    cerr << "Divide::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, eax);

    load(nullptr, edx);
    cout << "\tmovl\t%eax, %edx" << endl;

    cout << "\tsarl\t$31, %edx" << endl;
    load(_right, ecx);
    cout << "\tidivl\t" << _right << endl;
    load(nullptr, ecx);

    assign(this, eax);
    cerr << "Divide::generate done" << endl;

}

void Remainder::generate() {
    cerr << "Remainder::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, eax);

    load(nullptr, edx);
    cout << "\tmovl\t%eax, %edx" << endl;

    cout << "\tsarl\t$31, %edx" << endl;
    load(_right, ecx);
    cout << "\tidivl\t" << _right << endl;
    load(nullptr, ecx);
    load(nullptr, eax);

    assign(this, edx);
    cerr << "Remainder::generate done" << endl;

}

void Equal::generate() {
    cerr << "Equal::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsete\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "Equal::generate done" << endl;

}

void NotEqual::generate() {
    cerr << "NotEqual::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsetne\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "NotEqual::generate done" << endl;

}

void LessOrEqual::generate() {
    cerr << "LessOrEqual::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsetle\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "LessOrEqual::generate done" << endl;

}

void GreaterOrEqual::generate() {
    cerr << "GreaterOrEqual::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsetge\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "GreaterOrEqual::generate done" << endl;

}

void LessThan::generate() {
    cerr << "LessThan::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsetl\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "LessThan::generate done" << endl;

}

void GreaterThan::generate() {
    cerr << "GreaterThan::generate" << endl;
    _left->generate();
    _right->generate();

    load(_left, getreg());
    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << "\tsetg\t" << _left->_register->byte() << endl;
    cout << "\tmovzbl\t" << _left->_register->byte() << ", " << _left->_register << endl;

    assign(this, _left->_register);
    cerr << "GreaterThan::generate done" << endl;

}

void Negate::generate() {
    cerr << "Negate::generate" << endl;
    _expr->generate();
    load(_expr, getreg());
    cout << "\tnegl\t" << _expr << endl;
    assign(this, _expr->_register);
    cerr << "Negate::generate done" << endl;

}

void Not::generate() {
    cerr << "Not::generate" << endl;
    _expr->generate();
    load(_expr, getreg());
    cout << "\tcmpl\t$0, " << _expr << endl;
    cout << "\tsete\t" << _expr->_register->byte() << endl;
    cout << "\tmovzbl\t" << _expr->_register->byte() << ", " << _expr << endl;
    assign(this, _expr->_register);
    cerr << "Not::generate done" << endl;

}

void Address::generate() {
    cerr << "Address::generate" << endl;
    Expression *pointer;
    if (_expr->isDereference(pointer)) {
        pointer->generate();

        if (pointer->_register == nullptr) {
            load(pointer, getreg());
        }
        assign(this, pointer->_register);
    } else {
        assign(this, getreg());
        cout << "\tleal\t" << _expr << ", " << this << endl;
    }
    cerr << "Address::generate done" << endl;

}

void Dereference::generate() {
    cerr << "Dereference::generate" << endl;

    _expr->generate();

    if(_expr->_register == nullptr) {
        load(_expr, getreg());
    }

    if (_expr->type().deref().size() == 4) {
        cout << "\tmovl\t(" << _expr << "), " << _expr << endl;
    } else if (_expr->type().deref().size() == 1) {
        cout << "\tmovsbl\t(" << _expr << "), " << _expr << endl;
    }
    assign(this, _expr->_register);
    cerr << "Dereference::generate done" << endl;

}

void Return::generate() {
    cerr << "Retrun::generate" << endl;
    _expr->generate();
    if (_expr->_register != eax) {
        load(_expr, eax);
    }
    cout << "\tjmp\t" << funcname << ".exit" << endl;
    assign(_expr, nullptr);
    cerr << "Retrun::generate done" << endl;

}

void Expression::test(const Label &label, bool ifTrue) {
    cerr << "Expression::test" << endl;
    generate();

    if (_register == nullptr) {
        load(this, getreg());
    }

    cout << "\tcmpl\t$0, " << this << endl;
    cout << (ifTrue ? "\tjne\t" : "\tje\t") << label << endl;

    assign(this, nullptr);
    cerr << "Expression::test done" << endl;

}


void LogicalOr::generate() {
    cerr << "LogicalOr::generate" << endl;
    Label truelabel, exitlabel;

    _left->test(truelabel, true);

    _right->test(truelabel, true);
    cout << "\tjmp\t" << exitlabel << endl;
    assign(this, getreg());

    cout << truelabel << ":" << endl;
    cout << "\tmovl\t$1, " << this << endl;
    cout << exitlabel << ":" << endl;
    cerr << "LogicalOr::generate done" << endl;

}

void LogicalAnd::generate() {
    cerr << "LogicalAnd::generate" << endl;
    Label falselabel, exitlabel;

    _left->test(falselabel, false);

    _right->test(falselabel, false);
    cout << "\tjmp\t" << exitlabel << endl;
    assign(this, getreg());

    cout << falselabel << ":" << endl;
    cout << "\tmovl\t$0, " << this << endl;
    cout << exitlabel << ":" << endl;
    cerr << "LogicalAnd::generate done" << endl;

}

void While::generate() {
    cerr << "While::generate" << endl;

    Label looplabel, exitlabel;

    cout << looplabel << ":" << endl;

    _expr->test(exitlabel, false);
    _stmt->generate();

    cout << "\tjmp\t" << looplabel << endl;
    cout << exitlabel << ":" << endl;
    cerr << "While::generate done" << endl;

}

void If::generate() {
    cerr << "If::generate" << endl;

    Label skiplabel, exitlabel;

    _expr->generate();
    cout << "\tcmp\t$0, " << _expr << endl;
    cout << "\tje\t" << skiplabel << endl;
    assign(_expr, nullptr);

    _thenStmt->generate();

    if (_elseStmt == nullptr) {
        cout << skiplabel << ":" << endl;
    } else {
        cout << "\tjmp\t" << exitlabel << endl;
        cout << skiplabel << ":" << endl;
        _elseStmt->generate();
        cout << exitlabel << ":" << endl;
    }
    cerr << "If::generate done" << endl;

}

void For::generate() {
    cerr << "For::generate" << endl;
    Label looplabel, exitlabel;

    _init->generate();

    cout << looplabel << ":" << endl;

    _expr->test(exitlabel, false);
    _stmt->generate();
    _incr->generate();
    cout << "\tjmp\t" << looplabel << endl;
    cout << exitlabel << ":" << endl;
    cerr << "For::generate done" << endl;

}
