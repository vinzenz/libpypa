#include <pypa/parser/apply.hh>

extern "C" double strtod(const char *s00, char **se);

namespace pypa {



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
    StateGuard guard(s, ast);
    if(is(s, TokenKind::LeftBrace)) {
        location(s, create(ast));
        pop(s);
        for(;;) {
            AstExpr key, value;

            if(!expression(s, key))
                break;

            if(!expect(s, TokenKind::Colon))
                return false;

            if(!expression(s, value))
                return false;

            ast->keys.push_back(key);
            ast->values.push_back(value);

            if(!expect(s, TokenKind::Comma))
                break;
        }
        return expect(s, TokenKind::RightBrace) && guard.commit();
    }
    return false;
}

template< typename PtrT >
bool list_like(TokenKind open, TokenKind close, State & s, PtrT & ast ) {
    StateGuard guard(s, ast);
    if(is(s, open)) {
        location(s, create(ast));
        pop(s);
        AstExpr expr;
        while(expression(s, expr)) {
            ast->elements.push_back(expr);
            expr.reset();
            if(!expect(s, TokenKind::Comma))
                break;
        }
        return expect(s, close) && guard.commit();
    }
    return false;
}

bool set(State & s, AstSetPtr & ast) {
    return list_like(TokenKind::LeftBrace, TokenKind::RightBrace, s, ast);
}

bool dict_or_set(State & s, AstExpr & expr) {
    save(s);
    if(expect(s, TokenKind::LeftBrace)) {
        if(expect(s, TokenKind::RightBrace)) {
            revert(s);
            return apply<AstDictPtr>(s, expr, dict);
        }
        AstExpr expr;
        if(expression(s, expr)) {
            if(expect(s, TokenKind::Comma)) {
                revert(s);
                return apply<AstSetPtr>(s, expr, set);
            }
            if(expect(s, TokenKind::Colon)) {
                return apply<AstDictPtr>(s, expr, dict);
            }
        }
    }
    revert(s);
    return false;
}

bool list(State & s, AstListPtr & ast) {
    return list_like(TokenKind::LeftBracket, TokenKind::RightBracket, s, ast);
}

bool tuple(State & s, AstTuplePtr & ast) {
    return list_like(TokenKind::LeftParen, TokenKind::RightParen, s, ast);
}

bool str(State & s, AstStrPtr & ast) {
    if(is(s, Token::String)) {
        location(s, create(ast));
        return consume_value(s, Token::String, ast->value);
    }
    return false;
}

bool repr(State & s, AstReprPtr & ast) {
    if(expect(s, Token::BackQuote)) {
    }
}

bool number(State &s, AstNumberPtr &ast);

bool yield_expr(State &s, AstYieldExprPtr &ast);
bool atom(State & s, AstExpr & expr) {
    if(is(s, TokenKind::LeftParen))
    {
        StateGuard guard(s, expr);
        if(expect(s, TokenKind::LeftParen)) {
            if(apply<AstYieldExprPtr>(s, expr, yield_expr)) {
                return expect(s, TokenKind::RightParen)
                    && guard.commit();
            }
        }
    }
    return apply<AstTuplePtr>(s, expr, tuple)
        || apply<AstListPtr>(s, expr, list)
        || apply<AstDictPtr>(s, expr, dict)
        || apply<AstSetPtr>(s, expr, set)
        || apply<AstStrPtr>(s, expr, str)
        || apply<AstNumberPtr>(s, expr, number)
        || apply<AstReprPtr>(s, expr, repr)
        ;
}

bool compare(State &s, AstComparePtr &ast);
bool not_test(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    if(expect(s, Token::KeywordNot)) {
        auto op = create<AstBoolOp>(ast);
        location(s, op);
        AstExpr expr;
        if(!not_test(s, expr)) {
            return false;
        }
        op->values.push_back(expr);
        ast = op;
        return guard.commit();
    }
    return apply<AstComparePtr>(s, ast, compare) && guard.commit();
}

bool and_test(State & s, AstExpr & ast) {
    if(not_test(s, ast)) {
        StateGuard guard(s);
        if(expect(s, Token::KeywordAnd)) {
            AstBoolOpPtr op;
            location(s, create(op));
            AstExpr expr;
            if(not_test(s, expr)) {
                op->values.push_back(ast);
                op->values.push_back(expr);
                op->op = AstBoolOpType::And;
                ast = op;
                return guard.commit();
            }
        }
        return true;
    }
    return false;
}

bool or_test(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    if(and_test(s, ast)) {
        AstBoolOpPtr op;
        location(s, create(op));
        StateGuard guard2(s);
        if(expect(s, Token::KeywordOr)) {
            AstExpr right;
            if(!and_test(s, right)) {
                return false;
            }
            op->op = AstBoolOpType::Or;
            op->values.push_back(ast);
            op->values.push_back(right);
            ast = op;

            return guard2.commit()
                && guard.commit();
        }
        return guard.commit();
    }
    return false;
}

