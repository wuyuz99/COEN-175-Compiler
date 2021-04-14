# include <iostream>
# include <vector>
# include <cctype>

using namespace std;

template <class T>
bool in (T content, vector<T> vec) {
	size_t i = 0;
	for(; i < vec.size(); ++i)
		if(vec[i] == content)
			break;
	return(!(i == vec.size()));
}
int main() {
	vector<string> keywords= {"auto", "break", "case", "char", "const",
								"continue", "default", "do", "double", "else",
								"enum", "extern", "float", "for", "goto", "if",
								"int", "long", "register", "return", "short",
								"signed", "sizeof", "static", "struct",
								"switch", "typedef", "union", "unsigned",
								"void", "volatile", "while"};

	vector<string> operators = {"=", "|", "||", "&&", "==", "!=", "<",
								">", "<=", ">=", "+", "-", "*", "/", "%",
								"&", "!", "++", "--", ".", "->", "(", ")",
								"[", "]", "{", "}", ";", ":", ","};
	string token;
	int c = cin.get();

	while (!cin.eof()) {
		token.clear();

		if (isspace(c))
			c = cin.get();

		else if (isdigit(c)) {
			token += c;
			while (isdigit(c = cin.get())) {
				token += c;
			}
			cin.putback(c);
			cout << "int:" << token << endl;
			c = cin.get();

		} else if (c == '\"') {
			token += c;
			char prev = '\0';
			while ((c = cin.get()) != '\"' || prev == '\\') {
				prev = c;
				token += c;
			}
			token += c;
			cout << "string:" << token << endl;
			c = cin.get();

		} else if (c == '/' && cin.peek() == '*') {
			token += c;
			token += cin.get();
			token += c;
				while ((c = cin.get()) != '*' || cin.peek() != '/') {
					token += c;
				}
			token += c;
			token += cin.get();
			//cout << "comment:" << token << endl;
			c = cin.get();

		} else if (isalpha(c) || c == '_') {
			token += c;
			while (isalnum(c = cin.get()) || c == '_') {
				token += c;
			}
			if (in(token, keywords)) {
				cout << "keyword:" << token << endl;
			} else {
				cout << "identifier:" << token << endl;
			}
		} else if (ispunct(c)) {
			token += c;
			token += cin.peek();
			if(in(token, operators)){
				c = cin.get();
				cout << "operator:" << token << endl;
			}else if(in(string(1, c), operators)){
				cout << "operator:" << char(c) << endl;
			}
			c = cin.get();
		} else {
			c = cin.get();
		}
	}
}
