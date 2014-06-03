#include <pypa/parser/parser.hh>
#include <stdio.h>

int main() {
    pypa::AstPtr ast;
    pypa::Parser parser;
    pypa::Lexer lexer("test.py");
    if(parser.parse(lexer, ast)) {
        printf("Parsing successfull\n");
    }
    else {
        printf("Parsing failed\n");
    }
    return 0;
}
