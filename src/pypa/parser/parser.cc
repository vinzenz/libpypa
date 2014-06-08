#include <pypa/parser/apply.hh>

extern "C" double strtod(const char *s00, char **se);

namespace pypa {



// Parser
bool statement(State & s, AstStmt & ast);
bool expression(State & s, AstExpr & ast);
bool expression_nocond(State &s, AstExpr &ast);

bool number(State &s, AstNumberPtr &ast);

bool param_list(State &s, AstExprList &names, AstExprList &defaults);
bool argument_list(State & s, AstCallPtr & ast);
bool varargslist(State &s, AstArguments &args);
bool old_lambdadef(State & s, AstLambdaPtr & ast);
bool yield_expr(State &s, AstYieldExprPtr &ast);

bool target(State & s, AstExpr & ast);
bool target_list(State & s, AstExpr & ast);

bool comparison(State &s, AstExpr &ast);

bool or_test(State & s, AstExpr & ast);
bool and_test(State & s, AstExpr & ast);
bool not_test(State & s, AstExpr & ast);

bool comp_iter(State &s, AstExpr &ast);
bool attributeref(State &s, AstExpr &expr);
bool subscription(State &s, AstExpr &expr);
bool primary(State &s, AstExpr &ast);



bool expression(State & s, AstExprList & lst, bool & last_token_comma) {
    for(;;) {
        AstExpr one;
        if(!expression(s, one)) {
            last_token_comma = !lst.empty();
            break;
        }
        lst.push_back(one);
        if(!expect(s, TokenKind::Comma)) {
            last_token_comma = false;
            break;
        }
    }
    return !lst.empty();
}

bool expression(State & s, AstExprList & lst) {
    bool ignored = false;
    return expression(s, lst, ignored);
}

bool comp_if(State &s, AstExpr &ast) {
    StateGuard guard(s, ast);
    AstIfExprPtr expr;
    location(s, create(expr));
    ast = expr;
    if(expect(s, Token::KeywordIf)) {
        if(expression_nocond(s, expr->test)) {
            comp_iter(s, expr->body);
            return guard.commit();
        }
    }
    return false;
}

bool comp_for(State &s, AstExpr &ast) {
    StateGuard guard(s, ast);
    AstForExprPtr expr;
    location(s, create(expr));
    ast = expr;
    if(expect(s, Token::KeywordFor)) {
        if(target_list(s, expr->items)) {
            if(expect(s, Token::KeywordIn)) {
                if(or_test(s, expr->generators)) {
                    comp_iter(s, expr->iter);
                    return guard.commit();
                }
            }
        }
    }
    return false;
}

bool comp_iter(State &s, AstExpr &ast) {
    return comp_for(s, ast)
        || comp_if(s, ast);
}

bool generator_expression(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstGeneratorPtr generator;
    location(s, create(generator));
    ast = generator;
    return expect(s, TokenKind::LeftParen)
        && expression(s, generator->expression)
        && comp_for(s, generator->for_expr)
        && expect(s, TokenKind::RightParen)
        && guard.commit();
}

bool statement(State & s, AstStmtList & lst) {
    for(;;) {
        while(expect(s, TokenKind::NewLine) || expect(s, TokenKind::SemiColon));
        if(end(s) || !push_apply<AstStmt>(s, lst, statement)) {
            break;
        }
    }
    return !lst.empty();
}

bool dict(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstDictPtr dict;
    location(s, create(dict));
    ast = dict;
    if(expect(s, TokenKind::LeftBrace)) {
        for(;;) {
            AstExpr key, value;

            if(!expression(s, key))
                break;

            if(!expect(s, TokenKind::Colon))
                return false;

            if(!expression(s, value))
                return false;

            dict->keys.push_back(key);
            dict->values.push_back(value);

            if(!expect(s, TokenKind::Comma))
                break;
        }
        return expect(s, TokenKind::RightBrace) && guard.commit();
    }
    return false;
}

template< typename PtrT >
bool list_like(TokenKind open, TokenKind close, State & s, PtrT & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(expect(s, open)) {
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

bool set(State & s, AstExpr & ast) {
    AstSetPtr expr;
    if(list_like(TokenKind::LeftBrace, TokenKind::RightBrace, s, expr)) {
        ast = expr;
        return true;
    }
    return false;
}

bool dict_or_set(State & s, AstExpr & expr) {
    save(s);
    if(expect(s, TokenKind::LeftBrace)) {
        if(expect(s, TokenKind::RightBrace)) {
            revert(s);
            return dict(s, expr);
        }
        AstExpr expr;
        if(expression(s, expr)) {
            if(expect(s, TokenKind::Comma)) {
                revert(s);
                return set(s, expr);
            }
            if(expect(s, TokenKind::Colon)) {
                revert(s);
                return dict(s, expr);
            }
        }
    }
    revert(s);
    return false;
}

bool list(State & s, AstExpr & ast) {
    AstListPtr expr;
    if(list_like(TokenKind::LeftBracket, TokenKind::RightBracket, s, expr)) {
        ast = expr;
        return true;
    }
    return false;
}

bool tuple(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstTuplePtr t;
    bool result = list_like(TokenKind::LeftParen, TokenKind::RightParen, s, t);
    ast = t;
    return (result && t->elements.size() > 1 && guard.commit());
}

bool str(State & s, AstStrPtr & ast) {
    if(is(s, Token::String)) {
        location(s, create(ast));
        return consume_value(s, Token::String, ast->value);
    }
    return false;
}

bool repr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstReprPtr repr;
    location(s, create(repr));
    ast = repr;
    if(expect(s, Token::BackQuote)) {
        if(expression(s, repr->value) && !repr->value.empty()) {
            if(expect(s, Token::BackQuote)) {
                return guard.commit();
            }
        }
    }
    return false;
}

bool id(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstNamePtr name;
    location(s, create(name));
    ast = name;
    name->id = top(s).value;
    if(expect(s, Token::Identifier)) {        
        return guard.commit();
    }
    return false;
}

bool literal(State & s, AstExpr & expr) {
    return apply<AstNumberPtr>(s, expr, number)
        || apply<AstStrPtr>(s, expr, str)
        ;
}

bool lambda_expr(State & s, AstExpr & ast, bool nocond) {
    StateGuard guard(s, ast);
    AstLambdaPtr lambda;
    location(s, create(lambda));
    ast = lambda;
    if(expect(s, Token::KeywordLambda)) {
        if(param_list(s, lambda->arguments.arguments, lambda->arguments.defaults)) {
            if(expect(s, TokenKind::Colon)) {
                if(nocond) {
                    return expression_nocond(s, lambda->body)
                        && guard.commit();
                }
                else {
                    return expression(s, lambda->body)
                        && guard.commit();
                }
            }
        }
    }
    return false;
}

bool lambda_expr(State &s, AstExpr &ast) {
    return lambda_expr(s, ast, false);
}

bool lambda_expr_nocond(State & s, AstExpr & ast) {
    return lambda_expr(s, ast, true);
}

bool expression_nocond(State & s, AstExpr & ast) {
    return  lambda_expr_nocond(s, ast)
         || or_test(s, ast);
}

bool list_comprehension(State & s, AstExpr & ast) {
    return false;
}

bool list_display(State & s, AstExpr & ast) {
    return list_comprehension(s, ast)
         || list(s, ast);
}

// || apply<AstDictPtr>(s, expr, dict)
// || apply<AstSetPtr>(s, expr, set)
// || apply<AstReprPtr>(s, expr, repr)


bool parenth_form(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    if(is(s, TokenKind::LeftParen)) {
        if(tuple(s, ast) || expression(s, ast)) {
            return guard.commit();
        }
    }
    return false;
}

bool enclosure(State & s, AstExpr & expr) {
    if(parenth_form(s, expr)) {
        return true;
    }
    if(list_display(s, expr)) {
        return true;
    }
    if(generator_expression(s, expr)) {
        return true;
    }
    if(dict(s, expr)) {
        return true;
    }
    if(set(s, expr)) {
        return true;
    }
    if(repr(s, expr)) {
        return true;
    }
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
    return false;
}

bool enclosed(State & s, AstExpr & ast, TokenKind left, TokenKind right, bool (*fun)(State & s, AstExpr &)) {
    StateGuard guard(s, ast);
    if(expect(s, left)) {
        if(fun(s, ast)) {
            return expect(s, right)
                && guard.commit();
        }
    }
    return false;
}

bool target(State & s, AstExpr & ast) {
    return enclosed(s, ast, TokenKind::LeftParen, TokenKind::RightParen, target_list)
        || enclosed(s, ast, TokenKind::LeftBracket, TokenKind::RightBracket, target_list)
        || attributeref(s, ast)
        || subscription(s, ast)
        || id(s, ast)
        ;
}

bool target_list(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    for(;;) {
        AstExpr e;
        if(target(s, e)) {
            exprs->items.push_back(e);
            if(!expect(s, TokenKind::Comma)) {
                return guard.commit();
            }
        }
        else {
            return exprs->items.empty()
                && guard.commit();
        }
    }
    return false;
}

bool atom(State & s, AstExpr & expr) {
    return id(s, expr)
        || literal(s, expr)
        || enclosure(s, expr)
        ;
}

bool comprehension(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstComprPtr compr;
    location(s, create(compr));
    ast = compr;
    if(expression(s, compr->target)) {
        return comp_for(s, compr->iter)
            && guard.commit();
    }
    return false;
}

bool not_test(State & s, AstExpr & ast) {
    if(comparison(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBoolOpPtr op;
    location(s, create(op));
    ast = op;
    if(expect(s, Token::KeywordNot)) {
        AstExpr expr;
        if(!not_test(s, expr)) {
            return false;
        }
        op->values.push_back(expr);
        ast = op;
        return guard.commit();
    }
    return false;
}

bool and_test(State & s, AstExpr & ast) {
    if(not_test(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    if(cls(top(s)) == TokenClass::Default && and_test(s, ast)) {
        AstBoolOpPtr op;
        location(s, create(op));
        ast = op;
        if(expect(s, Token::KeywordOr)) {
            AstExpr right;
            if(!not_test(s, right)) {
                return false;
            }
            op->op = AstBoolOpType::Or;
            op->values.push_back(ast);
            op->values.push_back(right);
            return guard.commit();
        }
    }
    return false;
}

bool or_test(State & s, AstExpr & ast) {
    if(and_test(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    if(cls(top(s)) == TokenClass::Default && or_test(s, ast)) {
        AstBoolOpPtr op;
        location(s, create(op));
        ast = op;
        if(expect(s, Token::KeywordOr)) {
            AstExpr right;
            if(!and_test(s, right)) {
                return false;
            }
            op->op = AstBoolOpType::Or;
            op->values.push_back(ast);
            op->values.push_back(right);
            return guard.commit();
        }
    }
    return false;
}

bool old_test(State & s, AstExpr & expr) {
    return or_test(s, expr) || apply<AstLambdaPtr>(s, expr, old_lambdadef);
}

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

bool attributeref(State & s, AstExpr & expr) {
    StateGuard guard(s, expr);
    AstAttributePtr attr;
    location(s, create(attr));
    expr = attr;
    if((token(top(s)) == Token::Identifier && atom(s, attr->value)) || primary(s, attr->value)) {
        if(expect(s, TokenKind::Dot)) {
            if(consume_value(s, Token::Identifier, attr->attribute))
            {
                return guard.commit();
            }
        }
    }
    return false;
}

bool call(State & s, AstExpr & expr) {
    StateGuard guard(s, expr);

    AstCallPtr callptr;
    location(s, create(callptr));
    expr = callptr;
    if((token(top(s)) == Token::Identifier && atom(s, callptr->function)) || primary(s,callptr->function)) {
        if(!expect(s, TokenKind::LeftParen)) {
            return false;
        }
        AstExpr expr;
        if(comprehension(s, expr)) {
            callptr->arguments.push_back(expr);
        }
        // Not a comprehension
        else {
            if(!argument_list(s, callptr)) {
                return false;
            }
        }
        return expect(s, TokenKind::RightParen)
            && guard.commit();
    }
    return false;
}

bool simple_slicing(State & s, AstSliceKindPtr & ast) {
    StateGuard guard(s, ast);
    AstSlicePtr slice;
    location(s, create(slice));
    expression(s, slice->lower);
    if(!expect(s, TokenKind::Colon)) {
        return false;
    }
    expression(s, slice->upper);
    if(expect(s, TokenKind::Colon)) {
        return false;
    }
    return is(s, TokenKind::RightBracket)
        && guard.commit();
}

bool slice_item(State & s, AstSlicePtr & slice) {
    StateGuard guard(s, slice);
    location(s, create(slice));
    expression(s, slice->lower);
    if(!expect(s, TokenKind::Colon)) {
        return false;
    }
    expression(s, slice->upper);
    if(expect(s, TokenKind::Colon)) {
        expression(s, slice->step);
    }
    return (is(s, TokenKind::Comma)
        || is(s, TokenKind::RightBracket))
        && guard.commit();
}

bool extended_slicing(State & s, AstSliceKindPtr & ast) {
    StateGuard guard(s, ast);
    AstExtSlicePtr extslice;
    location(s, create(extslice));
    return false;
}

bool slicing(State & s, AstSliceKindPtr & ast) {
    return simple_slicing(s, ast)
        || extended_slicing(s, ast);
}

bool subscription(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstSubscriptPtr subscript;
    location(s, create(subscript));
    ast = subscript;
    if((token(top(s)) == Token::Identifier && atom(s, subscript->value)) || primary(s, subscript->value)) {
        AstIndexPtr idx;
        location(s, create(idx));
        if(!expect(s, TokenKind::LeftBracket)) {
            return false;
        }
        if(slicing(s, subscript->slice)) {
            return guard.commit();
        }
        subscript->slice = idx;
        if(!expression(s, idx->value)) {
            return false;
        }
        if(!expect(s, TokenKind::RightBracket)) {
            return false;
        }
        return guard.commit();
    }
    return false;
}

bool primary(State & s, AstExpr & ast) {
    if(end(s)) {
        return false;
    }
    if(token(top(s)) != Token::Identifier) {
        return atom(s, ast);
    }
    if(attributeref(s, ast) || call(s, ast) || subscription(s, ast) || atom(s, ast)) {
        return true;
    }
    return false;
}

// Behaves different - Returns true even without a match
// Only on error it'll return false
bool positional_arguments(State & s, AstExprList & ast) {
    for(;;) {
        StateGuard guard(s);
        AstExpr expr;
        if(!expression(s, expr)) {
            break;
        }
        if(is(s, Token::OpAssign)) {
            return false;
        }
        guard.commit();
        ast.push_back(expr);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return true;
}

// Behaves different - Returns true even without a match
// Only on error it'll return false
bool keyword_arguments(State & s, AstKeywordList & ast) {
    while(is(s, Token::Identifier)) {
        StateGuard guard(s);
        AstKeywordPtr kw;
        location(s, create(kw));
        consume_value(s, Token::Identifier, kw->name);
        if(!expect(s, Token::OpAssign)) {
            break;
        }
        if(!expression(s, kw->value)) {
            return false;
        }
        guard.commit();
        ast.push_back(kw);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return true;
}

// Behaves different - Returns true even without a match
// Only on error it'll return false
bool args_arguments(State & s, AstExprList & ast) {
    while(expect(s, TokenKind::Star)) {
        AstExpr expr;
        if(!expression(s, expr)) {
            return false;
        }
        ast.push_back(expr);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return true;
}

// Behaves different - Returns true even without a match
// Only on error it'll return false
bool kwargs_arguments(State & s, AstExprList & ast) {
    while(expect(s, TokenKind::DoubleStar)) {
        AstExpr expr;
        if(!expression(s, expr)) {
            return false;
        }
        ast.push_back(expr);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return true;
}

bool argument_list(State & s, AstCallPtr & ast) {
    StateGuard guard(s);
    // positional -> [ [*args], [keywords], **kwargs]
    if(!positional_arguments(s, ast->arguments)) {
        return false;
    }
    if(!args_arguments(s, ast->args)) {
        return false;
    }
    if(!keyword_arguments(s, ast->keywords)) {
        return false;
    }
    if(!kwargs_arguments(s, ast->kwargs)) {
        return false;
    }
    return guard.commit();
}

bool raise(State & s, AstRaisePtr & ast) {    
    if(expect(s, Token::KeywordRaise)) {
        location(s, create(ast));
        expression(s, ast->arg0);
        expect(s, TokenKind::Comma);
        expression(s, ast->arg1);
        expect(s, TokenKind::Comma);
        expression(s, ast->arg2);
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
    if(expression(s, ast->context)) {
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

bool with(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    if(expect(s, Token::KeywordWith)) {
        AstWithPtr with;
        location(s, create(with));
        ast = with;
        AstWithItemPtr item;
        while(with_item(s, item)) {
            with->items.push_back(item);
            if(!expect(s, TokenKind::Comma)) {
                break;
            }
        }
        if(expect(s, TokenKind::Colon)){
            expect(s, Token::NewLine);
            if(suite(s, with->body)) {
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

bool conditional_expression(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstIfExprPtr ifexpr;
    location(s, create(ifexpr));
    if(or_test(s, ast)) {
        if(expect(s, Token::KeywordIf)) {
            ifexpr->body = ast;
            ast = ifexpr;
            if(or_test(s, ifexpr->test)) {
                if(expect(s, Token::KeywordElse)) {
                    return expression(s, ifexpr->orelse)
                        && guard.commit();
                }
            }
        }
        else {
            return guard.commit();
        }
    }
    return false;
}

bool expression(State & s, AstExpr & ast) {
    return lambda_expr(s, ast)
        || conditional_expression(s, ast);
}
bool u_expr(State & s, AstExpr & ast);
bool power(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    if(primary(s, ast)) {
        if(expect(s, TokenKind::DoubleStar)) {
            AstBinOpPtr bin;
            location(s, create(bin));
            bin->left = ast;
            bin->op = AstBinOpType::Power;
            ast = bin;
            return u_expr(s, bin->right)
                && guard.commit();
        }
        return guard.commit();
    }
    return false;
}

bool u_expr(State & s, AstExpr & ast) {
    if(power(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstUnaryOpPtr unary;
    location(s, create(unary));
    ast = unary;
    if(expect(s, TokenKind::Minus)) {
        unary->op = AstUnaryOpType::Sub;
        return u_expr(s, unary->operand)
            && guard.commit();
    }
    if(expect(s, TokenKind::Plus)) {
        unary->op = AstUnaryOpType::Add;
        return u_expr(s, unary->operand)
            && guard.commit();
    }
    if(expect(s, TokenKind::Tilde)) {
        unary->op = AstUnaryOpType::Invert;
        return u_expr(s, unary->operand)
            && guard.commit();
    }
    return false;
}

bool m_expr(State & s, AstExpr & ast) {
    if(u_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;

    if(cls(top(s)) == TokenClass::Default && m_expr(s, bin->left)) {
        if(expect(s, TokenKind::Star)) {
            bin->op = AstBinOpType::Mult;
        }
        else if(expect(s, TokenKind::DoubleSlash)) {
            bin->op = AstBinOpType::FloorDiv;
        }
        else if(expect(s, TokenKind::Slash)) {
            bin->op = AstBinOpType::Div;
        }
        else if(expect(s, TokenKind::Percent)) {
            bin->op = AstBinOpType::Mod;
        }
        else {
            return false;
        }
        return u_expr(s, bin->right)
            && guard.commit();
    }
    return false;
}

bool a_expr(State & s, AstExpr & ast) {
    if(m_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    if(cls(top(s)) == TokenClass::Default && a_expr(s, bin->left)) {
        if(expect(s, TokenKind::Plus)) {
            bin->op = AstBinOpType::Add;
        }
        else if(expect(s, TokenKind::Minus)) {
            bin->op = AstBinOpType::Sub;
        }
        else {
            return false;
        }
        return m_expr(s, bin->right)
            && guard.commit();
    }
    return false;
}

bool shift_expr(State & s, AstExpr & ast) {
    if(a_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    if(cls(top(s)) == TokenClass::Default && shift_expr(s, bin->left)) {
        if(expect(s, TokenKind::RightShift)) {
            bin->op = AstBinOpType::RightShift;
        }
        else if(expect(s, TokenKind::LeftShift)) {
            bin->op = AstBinOpType::RightShift;
        }
        else {
            return false;
        }
        return a_expr(s, bin->right)
            && guard.commit();
    }
    return false;
}

bool and_expr(State & s, AstExpr & ast) {
    if(shift_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    if(cls(top(s)) == TokenClass::Default && and_expr(s, bin->left)) {
        if(expect(s, TokenKind::BinAnd)) {
            bin->op = AstBinOpType::BitAnd;
            return shift_expr(s, bin->right)
                && guard.commit();
        }
    }
    return false;
}

bool xor_expr(State & s, AstExpr & ast) {
    if(and_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    if(cls(top(s)) == TokenClass::Default && xor_expr(s, bin->left)) {
        if(expect(s, TokenKind::CircumFlex)) {
            bin->op = AstBinOpType::BitXor;
            return and_expr(s, bin->right)
                && guard.commit();
        }
    }
    return false;
}

bool or_expr(State & s, AstExpr & ast) {
    if(xor_expr(s, ast)) {
        return true;
    }
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    if(cls(top(s)) == TokenClass::Default && or_expr(s, bin->left)) {
        if(expect(s, TokenKind::BinOr   )) {
            bin->op = AstBinOpType::BitOr;
            return xor_expr(s, bin->right)
                && guard.commit();
        }
    }
    return false;
}

bool comparison(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstComparePtr comp;
    location(s, create(comp));
    if(or_expr(s, ast)) {
        bool had_op = false;
        AstCompareOpType op;
        while(op_compare(s, op)) {
            if(!had_op) {
                comp->left = ast;
                ast = comp;
            }
            had_op = true;
            AstExpr expr;
            if(!or_expr(s, expr)) {
                return false;
            }
            comp->comperators.push_back(expr);
            comp->ops.push_back(op);
        }
        if(!had_op || (had_op && !comp->comperators.empty())) {
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
        if(!expression(s, ast->values, ast->newline)) {
            return false;
        }
        // invert token last comma
        ast->newline = !ast->newline;
        return guard.commit();
    }
    return false;
}

bool expression_statement(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstExpressionStatementPtr expr;
    location(s ,create(expr));
    ast = expr;
    if(expression(s, expr->expr)) {
        if(expect(s, TokenKind::SemiColon) || expect(s, TokenKind::NewLine)) {
            return guard.commit();
        }
    }
    return false;
}

bool statement_inner(State & s, AstStmt & ast) {
    switch(token(top(s))) {
    case Token::Dedent:
        return false;
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
    case Token::KeywordWith:
        return with(s, ast);
    case Token::Identifier:
        if(top(s).value == "print") {
            return apply<AstPrintPtr>(s, ast, print);
        }
        break;
    }
    return flow_statement(s, ast)
         || expression_statement(s, ast);
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
