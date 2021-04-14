# include <iostream>
# include "tokens.h"
# include "type.h"
# include "scope.h"
# include "symbol.h"
# include "scope.h"
# include "lexer.h"

using namespace std;

Scope* global_scope = nullptr;
Scope* current = nullptr;

static string redefined = "redefinition of '%s'";
static string conflicted = "conflicting types for '%s'";
static string redeclared = "redeclaration of '%s'";
static string undeclared = "'%s' undeclared";
static string voidtype = "'%s' has type void";

string revEnum(int token) {
    if (token == INT) {
        return "INT";
    } else if (token == CHAR) {
        return "CHAR";
    }
    return "VOID";
}

void openScope() {
    if (current == nullptr) {
        current = new Scope();
        cout << "opening global scope" << endl;
        return;
    }
    Scope* tmp = new Scope(current);
    current = tmp;
    cout << "opening scope" << endl;
}

void closeScope() {
    if (current->enclosing() == nullptr) {
        delete current;
        current = nullptr;
        cout << "closing global scope" << endl;
        return;
    }
    Scope *tmp = current;
    current = tmp->enclosing();
    delete tmp;
    cout << "closing scope" << endl;
}
void declareVariable(string name, Type type) {
    Symbol * symbol = current->find(name);
    if (symbol == nullptr) {
        if ((!type.isError()) && type.indirection() == 0 && type.specifier() == VOID) {
            report(voidtype, name);
        } else {
            current->insert(new Symbol(name, type));
            cout << "declare " << name << " as ";
            if (type.isArray())
                cout << "(" << revEnum(type.specifier()) << ", " << type.indirection() << ", " << "ARRAY)" << endl;
            if (type.isScalar())
                cout << "(" << revEnum(type.specifier()) << ", " << type.indirection() << ", " << "SCALAR)" << endl;
        }
    } else if (current->enclosing() != nullptr) {
        report(redeclared, name);
    } else if (symbol->type() != type) {
        report(conflicted, name);
    }
}
void declareFunction(string name, Type type) {
    Symbol * symbol = current->find(name);
    if (symbol == nullptr) {
        current->insert(new Symbol(name,type));
        cout << "declare " << name << " as ";
        cout << "(" << revEnum(type.specifier()) << ", " << type.indirection() << ", " << "FUNCTION)" << endl;
    } else if (current->enclosing() != nullptr) {
        report(redeclared, name);
    } else if (symbol->type() != type) {
        report(conflicted, name);
    }
}
void defineFunction(string name, Type type) {
    Symbol *symbol = current->find(name);
    if (symbol == nullptr) {
        current->insert(new Symbol(name,type));
        cout << "define " << name << " as ";
        cout << "(" << revEnum(type.specifier()) << ", " << type.indirection() << ", " << "FUNCTION)" << endl;
    } else if (symbol->type().parameters() == nullptr) {
        current->remove(name);  //potential mem leak
        current->insert(new Symbol(name,type));
        cout << "define declared " << name << " as ";
        cout << "(" << revEnum(type.specifier()) << ", " << type.indirection() << ", " << "FUNCTION)" << endl;
    } else {
        report(redefined, name);
    }
}
void checkID(string name) {
    if(current->lookup(name) == nullptr){
        report(undeclared, name);
    } else {
        cout << "check " << name << endl;
    }
}
