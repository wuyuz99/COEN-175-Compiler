# include "generator.h"
# include "Type.h"
# include "Tree.h"
# include <iostream>

using namespace std;

static ostream &operator <<(ostream &ostr, const Expression *expr)
{
    expr->operand(ostr);
    return ostr;
}


void prologue(int n) {
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << n << ", %esp" << endl;
}

void epilogue() {
    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl;
}

void Function::generate() {
    cout << ".globl " << _id->name() << endl;
    cout << _id->name() << ":" << endl;
    Parameters * params = _id->type().parameters();
    unsigned paramLen = 0;
    if (params != nullptr) {
        paramLen = params->size();
    }
    Symbols symbols = _body->declarations()->symbols();
    unsigned i = 0;
    int offset = 8;

    for (; i < paramLen; ++i) {
        symbols[i]->_offset = offset;
        offset += symbols[i]->type().size();
        cerr << symbols[i]->name() << "\t" << symbols[i]->_offset << endl;
    }

    offset = 0;
    for (; i < symbols.size(); ++i) {
        offset -= symbols[i]->type().size();
        symbols[i]->_offset = offset;
        cerr << symbols[i]->name() << "\t" << symbols[i]->_offset << endl;
    }

    prologue(-offset);
    _body->generate();
    epilogue();
}

void generateGlobals(Scope *scope) {
    Symbols symbols = scope->symbols();
    for (unsigned i = 0; i < symbols.size(); ++i) {
        if (!symbols[i]->type().isFunction()) {
            cout << ".comm\t" << symbols[i]->name() << ", " << symbols[i]->type().size() << endl;
        }
    }
}

void Block::generate() {
    cerr << "Block" << endl;
    for (unsigned i = 0; i < _stmts.size(); ++i) {
        _stmts[i]->generate();
    }
}

void Simple::generate() {
    cerr << "Simple" << endl;
    _expr->generate();
}

void Call::generate() {
    cerr << "Call" << endl;
    unsigned argSize = 0;
    cerr << "args size: " << _args.size() << endl;
    for (int i = _args.size() - 1; i >= 0 ; --i) {
        cout << "\tpushl\t" << _args[i] << endl;
        argSize += _args[i]->type().size();
    }

    cout << "\tcall\t" << _id->name() << endl;
    cout << "\tsubl\t" << "$" << argSize << ", %esp" << endl;
}

void Assignment::generate() {
    cerr << "Assignment" << endl;
    cout << "\tmovl\t" << _right << ", " << _left << endl;
    return;
}

void Number::operand(ostream &ostr) const {
    ostr << "$" << _value;
}

void Identifier::operand(ostream &ostr) const {
    if (_symbol->_offset == 0) {
        ostr << _symbol->name();
    } else {
        ostr << _symbol->_offset << "(%ebp)";
    }
}
