# ifndef TYPE_H
# define TYPE_H
#include <vector>
#include <ostream>

typedef std::vector<class Type> Parameters;

class Type {
    int _specifier;
    unsigned _indirection;
    unsigned _length;
    Parameters *_parameters;    //nullptr = (), [] = (void)

    enum { ARRAY, ERROR, FUNCTION, SCALAR } _kind;

public:
    Type();
    Type(int specifier, unsigned indirection);
    Type(int specifier, unsigned indirection, unsigned length);
    Type(int specifier, unsigned indirection, Parameters *parameters);

    bool isArray() const;
    bool isError() const;
    bool isFunction() const;
    bool isScalar() const;

    int specifier() const;
    unsigned indirection() const;
    unsigned length() const;
    Parameters *parameters() const;

    bool operator ==(const Type &rhs) const;
    bool operator !=(const Type &rhs) const;


};

std::ostream &operator<<(std::ostream &ostr, const Type &type);
# endif
