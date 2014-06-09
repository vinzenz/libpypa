#include  <pypa/parser/apply.hh>
#include <pypa/parser/parser_fwd.hh>
#include <cassert>

extern "C" double strtod(const char *s00, char **se);

namespace pypa {

void syntax_error(State & s, char const * message) {}

template< typename Fun >
bool generic_binop_expr(State & s, AstExpr & ast, TokenKind op, AstBinOpType op_type, Fun fun) {
    StateGuard guard(s, ast);
    if(fun(s, ast)) {
        if(expect(s, op)) {
            AstBinOpPtr bin;
            location(s, create(bin));
            bin->left = ast;
            bin->op = op_type;
            ast = bin;
            if(!fun(s, bin->right)) {
                return false;
            }
        }
    }
    return guard.commit();
}

template< typename Fun >
bool generic_boolop_expr(State & s, AstExpr & ast, Token op, AstBoolOpType op_type, Fun fun) {
    StateGuard guard(s, ast);
    if(fun(s, ast)) {
        if(is(s, op)) {
            AstBoolOpPtr p;
            location(s, create(p));
            p->values.push_back(ast);
            p->op = op_type;
            ast = p;
            while(expect(s, op)) {
                AstExpr tmp;
                if(!fun(s, tmp)) {
                    return false;
                }
                p->values.push_back(tmp);
            }
        }
    }
    return guard.commit();
}

bool dotted_as_names(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr lst;
    location(s, create(lst));
    ast = lst;
    AstExpr dotted;
    while(dotted_as_name(s, dotted)) {
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !lst->items.empty() && guard.commit();
}

bool import_as_name(State & s, AstAliasPtr & ast) {
    StateGuard guard(s, ast);    
    location(s, create(ast));
    if(consume_value(s, Token::Identifier, ast->name))
    {
        if(expect(s, Token::KeywordAs)) {
            if(!consume_value(s, Token::Identifier, ast->as_name) || ast->as_name.empty()) {
                return false;
            }
        }
        return guard.commit();
    }
    return false;
}

bool try_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstTryExceptPtr try_except;
    location(s, create(try_except));
    ast = try_except;
    // TODO:
    // (expect(s, Token::KeywordTry) expect(s, TokenKind::Colon)
    // suite
    // ((except_clause expect(s, TokenKind::Colon) suite)+
    // [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    // [expect(s, Token::KeywordFinally) expect(s, TokenKind::Colon) suite]
    // ||expect(s, Token::KeywordFinally) expect(s, TokenKind::Colon) suite))
    return false ; //guard.commit();
}

bool comp_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordIf) old_test [comp_iter]
    return guard.commit();
}

bool testlist1(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    if(!test(s, ast)) {
        return false;
    }
    if(is(s, TokenKind::Comma)) {
        AstExpressionsPtr exprs;
        create(exprs);
        exprs->line = ast->line;
        exprs->column = ast->column;
        ast = exprs;
        while(expect(s, TokenKind::Comma)) {
            AstExpr temp;
            if(!test(s, temp)) {
                return false;
            }
            exprs->items.push_back(temp);
        }
    }
    return guard.commit();
}

bool get_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstNamePtr name;
    location(s, create(name));
    ast = name;
    if(consume_value(s, Token::Identifier, name->id)) {
        return guard.commit();
    }
    return false;
}

bool dotted_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstAttributePtr attribute;
    location(s, create(attribute));
    ast = attribute;
    if(get_name(s, attribute->value)) {
        if(expect(s, TokenKind::Dot)) {
            return dotted_name(s, attribute->attribute)
                && guard.commit();
        }
        else {
            ast = attribute->value;
            return guard.commit();
        }
    }
    return false;
}

bool small_stmt(State & s, AstStmt & ast) {
    return expr_stmt(s, ast)
        || print_stmt(s, ast)
        || del_stmt(s, ast)
        || pass_stmt(s, ast)
        || flow_stmt(s, ast)
        ||import_stmt(s, ast)
        || global_stmt(s, ast)
        || exec_stmt
        || assert_stmt(s, ast)
    ;
}

