# include <string>
# include "symbol.h"
# include "type.h"

using namespace std;

Symbol::Symbol(const string &name, const Type &type)
    : _name(name), _type(type)
{
}

const string &Symbol::name() const{
    return _name;
}

const Type &Symbol::type() const{
    return _type;
}
