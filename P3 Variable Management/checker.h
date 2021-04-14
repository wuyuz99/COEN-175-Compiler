# ifndef CHECKER_H
# define CHECKER_H
# include <iostream>
# include <string>
# include "type.h"

void openScope();
void closeScope();
void declareVariable(std::string name, Type type);
void declareFunction(std::string name, Type type);
void defineFunction(std::string name, Type type);
void checkID(std::string name);

# endif /*CHECKER_H*/