bool augassign(State & s, AstAugAssignPtr & ast) {
    if(expect(s, TokenKind::PlusEqual))
        ast->op = AstBinOpType::Add;
    else if(expect(s, TokenKind::MinusEqual))
        ast->op = AstBinOpType::Sub;
    else if(expect(s, TokenKind::StarEqual))
        ast->op = AstBinOpType::Mult;
    else if(expect(s, TokenKind::SlashEqual))
        ast->op = AstBinOpType::Div;
    else if(expect(s, TokenKind::PercentEqual))
        ast->op = AstBinOpType::Mod;
    else if(expect(s, TokenKind::BinAndEqual))
        ast->op = AstBinOpType::BitAnd;
    else if(expect(s, TokenKind::BinOrEqual))
        ast->op = AstBinOpType::BitOr;
    else if(expect(s, TokenKind::CircumFlexEqual))
        ast->op = AstBinOpType::BitXor;
    else if(expect(s, TokenKind::LeftShiftEqual))
        ast->op = AstBinOpType::LeftShift;
    else if(expect(s, TokenKind::RightShiftEqual))
        ast->op = AstBinOpType::RightShift;
    else if(expect(s, TokenKind::DoubleStarEqual))
        ast->op = AstBinOpType::Power;
    else if(expect(s, TokenKind::DoubleSlashEqual))
        ast->op = AstBinOpType::FloorDiv;
    else
        return false;
    return true;
}

bool import_from(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstImportFromPtr impfrom;
    location(s, create(impfrom));
    ast = impfrom;
    impfrom->level = 0;
    if(expect(s, Token::KeywordFrom)) {
        // (expect(s, TokenKind::Dot)* dotted_name || expect(s, TokenKind::Dot)+)
        if(is(s, TokenKind::Dot)) {
            while(expect(s, TokenKind::Dot)) ++impfrom->level;
        }
        if(!dotted_name(s, impfrom->module) && impfrom->level == 0) {
            syntax_error(s, "Expected name of module");
            return false;
        }
        //    expect(s, Token::KeywordImport)
        if(!expect(s, Token::KeywordImport)) {
            syntax_error(s, "Expected 'import'");
            return false;
        }
        // expect(s, TokenKind::Star)
        if(expect(s, TokenKind::Star)) {
            // ok
        }
        // || expect(s, TokenKind::LeftParen) import_as_names expect(s, TokenKind::RightParen)
        else if(expect(s, TokenKind::LeftParen)) {
            if(!import_as_names(s, impfrom->names)) {
                    return false;
            }
            if(!expect(s, TokenKind::RightParen)) {
                return false;
            }
            // ok
        }
        // || import_as_names))
        else if(import_as_names(s, impfrom->names)) {
            // ok
        }
        else {
            return false;
        }
    }
    return guard.commit();
}

bool fplist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // fpdef (expect(s, TokenKind::Comma) fpdef)* [expect(s, TokenKind::Comma)]    
    AstExpr temp;
    if(fpdef(s, temp)) {
        exprs->items.push_back(temp);
        while(is(s, TokenKind::Comma)) {
            if(!fpdef(s, temp)) {
                break;
            }
        }
        return guard.commit();
    }
    return false;
}

bool import_as_names(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // import_as_name (expect(s, TokenKind::Comma) import_as_name)* [expect(s, TokenKind::Comma)]
    return guard.commit();
}

bool return_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstReturnPtr ret;
    location(s, create(ret));
    ast = ret;
    // expect(s, Token::KeywordReturn) [testlist]
    if(!expect(s, Token::KeywordReturn))
    {
        return false;
    }
    testlist(s, ret->value);
    return guard.commit();
}

bool testlist_safe(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // old_test [(expect(s, TokenKind::Comma) old_test)+ [expect(s, TokenKind::Comma)]]
    return guard.commit();
}

bool not_test(State & s, AstExpr & ast) {
    StateGuard guard(s);
    // expect(s, Token::KeywordNot) not_test || comparison
    if(expect(s, Token::KeywordNot)) {
        if(!not_test(s, ast)) {
            return false;
        }
    }
    else if(!comparison(s, ast)) {
        return false;
    }
    return guard.commit();
}

bool testlist_comp(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    AstExpr tmp;
    // test ( comp_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] )
    if(!test(s, tmp)) {
        return false;
    }
    exprs->items.push_back(tmp);
    for(;;) {
        if(!comp_for(s, tmp)) {
            if(!expect(s, TokenKind::Comma)) {
                break;
            }
            if(!test(s, tmp)) {
                return false;
            }
        }
        exprs->items.push_back(tmp);
    }
    return guard.commit();
}

bool except_clause(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordExcept) [test [(expect(s, Token::KeywordAs) || expect(s, TokenKind::Comma)) test]]
    return guard.commit();
}

bool listmaker(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // test ( list_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] )
    return guard.commit();
}

