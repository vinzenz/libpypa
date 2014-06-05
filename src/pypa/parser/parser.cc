#include <pypa/parser/parser.hh>
#include <string>
#include <unordered_set>
#include <deque>
#include <stack>

extern "C" double strtod(const char *s00, char **se);

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
            s.savepoints.pop();
        }
    }

    struct StateGuard {
        StateGuard(State & s) : s_(&s) { save(s); }
        ~StateGuard() { if(s_) revert(*s_); }
        bool commit() { s_ = 0; return true; }
    private:
        State * s_;
    };

    void commit(State & s) {
        s.popped = {};
    }

    TokenInfo const & top(State & s) {
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

    void location(State & s, AstPtr a) {
        a->line = top(s).line;
        a->column = top(s).column;
    }

    void location(State & s, Ast & a) {
        a.line = top(s).line;
        a.column = top(s).column;
    }

    bool is(TokenInfo const & info, Token tok) {
        return token(info) == tok;
    }

    bool is(TokenInfo const & info, TokenKind k) {
        return kind(info) == k;
    }

    bool is(TokenInfo const & info, TokenClass c) {
        return cls(info) == c;
    }

    bool is(State & s, Token tok) {
        return is(top(s), tok);
    }

    bool is(State & s, TokenKind k) {
        return is(top(s), k);
    }

    bool is(State & s, TokenClass c) {
        return is(top(s), c);
    }

    template< typename T >
    bool expect(State & s, T t) {
        if(is(top(s), t)) {
            pop(s);
            return true;
        }
        return false;
    }

    template< typename T >
    bool consume_value(State & s, T t, String & v) {
        if(is(s, t)) {
            v = top(s).value;
            pop(s);
            return true;
        }
        return false;
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
bool statement(State & s, AstStmt & ast);
bool expression(State & s, AstExpr & ast);

bool expression(State & s, AstExprList & lst) {
    for(;;) {
        AstExpr one;
        if(!expression(s, one)) {
            break;
        }
        lst.push_back(one);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !lst.empty();
}

bool statement(State & s, AstStmtList & lst) {
    for(;;) {
        expect(s, TokenKind::NewLine) || expect(s, TokenKind::SemiColon);
        if(!push_apply<AstStmt>(s, lst, statement)) {
            break;
        }
    }
    return !lst.empty();
}

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
    // TODO: Implement properly
    return expression(s, ast);
}

bool raise(State & s, AstRaisePtr & ast) {
    return false;
}

bool _return(State & s, AstReturnPtr & ast) {
    return false;
}

bool yield(State & s, AstYieldPtr & ast) {
    StateGuard guard(s);
    if(expect(s, Token::KeywordYield)) {
        ast = std::shared_ptr<AstYield>();
        if(expression(s, ast->args)) {
            return guard.commit();
        }
        ast.reset();
    }
    return false;
}

bool global(State & s, AstGlobalPtr & ast) {
    return false;
}

bool as(State & s, AstExpr & ast) {
    if(expect(s, Token::KeywordAs)) {
        return expression(s, ast);
    }
    return false;
}

bool with_item(State & s, AstWithItemPtr & ast) {
    ast = std::make_shared<AstWithItem>();
    if(test(s, ast->context)) {
        as(s, ast->optional);
        return true;
    }
    ast.reset();
    return false;
}

bool suite(State & s, AstStmtList & lst) {
    StateGuard guard(s);
    expect(s, Token::Indent);
    if(statement(s, lst)) {
        expect(s, Token::Dedent);
        return guard.commit();
    }
    return false;
}

bool with(State & s, AstWithPtr & ast) {
    StateGuard guard(s);
    if(expect(s, Token::KeywordWith)) {
        ast = std::make_shared<AstWith>();
        AstWithItemPtr item;
        while(with_item(s, item)) {
            ast->items.push_back(item);
            if(!expect(s, TokenKind::Comma)) {
                break;
            }
        }
        if(expect(s, TokenKind::Colon)){
            expect(s, Token::NewLine);
            if(suite(s, ast->body)) {
                return guard.commit();
            }
        }
        ast.reset();
    }
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
        return apply<AstReturnPtr>(s, ast, _return);
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

bool number(State & s, AstNumberPtr & ast) {
    if(is(s, Token::NumberFloat)) {
        String const & dstr = top(s).value;
        char * e = 0;
        double result = strtod(dstr.c_str(), &e);
        if(!e || *e) {
            return false;
        }
        ast = std::make_shared<AstNumber>();
        ast->num_type = AstNumber::Float;
        ast->floating = result;
        pop(s);
        return true;
    }
    return false;
}

bool expression(State & s, AstExpr & ast) {
    if(apply<AstNumberPtr>(s, ast, number)) return true;
    return false;
}

bool compare(State & s, AstComparePtr & ast) {
    StateGuard guard(s);
    ast = std::make_shared<AstCompare>();
    if(expression(s, ast->left)) {
        AstCompareOpType op;
        while(op_compare(s, op)) {
            AstExpr expr;
            if(!expression(s, expr)) {
                return false;
            }
            ast->comperators.push_back(expr);
            ast->ops.push_back(op);
        }
        if(!ast->comperators.empty()) {
            return guard.commit();
        }
    }
    return false;
}

bool param_def(State & s, AstNamePtr & param_name, AstExpr & param_default) {
    param_name = std::make_shared<AstName>();
    param_name->context = AstContext::Param;
    StateGuard guard(s);
    if(consume_value(s, TokenKind::Name, param_name->id)) {
        if(expect(s, TokenKind::Equal)) {
            if(expression(s, param_default)) {
                return guard.commit();
            }
        }
        else {
            return guard.commit();
        }
    }
    return false;
}

bool param_list(State & s, AstExprList & names, AstExprList & defaults) {
    StateGuard guard(s);
    for(;;) {
        AstExpr pdef;
        AstNamePtr pname;
        if(names.empty() || !names.empty() && expect(s, TokenKind::Comma)) {
            if(param_def(s, pname, pdef)) {
                names.push_back(pname);
                defaults.push_back(pdef);
                if(!is(s, TokenKind::Comma)) {
                    break;
                }
            }
            else if(!names.empty()) {
                unpop(s);
                break;
            }
        }
        else {
            break;
        }
    }
    return !names.empty() ? guard.commit() : false;
}

bool kwarg_param(State & s, String & name) {
    if(is(s, TokenKind::DoubleStar)) {
        StateGuard guard(s);
        pop(s);
        if(is(s, TokenKind::Name) && !is(s, TokenClass::Keyword)) {
            consume_value(s, TokenKind::Name, name);
            return guard.commit();
        }        
    }
    return false;
}

bool args_params(State & s, String & name) {
    if(expect(s, TokenKind::Star)) {
        StateGuard guard(s);
        if(is(s, TokenKind::Name) && !is(s, TokenClass::Keyword)) {
            consume_value(s, TokenKind::Name, name);
            return guard.commit();
        }
    }
    return false;
}

bool varargslist(State & s, AstArguments & args) {
    switch(kind(top(s))) {
    case TokenKind::Star:
        if(args_params(s, args.args)) {
            if(is(s, TokenKind::DoubleStar)) {
                return kwarg_param(s, args.kwargs);
            }
        }
        break;
    case TokenKind::DoubleStar:
        return kwarg_param(s, args.kwargs);
    case TokenKind::Name:
        if(param_list(s, args.arguments, args.defaults)) {
            if(expect(s, TokenKind::Comma)) {
                if(is(s, TokenKind::Star)) {
                    if(!args_params(s, args.args)) {
                        return false;
                    }
                } else {
                    unpop(s);
                }
            }
            if(expect(s, TokenKind::Comma)) {
                if(is(s, TokenKind::DoubleStar)) {
                    if(!kwarg_param(s, args.kwargs)) {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool parameters(State & s, AstArguments & args) {
    StateGuard guard(s);
    location(s, args);
    if(expect(s, TokenKind::LeftParen)) {
        switch(kind(top(s))) {
        case TokenKind::RightParen:
            pop(s);
            return guard.commit();
        case TokenKind::Star:
        case TokenKind::DoubleStar:
        case TokenKind::Name:
            if(!varargslist(s, args)) {
                break;
            }
            if(expect(s, TokenKind::RightParen)) {
                return guard.commit();
            }
        }
    }
    return false;
}

bool class_def(State & s, AstClassDefPtr & ast) {
    return false;
}

bool function_def(State & s, AstFunctionDefPtr & ast) {
    StateGuard guard(s);
    location(s, ast = std::make_shared<AstFunctionDef>());
    if(expect(s, Token::KeywordDef)) {
        if(consume_value(s, Token::Identifier, ast->name)) {
            if(parameters(s, ast->args)) {
                if(expect(s, TokenKind::Colon)) {
                    expect(s, Token::NewLine);
                    if(suite(s, ast->body)) {
                        return guard.commit();
                    }
                }
            }
        }
    }
    ast.reset();
    return false;
}

bool decorated(State & s, AstStmt & ast) {
    return false;
}

bool print(State & s, AstPrintPtr & ast) {
    StateGuard guard(s);
    String identifier;
    if(consume_value(s, Token::Identifier, identifier) && identifier == "print") {
        ast = std::make_shared<AstPrint>();
        if(expect(s, TokenKind::RightShift)) {
            if(!expression(s, ast->destination)) {
                return false;
            }
        }
        if(!expression(s, ast->values)) {
            return false;
        }
        return guard.commit();
    }
    return false;
}

bool statement_inner(State & s, AstStmt & ast) {
    switch(token(top(s))) {
    case Token::DelimAt:
        return apply<AstStmt>(s, ast, decorated);
    case Token::KeywordDef:
        // Function
        return apply<AstFunctionDefPtr>(s, ast, function_def);
    case Token::KeywordClass:
        return apply<AstClassDefPtr>(s, ast, class_def);
    case Token::KeywordPass:
        location(s, ast = std::make_shared<AstPass>());
        pop(s);
        return true;
    case Token::Identifier:
        return apply<AstPrintPtr>(s, ast, print);
    case Token::KeywordWith:
        return apply<AstWithPtr>(s, ast, with);
    }
    return false;
}

bool statement(State & s, AstStmt & ast) {
    StateGuard guard(s);
    if(statement_inner(s, ast)) {
        return guard.commit();
    }
    return false;
}

bool module(State & s, AstModulePtr & ast) {
    TokenKind t = kind(top(s));
    auto mod = std::make_shared<AstModule>();
    location(s, mod);
    if(statement(s, mod->body)) {
        ast = mod;        
        return true;
    }
    return false;
}

bool Parser::parse(Lexer & lexer, AstModulePtr & ast) {
    State state{&lexer, {}, {}, {}, lexer.next()};
    return module(state, ast);
}

}
