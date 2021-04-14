int NUM;
int NL, LPAREN, RPAREN, PLUS, MINUS, STAR, SLASH;
int lookahead, expr();
int c, lexval;

int printf(), exit(), getchar(), isspace(), isdigit();

int main(void)
{
    int n;
    NUM = 256;

    NL = *"\n";
    LPAREN = *"(";
    RPAREN = *")";
    PLUS = *"+";
    MINUS = *"-";
    STAR = *"*";
    SLASH = *"/";
    printf("NL: %d\n", NL);

    printf("LPAREN: %d\n", LPAREN);

    printf("RPAREN: %d\n", RPAREN);

    printf("PLUS: %d\n", PLUS);

    printf("MINUS: %d\n", MINUS);

    printf("RPAREN: %d\n", RPAREN);

    printf("STAR: %d\n", STAR);

    printf("SLASH: %d\n", SLASH);

}