bool old_test(State & s, AstExpr & ast) {
    // or_test || old_lambdef
    return or_test(s, ast)
        || old_lambdef(s, ast);
}

bool import_name(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordImport) dotted_as_names
    return guard.commit();
}

bool break_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordBreak)
    return guard.commit();
}

bool with_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordWith) with_item (expect(s, TokenKind::Comma) with_item)*  expect(s, TokenKind::Colon) suite
    return guard.commit();
}

bool comp_op(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::Less)||expect(s, TokenKind::Greater)||expect(s, TokenKind::EqualEqual)||expect(s, TokenKind::GreaterEqual)||expect(s, TokenKind::LessEqual)||expect(s, TokenKind::NotEqual)||expect(s, Token::KeywordIn)||expect(s, Token::KeywordNot) expect(s, Token::KeywordIn)||expect(s, Token::KeywordIs)||expect(s, Token::KeywordIs) expect(s, Token::KeywordNot)
    return guard.commit();
}

bool raise_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordRaise) [test [expect(s, TokenKind::Comma) test [expect(s, TokenKind::Comma) test]]]
    return guard.commit();
}

bool atom(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // (expect(s, TokenKind::LeftParen) [yield_expr||testlist_comp] expect(s, TokenKind::RightParen) ||expect(s, TokenKind::LeftBracket) [listmaker] expect(s, TokenKind::RightBracket) ||expect(s, TokenKind::LeftBrace) [dictorsetmaker] expect(s, TokenKind::RightBrace) ||expect(s, TokenKind::BackQuote) testlist1 expect(s, TokenKind::BackQuote) ||expect(s, Token::Identifier) || NUMBER || STRING+)
    return guard.commit();
}

bool parameters(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::LeftParen) [varargslist] expect(s, TokenKind::RightParen)
    return guard.commit();
}

bool dotted_as_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // dotted_name [expect(s, Token::KeywordAs) expect(s, Token::Identifier)]
    return guard.commit();
}

bool arglist(State & s, AstArguments & ast) {
    StateGuard guard(s);
    // location(s, create(ast));
    // (argument expect(s, TokenKind::Comma))* (argument [expect(s, TokenKind::Comma)]||expect(s, TokenKind::Star) test (expect(s, TokenKind::Comma) argument)* [expect(s, TokenKind::Comma) expect(s, TokenKind::DoubleStar) test]||expect(s, TokenKind::DoubleStar) test)
    return guard.commit();
}

bool single_input(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::NewLine) || simple_stmt || compound_stmt expect(s, Token::NewLine)
    return guard.commit();
}

bool fpdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::Identifier) || expect(s, TokenKind::LeftParen) fplist expect(s, TokenKind::RightParen)
    if(!get_name(s, ast) && !(expect(s, TokenKind::LeftParen) && fplist(s, ast) && expect(s, TokenKind::RightParen))) {
        return false;
    }
    return guard.commit();
}

bool shift_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstBinOpPtr bin;
    location(s, create(bin));
    ast = bin;
    // arith_expr ((expect(s, TokenKind::LeftShift)||expect(s, TokenKind::RightShift)) arith_expr)*
    if(!arith_expr(s, bin->left)) {
        return false;
    }
    if(!(expect(s, TokenKind::LeftShift) || expect(s, TokenKind::RightShift))) {
        ast = bin->left;
    }
    else if(!arith_expr(s, bin->right)){
        return false;
    }
    return guard.commit();
}

bool exprlist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // expr (expect(s, TokenKind::Comma) expr)* [expect(s, TokenKind::Comma)]
    AstExpr tmp;
    while(expr(s, tmp)) {
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !exprs->items.empty() && guard.commit();
}

bool comp_for(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstForExprPtr for_;
    location(s, create(for_));
    ast = for_;
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) or_test [comp_iter]
    if(!expect(s, Token::KeywordFor)) {
        return false;
    }
    if(!exprlist(s, for_->items)) {
        return false;
    }
    if(!expect(s, Token::KeywordIn)) {
        return false;
    }
    if(!or_test(s, for_->generators)) {
        return false;
    }
    comp_iter(s, for_->iter);
    return guard.commit();
}

