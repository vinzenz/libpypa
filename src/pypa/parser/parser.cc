#include <pypa/parser/parser.hh>

namespace pypa {

struct ParserState {
    std::deque<TokenInfo> token_buffer;
    Lexer * lexer;
    AstPtr  root;
    AstPtr  ast_cur;
};

inline TokenInfo cur(ParserState const & state) {
    return state.token_buffer.back();
}

inline TokenInfo fetch(ParserState & state) {
    state.token_buffer.push_back(state.lexer->next());
    return cur(state);
}

inline TokenInfo next(ParserState & state) {
    if(state.token_buffer.empty()) {
        return fetch(state);
    }
    return state.token_buffer.back();
}

bool parse_parameters(ParserState & state) {
    switch(cur(state).ident.id()) {
    case Token::DelimParenOpen:
        fetch(state);
        if(parse_vararglist(state)) {
            
        }

    }
}

bool parse_expression(ParserState & state) {
    switch(cur(state).ident.id()) {
    }
    return true;
}

bool parse_statement(ParserState & state) {
    switch(cur(state).ident.id()) {
    }
    return true;
}

bool Parser::parse(Lexer & lexer, AstPtr ast) {
    ParserState state{{}, &lexer, ast, {}};
    while(fetch(state).ident.id() != Token::End) {
        if(cur(state).ident.id() != Token::NewLine) {
            if(!parse_statement(state)) {
                return false;
            }
        }
    }
    return cur(state).ident.id() == Token::End;
}

}
