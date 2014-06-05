#include <pypa/parser/parser.hh>
#include <stdio.h>

namespace pypa {
    void dump(AstPtr);
}

int main() {
    pypa::AstModulePtr ast;
    pypa::Parser parser;
    pypa::Lexer lexer("test.py");
    if(parser.parse(lexer, ast)) {
        printf("Parsing successfull %d\n", int(ast->body->items.front()->type));
        dump(ast);
    }
    else {
        printf("Parsing failed\n");
    }
    return 0;
}