bool old_lambdadef(State & s, AstLambdaPtr & ast);
bool old_test(State & s, AstExpr & expr) {
    return or_test(s, expr) || apply<AstLambdaPtr>(s, expr, old_lambdadef);
}

bool varargslist(State &s, AstArguments &args);
bool old_lambdadef(State & s, AstLambdaPtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(expect(s, Token::KeywordLambda)) {
        if(varargslist(s, ast->arguments)) {
            if(old_test(s, ast->body)) {
                return guard.commit();
            }
        }
    }
    return false;
}

bool comp_if(State & s, AstIfExprPtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(expect(s, Token::KeywordIf)) {

    }
}

bool test(State & s, AstExpr & ast) {
    // TODO: Implement properly
    return expression(s, ast);
}


bool test_list1(State & s, AstExprList & ast) {
    AstExpr expr;
    while(test(s, expr) && expr) {
        ast.push_back(expr);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !ast.empty();
}

bool test_list(State & s, AstExprList & ast) {
    if(test_list1(s, ast)) {
        expect(s, TokenKind::Comma);
        return true;
    }
    return false;
}

bool raise(State & s, AstRaisePtr & ast) {    
    if(expect(s, Token::KeywordRaise)) {
        location(s, create(ast));
        test(s, ast->arg0);
        expect(s, TokenKind::Comma);
        test(s, ast->arg1);
        expect(s, TokenKind::Comma);
        test(s, ast->arg2);
        expect(s, TokenKind::NewLine) || expect(s, TokenKind::SemiColon);
        return true;
    }
    return false;
}

bool _return(State & s, AstReturnPtr & ast) {
    return false;
}

bool yield_expr(State & s, AstYieldExprPtr & ast) {
    StateGuard guard(s, ast);
    if(expect(s, Token::KeywordYield)) {
        if(expression(s, create(ast)->args)) {
            return guard.commit();
        }
    }
    return false;
}

bool yield(State &s, AstYieldPtr &ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    return yield_expr(s, ast->yield) && guard.commit();
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
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(test(s, ast->context)) {
        as(s, ast->optional);
        return guard.commit();
    }
    return false;
}

bool suite(State & s, AstSuitePtr & suite) {
    StateGuard guard(s);
    expect(s, Token::Indent);
    location(s, create(suite));
    if(statement(s, suite->items)) {
        expect(s, Token::Dedent);
        return guard.commit();
    }
    return false;
}

bool with(State & s, AstWithPtr & ast) {
    StateGuard guard(s, ast);
    if(expect(s, Token::KeywordWith)) {
        create(ast);
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
        location(s, create<AstContinue>(ast));
        return true;
    case Token::KeywordBreak:
        pop(s);
        location(s, create<AstBreak>(ast));
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

inline int64_t base_char_to_value(char c) {
    if(c >= '0' && c <= '9') {
        return int64_t(c - '0');
    }
    if(c >= 'A' && c <= 'F') {
        return int64_t(c - 'A') + 10;
    }
    return int64_t(c - 'a') + 10;
}

bool number_from_base(int64_t base, State & s, AstNumberPtr & ast) {
    String const & value = top(s).value;
    AstNumber & result = *ast;
    result.num_type = AstNumber::Integer;
    result.integer = 0;
    for(auto c : value) {
        result.integer *= base;
        int64_t tmp = base_char_to_value(c);
        result.integer += tmp;
    }
    return true;
}

bool number(State & s, AstNumberPtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    int base = 0;
    if(is(s, Token::NumberFloat)) {
        String const & dstr = top(s).value;
        char * e = 0;
        double result = strtod(dstr.c_str(), &e);
        if(!e || *e) {
            return false;
        }
        ast->num_type = AstNumber::Float;
        ast->floating = result;
        pop(s);
        return guard.commit();
    }
    else if(is(s, Token::NumberBinary)) {
        base = 2;
    }
    else if(is(s, Token::NumberOct)) {
        base = 8;
    }
    else if(is(s, Token::NumberInteger)) {
        base = 10;
    }
    else if(is(s, Token::NumberHex)) {
        base = 16;
    }
    if(base && number_from_base(base, s, ast)) {
        pop(s);
        return guard.commit();
    }
    return false;
}

bool expression(State & s, AstExpr & ast) {
    return atom(s, ast);
}

bool compare(State & s, AstComparePtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
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
    location(s, create(param_name));
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
    StateGuard guard(s, ast);
    location(s, create(ast));
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
    return false;
}

bool decorated(State & s, AstStmt & ast) {
    return false;
}

bool print(State & s, AstPrintPtr & ast) {
    StateGuard guard(s, ast);
    String identifier;
    if(consume_value(s, Token::Identifier, identifier) && identifier == "print") {
        create(ast);
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
        location(s, create<AstPass>(ast));
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
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(suite(s, ast->body)) {
        return guard.commit();
    }
    return false;
}

bool Parser::parse(Lexer & lexer, AstModulePtr & ast) {
    State state{&lexer, {}, {}, {}, lexer.next()};
    return module(state, ast);
}

}
