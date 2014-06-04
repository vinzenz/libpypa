#include <pypa/parser/parser.hh>
#include <string>
#include <unordered_set>
#include <deque>
#include <stack>

namespace pypa {
namespace {
    typedef std::unordered_set<std::string> Symbols;
    struct State {
        Lexer *                 lexer;
        std::stack<TokenInfo>   tokens;
        std::stack<TokenInfo>   popped;
        std::stack<std::size_t> savepoints;
        TokenInfo               tok_cur;
    };

    TokenInfo pop(State & s) {
        s.popped.push(s.tok_cur);
        if(s.tokens.empty()) {
            s.tok_cur = s.lexer->next();
        }
        else {
            s.tok_cur = s.tokens.top();
            s.tokens.pop();
        }

        return s.tok_cur;
    }

    void unpop(State & s) {
        s.tokens.push(s.tok_cur);
        s.tok_cur = s.popped.top();
        s.popped.pop();
    }

    void save(State & s) {
        s.savepoints.push(s.popped.size());
    }

    void revert(State & s) {
        if(!s.savepoints.empty()) {
            while(s.popped.size() != s.savepoints.top()) {
                unpop(s);
            }
        }
    }

    void commit(State & s) {
        s.popped = {};
    }

    TokenInfo top(State & s) {
        return s.tok_cur;
    }

    TokenKind kind(TokenInfo const & tok) {
        return tok.ident.kind();
    }

    TokenClass cls(TokenInfo const & tok) {
        return tok.ident.cls();
    }