bool simple_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstSuitePtr suite_;
    location(s, create(suite_));
    // small_stmt (expect(s, TokenKind::SemiColon) small_stmt)* [expect(s, TokenKind::SemiColon)] expect(s, Token::NewLine)
    AstStmt tmp;
    if(!small_stmt(s, tmp)) {
        return false;
    }
    suite_->items.push_back(tmp);
    if(expect(s, TokenKind::SemiColon)) {
        while(small_stmt(s, tmp)) {
            if(!expect(s, TokenKind::SemiColon)) {
                break;
            }
        }
    }
    if(!expect(s, TokenKind::NewLine)) {
        return false;
    }
    return guard.commit();
}

bool list_iter(State & s, AstExpr & ast) {
    return list_for(s, ast)
        || list_if(s, ast);
}

bool exec_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstExecPtr exec;
    location(s, create(exec));
    // expect(s, Token::KeywordExec) expr [expect(s, Token::KeywordIn) test [expect(s, TokenKind::Comma) test]]       
    if(!expect(s, Token::KeywordExec)) {
        return false;
    }
    if(!test(s, exec->body)) {
        return false;
    }
    if(expect(s, Token::KeywordIn)) {
        if(!test(s, exec->globals)) {
            return false;
        }
        if(expect(s, TokenKind::Comma)) {
            if(!test(s, exec->locals)) {
                return false;
            }
        }
    }
    return guard.commit();
}

bool factor(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // (expect(s, TokenKind::Plus)||expect(s, TokenKind::Minus)||expect(s, TokenKind::Tilde)) factor || power
    if(is(s, TokenKind::Plus)||is(s, TokenKind::Minus)||is(s, TokenKind::Tilde)) {
        // AstUnaryOpType::
        AstUnaryOpPtr unary;
        location(s, create(unary));
        ast = unary;
        if(expect(s, TokenKind::Plus)) {
            unary->op = AstUnaryOpType::Add;
        }
        else if(expect(s, TokenKind::Minus)) {
            unary->op = AstUnaryOpType::Sub;
        }
        else if(expect(s, TokenKind::Tilde)) {
            unary->op = AstUnaryOpType::Invert;
        }
        return factor(s, unary->operand) && guard.commit();
    }
    return power(s, ast) && guard.commit();
}

bool test(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // or_test [expect(s, Token::KeywordIf) or_test expect(s, Token::KeywordElse) test] || lambdef
    if(or_test(s, ast)) {
        if(expect(s, Token::KeywordIf)) {
            AstIfExprPtr ifexpr;
            location(s, create(ifexpr));
            ifexpr->body = ast;
            ast = ifexpr;
            if(!or_test(s, ifexpr->test)) {
                return false;
            }
            if(!expect(s, Token::KeywordElse)) {
                return false;
            }
            if(!test(s, ifexpr->orelse)) {
                return false;
            }
        }
    }
    else if(!lambdef(s, ast)){
        return false;
    }
    return guard.commit();
}

bool global_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordGlobal) expect(s, Token::Identifier) (expect(s, TokenKind::Comma) expect(s, Token::Identifier))*
    return guard.commit();
}

bool subscript(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::Dot) expect(s, TokenKind::Dot) expect(s, TokenKind::Dot) || test || [test] expect(s, TokenKind::Colon) [test] [sliceop]
    return guard.commit();
}

bool with_item(State & s, AstWithItemPtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // test [expect(s, Token::KeywordAs) expr]
    if(!test(s, ast->context)) {
        return false;
    }
    if(expect(s, Token::KeywordAs)) {
        if(!expr(s, ast->optional)) {
            return false;
        }
    }
    return guard.commit();
}

bool decorators(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // decorator+
    AstExpr temp;
    while(decorator(s, temp)) {
        exprs->items.push_back(temp);
    }
    return !exprs->items.empty() && guard.commit();
}

bool compound_stmt(State & s, AstStmt & ast) {
    // if_stmt || while_stmt || for_stmt || try_stmt || with_stmt || funcdef || classdef || decorated
    return if_stmt(s, ast)
        || while_stmt(s, ast)
        || for_stmt(s, ast)
        || try_stmt(s, ast)
        || with_stmt(s, ast)
        || funcdef(s, ast)
        || classdef(s, ast)
        || decorated(s, ast)
    ;
}

bool and_expr(State & s, AstExpr & ast) {
    return generic_binop_expr(s, ast, TokenKind::BinAnd, AstBinOpType::BitAnd, shift_expr);
}

bool flow_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // break_stmt || continue_stmt || return_stmt || raise_stmt || yield_stmt
    return guard.commit();
}

bool yield_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordYield) [testlist]
    return guard.commit();
}

bool power(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // atom trailer* [expect(s, TokenKind::DoubleStar) factor]
    return guard.commit();
}

