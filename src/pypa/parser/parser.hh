#ifndef GUARD_PYPA_PARSER_PARSER_HH_INCLUDED
#define GUARD_PYPA_PARSER_PARSER_HH_INCLUDED

#include <pypa/lexer/lexer.hh>
#include <pypa/ast/ast.hh>

namespace pypa {

class Parser {
public:
    bool parse(Lexer & lexer, AstModulePtr & ast);
};

}

#endif // GUARD_PYPA_PARSER_PARSER_HH_INCLUDED