    Token token(TokenInfo const & tok) {
        return tok.ident.id();
    }
}

template< typename TargetT, typename BaseT>
bool apply(State & s, BaseT & b, bool(*f)(State &, TargetT&))
{
    TargetT t;
    if(f(s, t)) {
        b = t;
        return true;
    }
    return false;
}

template< typename TargetT, typename ContainerT>
bool push_apply(State & s, ContainerT & c, bool(*f)(State &, TargetT&)) {
    typename ContainerT::value_type v;
    if(apply<TargetT>(s, v, f)) {
        c.push_back(v);
        return true;
    }
    return false;
}

// Parser

bool bin_op(State & s, AstBinOpPtr & ast) {
    return false;
}

bool unary_op(State & s, AstUnaryOpPtr & ast) {
    return false;
}

bool bool_op(State & s, AstBinOpPtr & ast) {
    return false;
}

bool lambda(State & s, AstLambdaPtr & ast) {
    return false;
}

bool if_expr(State & s, AstIfExprPtr & ast) {
    return false;
}

bool dict(State & s, AstDictPtr & ast) {
    return false;
}

bool set(State & s, AstSetPtr & ast) {
    return false;
}

bool list(State & s, AstListPtr & ast) {
    return false;
}

bool tuple(State & s, AstTuplePtr & ast) {
    return false;
}

bool test_list(State & s, AstExpr & ast) {
    return false;
}

bool test(State & s, AstExpr & ast) {
    return false;
}

bool raise(State & s, AstRaisePtr & ast) {
    return false;
}

bool retrn(State & s, AstReturnPtr & ast) {
    return false;
}

bool yield(State & s, AstYieldPtr & ast) {
    return false;
}

bool global(State & s, AstGlobalPtr & ast) {
    return false;
}

bool flow_statement(State & s, AstStmt & ast) {
    if(cls(top(s)) != TokenClass::Keyword) {
        return false;
    }
    switch(token(top(s))) {
    case Token::KeywordContinue:
        pop(s);
        ast = std::make_shared<AstContinue>();
        return true;
    case Token::KeywordBreak:
        pop(s);
        ast = std::make_shared<AstContinue>();
        return true;
    case Token::KeywordRaise:
        return apply<AstRaisePtr>(s, ast, raise);
    case Token::KeywordReturn:
        return apply<AstReturnPtr>(s, ast, retrn);
    case Token::KeywordYield:
        return apply<AstYieldPtr>(s, ast, yield);
        break;
    }
    return false;
}

bool op_compare(State & s, AstCompareOpType & op) {
    switch(token(top(s))) {
    case Token::OpEqual:
        op = AstCompareOpType::Equals;
        return true;
    case Token::OpNotEqual:
        op = AstCompareOpType::NotEqual;
        return true;
    case Token::OpMoreEqual:
        op = AstCompareOpType::MoreEqual;
        return true;
    case Token::OpLessEqual:
        op = AstCompareOpType::LessEqual;
        return true;
    case Token::OpMore:
        op = AstCompareOpType::More;
        return true;
    case Token::OpLess:
        op = AstCompareOpType::Less;
        return true;
    case Token::KeywordIs:
        if(token(pop(s)) == Token::KeywordNot) {
            op = AstCompareOpType::IsNot;
            return true;
        }
        op = AstCompareOpType::Is;
        unpop(s);
        return true;
    case Token::KeywordIn:
        op = AstCompareOpType::In;
        return true;
    case Token::KeywordNot:
        if(token(pop(s)) == Token::KeywordIn) {
            op = AstCompareOpType::NotIn;
            return true;
        }
        unpop(s);
        break;
    }
    return false;
}

bool op_binary(State & s, AstBinOpType & op) {
    switch(token(top(s))) {
    case Token::OpAdd:
        op = AstBinOpType::Add;
        return true;
    case Token::OpSub:
        op = AstBinOpType::Sub;
        return true;
    case Token::OpMul:
        op = AstBinOpType::Mult;
        return true;
    case Token::OpDiv:
        op = AstBinOpType::Div;
        return true;
    case Token::OpExp:
        op = AstBinOpType::Power;
        return true;
    case Token::OpMod:
        op = AstBinOpType::Mod;
        return true;
    case Token::OpAnd:
        op = AstBinOpType::BitAnd;
        return true;
    case Token::OpOr:
        op = AstBinOpType::BitOr;
        return true;
    case Token::OpXor:
        op = AstBinOpType::BitXor;
        return true;
    case Token::OpDivDiv:
        op = AstBinOpType::FloorDiv;
        return true;
    case Token::OpShiftLeft:
        op = AstBinOpType::LeftShift;
        return true;
    case Token::OpShiftRight:
        op = AstBinOpType::RightShift;
        return true;
    }
    return false;
}

bool op_bool(State & s, AstBoolOpType & op) {
    switch(token(top(s))) {
    case Token::KeywordAnd:
        op = AstBoolOpType::And;
        return true;
    case Token::KeywordOr:
        op = AstBoolOpType::Or;
        return true;
    }
    return false;
}

bool op_unary(State & s, AstUnaryOpType & op) {
    switch(token(top(s))) {
    case Token::OpAdd:
        op = AstUnaryOpType::Add;
        return true;
    case Token::OpSub:
        op = AstUnaryOpType::Sub;
        return true;
    case Token::OpInv:
        op = AstUnaryOpType::Invert;
        return true;
    case Token::KeywordNot:
        op = AstUnaryOpType::Not;
        return true;
    }
    return false;
}

bool expression(State & s, AstExpr & ast) {
    return false;
}

bool param_def(State & s, AstNamePtr & param_name, AstExpr & param_default) {
    if(kind(top(s)) == TokenKind::Name) {
        save(s);
        param_name = std::make_shared<AstName>();
        param_name->context = AstContext::Param;
        param_name->id = top(s).value;
        pop(s);
        if(kind(top(s)) == TokenKind::Equal) {
            pop();
            if(expression(s, param_default)) {
                return true;
            }
        }
        else {
            return true;
        }
        revert(s);
    }
    return false;
}

bool param_list(State & s, AstExprList & names, AstExprList & defaults) {
    save(s);
    for(;;) {
        AstExpr pname, pdef;
        if(param_def(s, pname, pdef)) {
            names.push_back(pname);
            defaults.push_back(pdef);
            if(token(top(s)) != Token::DelimComma) {
                return true;
            }
            pop();
        }
    }
    revert(s);
    return false;
}

bool kwarg_param(State & s, String & name) {
    if(kind(top(s)) == TokenKind::DoubleStar) {
        save(s);
        pop();
        if(kind(top(s)) == TokenKind::Name) {
            if(cls(top(s)) != TokenClass::Keyword) {
                name = top(s)->value;
                pop();
                return true;
            }
        }
        revert(s);
    }
    return false;
}

bool args_params(State & s, String & name) {
    if(kind(top(s)) == TokenKind::Star) {
        save(s);
        pop();
        if(kind(top(s)) == TokenKind::Name) {
            if(cls(top(s)) != TokenClass::Keyword) {
                name = top(s)->value;
                pop();
                return true;
            }
        }
        revert(s);
    }
    return false;
}

bool varargslist(State & s, AstArguments & args) {

}

bool parameters(State & s, AstArguments & args) {
    if(kind(top(s)) == TokenKind::LeftParen) {
        switch(kind(pop(s))) {
        case TokenKind::RightParen:
            pop();
            return true;
        case TokenKind::Name:

        }
    }
    return false;
}

bool function_def(State & s, AstFunctionDefPtr & ast) {
    save(s);
    pop(s);
    ast = std::make_shared<AstFunctionDef>();
    if(token(top(s)) == Token::Identifier) {
        pop();
        if(parameters(s, ast->args) && kind(top(s)) == TokenKind::Colon) {
            ast->name = top(s).value;
            pop(s);

        }
    }

    revert(s);
    ast.reset();
    return false;
}

bool decorated(State & s, AstStmt & ast) {
    return false;
}

bool statement(State & s, AstStmt & ast) {
    switch(token(top(s))) {
    case Token::DelimAt:
        // Decorator
        return apply<AstStmt>(s, ast, decorated);
    case Token::KeywordDef:
        // Function
        return apply<AstFunctionDef>(s, ast, function_def);
    }
    return false;
}

bool module(State & s, AstModulePtr & ast) {
    TokenKind t = kind(top(s));
    auto mod = std::make_shared<AstModule>();
    while(t != TokenKind::End) {
        switch(t) {
        case TokenKind::NewLine:
        case TokenKind::SemiColon:
            t = kind(pop(s));
            break;
        default:
            return push_apply<AstStmt>(s, mod->body, statement);
        }
    }
    ast = mod;
    return true;
}

bool Parser::parse(Lexer & lexer, AstModulePtr & ast) {
    State state{&lexer, {}, {}, {}, lexer.next()};
    return module(state, ast);
}

}