bool print_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // 'print' ( [ test (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] ] ||expect(s, TokenKind::RightShift) test [ (expect(s, TokenKind::Comma) test)+ [expect(s, TokenKind::Comma)] ] )
    return guard.commit();
}

bool subscriptlist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // subscript (expect(s, TokenKind::Comma) subscript)* [expect(s, TokenKind::Comma)]
    return guard.commit();
}

bool testlist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // test (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)]
    return guard.commit();
}

bool classdef(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordClass) expect(s, Token::Identifier) [expect(s, TokenKind::LeftParen) [testlist] expect(s, TokenKind::RightParen)] expect(s, TokenKind::Colon) suite
    return guard.commit();
}

bool stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // simple_stmt || compound_stmt
    return guard.commit();
}

bool argument(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // test [comp_for] || test expect(s, TokenKind::Equal) test
    return guard.commit();
}

bool assert_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordAssert) test [expect(s, TokenKind::Comma) test]
    return guard.commit();
}

bool list_for(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstForExprPtr for_;
    location(s, create(for_));
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) testlist_safe [list_iter]
    if(!expect(s, Token::KeywordFor)) {
        return false;
    }
    if(!exprlist(s, for_->items)) {
        return false;
    }
    if(!expect(s, Token::KeywordIn)) {
        return false;
    }
    if(!testlist_safe(s, for_->generators)) {
        return false;
    }
    list_iter(s, for_->iter);
    return guard.commit();
}

bool for_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) testlist expect(s, TokenKind::Colon) suite [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    return guard.commit();
}

bool and_test(State & s, AstExpr & ast) {
    // not_test (expect(s, ) not_test)*
    return generic_boolop_expr(s, ast, Token::KeywordAnd, AstBoolOpType::And, not_test);
}

bool lambdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordLambda) [varargslist] expect(s, TokenKind::Colon) test
    return guard.commit();
}

bool suite(State & s, AstStmt & ast) {
    // simple_stmt || expect(s, Token::NewLine) expect(s, Token::Indent) stmt+ expect(s, Token::Dedent)
    if(simple_stmt(s, ast)) return true;
    if(expect(s, Token::NewLine)) {
        StateGuard guard(s, ast);
        AstSuitePtr suite_;
        location(s, create(suite_));
        ast = suite_;
        if(expect(s, Token::Indent)) {
            AstStmt stmt_;
            if(stmt(s, stmt_)) {
                suite_->items.push_back(stmt_);
                while(stmt(s, stmt_)) {
                    suite_->items.push_back(stmt_);
                }
                if(!expect(s, Token::Dedent)) {
                    return false;
                }
                return guard.commit();
            }
        }
    }
    return false;
}

bool funcdef(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordDef) expect(s, Token::Identifier) parameters expect(s, TokenKind::Colon) suite
    return guard.commit();
}

bool decorated(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // decorators (classdef || funcdef)
    return guard.commit();
}

bool expr_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // testlist (augassign (yield_expr||testlist) ||(expect(s, TokenKind::Equal) (yield_expr||testlist))*)
    return guard.commit();
}

bool old_lambdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordLambda) [varargslist] expect(s, TokenKind::Colon) old_test
    return guard.commit();
}

bool continue_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordContinue)
    return guard.commit();
}

bool decorator(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::At) dotted_name [ expect(s, TokenKind::LeftParen) [arglist] expect(s, TokenKind::RightParen) ] expect(s, Token::NewLine)
    return guard.commit();
}

bool if_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstIfPtr if_;
    location(s, create(if_));
    ast = if_;
    // expect(s, Token::KeywordIf) test expect(s, TokenKind::Colon) suite
    if(!expect(s, Token::KeywordIf) || !test(s, if_->test) || !expect(s, TokenKind::Colon) || !suite(s, if_->body)) {
        return false;
    }
    // (expect(s, Token::KeywordElIf) test expect(s, TokenKind::Colon) suite)*
    if(is(s, Token::KeywordElIf)) {
        AstExpressionsPtr lst;
        location(s, create(lst));
        while(expect(s, Token::KeywordElIf)) {
            AstElseIfPtr elif;
            location(s, create(elif));
            if(!test(s, elif->test) || !expect(s, TokenKind::Colon) || !suite(s, elif->body)) {
                return false;
            }
            lst->items.push_back(elif);
        }
        if_->elif = lst;
    }
    // [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    if(expect(s, Token::KeywordElse)) {
        if(!expect(s, TokenKind::Colon) || !suite(s, if_->orelse)) {
            return false;
        }
    }
    return guard.commit();
}

