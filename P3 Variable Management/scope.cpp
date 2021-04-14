# include <vector>
# include "type.h"
# include "scope.h"
using namespace std;

Scope::Scope(Scope *enclosing)
    : _enclosing(enclosing)
{
}

void Scope::insert(Symbol *symbol) {
    _symbols.push_back(symbol);
}

void Scope::remove(const string &name) {
    size_t i = 0;
    for(; i < _symbols.size(); ++i) {
        if(_symbols[i]->name() == name){
            break;
        }
    }
    if (i != _symbols.size()) {
        _symbols.erase(_symbols.begin() + i);
    }
    return;
}

Symbol *Scope::find(const string &name) const {
    for (size_t i = 0; i < _symbols.size(); ++i) {
        if(_symbols[i]->name() == name){
            return _symbols[i];
        }
    }
    return nullptr;
}

Symbol *Scope::lookup(const string &name) const {
    const Scope *ptr = this;
    while (ptr != nullptr) {
        Symbol *result = ptr->find(name);
        if(result) {
            return result;
        }
        ptr = ptr->enclosing();
    }
    return nullptr;
}

Scope *Scope::enclosing() const {
    return _enclosing;
}

const Symbols &Scope::symbols() const {
    return _symbols;
}
