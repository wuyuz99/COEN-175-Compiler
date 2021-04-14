# include "tokens.h"
# include "type.h"
# include <iostream>
using namespace std;

Type::Type()
    :_kind(ERROR)
{
}

Type::Type(int specifier, unsigned indirection)
    : _specifier(specifier), _indirection(indirection), _kind(SCALAR)
{
}

Type::Type(int specifier, unsigned indirection, unsigned length)
    : _specifier(specifier), _indirection(indirection), _length(length), _kind(ARRAY)
{
}

Type::Type(int specifier, unsigned indirection, Parameters *parameters)
    : _specifier(specifier), _indirection(indirection), _parameters(parameters), _kind(FUNCTION)
{

}

bool Type::isArray() const
{
    return _kind == ARRAY;
}

bool Type::isError() const
{
    return _kind == ERROR;
}

bool Type::isFunction() const
{
    return _kind == FUNCTION;
}

bool Type::isScalar() const
{
    return _kind == SCALAR;
}

int Type::specifier() const
{
    return _specifier;
}

unsigned Type::indirection() const
{
    return _indirection;
}

unsigned Type::length() const{
    return _length;
}

Parameters *Type::parameters() const
{
    return _parameters;
}

bool Type::operator==(const Type &rhs) const
{
    if (_kind != rhs._kind)
        return false;

    if (_kind == ERROR)
        return true;

    if (_specifier != rhs._specifier)
        return false;

    if (_indirection != rhs._indirection)
        return false;

    if (_kind == SCALAR)
        return true;

    if (_kind == ARRAY)
        return _length == rhs._length;

    if (!_parameters || !rhs._parameters)
        return true;

    return *_parameters == *rhs._parameters;
}

bool Type::operator!=(const Type &rhs) const
{
    return !operator==(rhs);
}

std::ostream &operator<<(std::ostream &ostr, const Type &type)
{
    if (type.isError()) {
        ostr << "ERROR" << endl;
        return ostr;
    }
    if (type.isScalar()) {
        ostr << "SCALAR\t" << type.specifier() << '\t' << type.indirection();
        return ostr;
    }
    if (type.isArray()) {
        ostr << "ARRAY\t" << type.specifier() << '\t' << type.indirection() << '\t' << type.length();
        return ostr;
    }
    ostr << "FUNCTION\t" << type.specifier() << '\t' << type.indirection();
    return ostr;
}