bool sliceop(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::Colon) [test]
    return guard.commit();
}

bool comparison(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expr (comp_op expr)*
    return guard.commit();
}

bool term(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);    
    // factor (( factor)*
    if(factor(s, ast)) {
        TokenKind k = kind(top(s));
        if(expect(s, TokenKind::Star) || expect(s, TokenKind::Slash) || expect(s, TokenKind::Percent) || expect(s, TokenKind::DoubleSlash)) {
            AstBinOpPtr bin;
            location(s, create(bin));
            bin->left = ast;
            ast = bin;
            switch(k) {
            case TokenKind::Star:           bin->op = AstBinOpType::Mult; break;
            case TokenKind::Slash:          bin->op = AstBinOpType::Div; break;
            case TokenKind::Percent:        bin->op = AstBinOpType::Mod; break;
            case TokenKind::DoubleSlash:    bin->op = AstBinOpType::FloorDiv; break;
            default: // WTF?
                assert(false && "This should not happen. Unexpected token type received.");
                return false;
            }
            if(!factor(s, bin->right)) {
                return false;
            }
        }

    }
    return guard.commit();
}

bool pass_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstPassPtr pass_;
    location(s, create(pass_));
    ast = pass_;
    if(!expect(s, Token::KeywordPass)) {
            return false;
    }
    return guard.commit();
}

bool xor_expr(State & s, AstExpr & ast) {
    return generic_binop_expr(s, ast, TokenKind::CircumFlex, AstBinOpType::BitXor, shift_expr);
}

bool file_input(State & s, AstModulePtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));    
    location(s, create(ast->body));
    // (expect(s, Token::NewLine) || stmt)* expect(s, Token::End)
    while(!is(s, Token::End)) {
        AstStmt statement;
        if(expect(s, Token::NewLine)) {
            continue;
        }
        else if(stmt(s, statement)) {
            ast->body->items.push_back(statement);
        }
        else {
            return false;
        }
    }
    return guard.commit();
}

bool or_test(State & s, AstExpr & ast) {
    return generic_boolop_expr(s, ast, Token::KeywordOr, AstBoolOpType::Or, and_test);
}

bool dictorsetmaker(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // ( (test expect(s, TokenKind::Colon) test (comp_for || (expect(s, TokenKind::Comma) test expect(s, TokenKind::Colon) test)* [expect(s, TokenKind::Comma)])) ||(test (comp_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)])) )
    return guard.commit();
}

bool expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // xor_expr (expect(s, TokenKind::BinOr) xor_expr)*
    return guard.commit();
}

bool del_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordDel) exprlist
    return guard.commit();
}

bool while_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordWhile) test expect(s, TokenKind::Colon) suite [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    return guard.commit();
}

bool varargslist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // ((fpdef [expect(s, TokenKind::Equal) test] expect(s, TokenKind::Comma))*(expect(s, TokenKind::Star) expect(s, Token::Identifier) [expect(s, TokenKind::Comma) expect(s, TokenKind::DoubleStar) expect(s, Token::Identifier)] || expect(s, TokenKind::DoubleStar) expect(s, Token::Identifier)) ||fpdef [expect(s, TokenKind::Equal) test] (expect(s, TokenKind::Comma) fpdef [expect(s, TokenKind::Equal) test])* [expect(s, TokenKind::Comma)])
    return guard.commit();
}

bool trailer(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, TokenKind::LeftParen) [arglist] expect(s, TokenKind::RightParen) || expect(s, TokenKind::LeftBracket) subscriptlist expect(s, TokenKind::RightBracket) || expect(s, TokenKind::Dot) expect(s, Token::Identifier)
    return guard.commit();
}

bool import_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // import_name || import_from
    return guard.commit();
}

bool eval_input(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // testlist expect(s, Token::NewLine)* expect(s, Token::End)
    return guard.commit();
}

bool list_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::KeywordIf) old_test [list_iter]
    return guard.commit();
}

bool arith_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // term ((expect(s, TokenKind::Plus)||expect(s, TokenKind::Minus)) term)*
    return guard.commit();
}

bool comp_iter(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // comp_for || comp_if
    return guard.commit();
}

bool yield_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // yield_expr
    return guard.commit();
}


bool Parser::parse(Lexer & lexer, AstModulePtr & ast) {
    State state{&lexer, {}, {}, {}, lexer.next()};
    return file_input(state, ast);
}

}
