// Copyright 2014 Vinzenz Feenstra
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include  <pypa/parser/apply.hh>
#include <pypa/parser/parser_fwd.hh>
#include <cassert>
#include <double-conversion/src/double-conversion.h>

namespace pypa {

template<typename T>
AstPtr error_transform(std::shared_ptr<T> const & t) {
    return t;
}

template<typename T>
AstPtr error_transform(T const & t) {
    return std::make_shared<T>(t);
}

void report_error(State & s) {
    Error & e = s.errors.top();
    printf("  File \"%s\", line %d\n    %s\n    ", s.lexer->get_name().c_str(), e.cur.line, e.line.c_str());
    if(e.cur.column == 0) ++e.cur.column;
    for(int i = 0; i < e.cur.column - 1; ++i) {
        printf(" ");
    }
    printf("^\n%s: %s", e.type == ErrorType::SyntaxError ? "SyntaxError" : "IndentationError", e.message.c_str());
    if(e.detected_line != -1) {
        printf("\n-> Reported @%s:%d in %s\n\n", e.detected_file, e.detected_line, e.detected_function);
    }
}

void syntax_error_dbg(State & s, AstPtr ast, char const * message, int line = -1, char const * file = 0, char const * function = 0) {
    s.errors.push({ErrorType::SyntaxError, message, top(s), ast, s.lexer->get_line(top(s).line), line, file ? file : "", function ? function : ""});
    report_error(s);
}

void indentation_error_dbg(State & s, AstPtr ast, int line = -1, char const * file = 0, char const * function = 0) {
    s.errors.push({ErrorType::IndentationError, "", top(s), ast, s.lexer->get_line(top(s).line), line, file ? file : "", function ? function : ""});
    report_error(s);
}

#define syntax_error(s, AST_ITEM, msg) syntax_error_dbg(s, error_transform(AST_ITEM), msg, __LINE__, __FILE__, __PRETTY_FUNCTION__)
#define indentation_error(s, AST_ITEM) indentation_error_dbg(s, error_transform(AST_ITEM), __LINE__, __FILE__, __PRETTY_FUNCTION__)


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
    pop(s);
    result.num_type = top(s).value == "L" ? AstNumber::Long : AstNumber::Integer;
    unpop(s);
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
        double_conversion::StringToDoubleConverter conv(0, 0.0, 0.0, 0, 0);
        int length = int(dstr.size());
        int processed = 0;
        double result = conv.StringToDouble(dstr.c_str(), length, &processed);
        if(length != processed) {
            syntax_error(s, ast, "Invalid floating point number");
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
        if(ast->num_type == AstNumber::Long) {
            pop(s);
        }
        return guard.commit();
    }
    return false;
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

template< typename Fun >
bool generic_binop_expr(State & s, AstExpr & ast, TokenKind op, AstBinOpType op_type, Fun fun) {
    StateGuard guard(s, ast);
    if(fun(s, ast)) {
        while(expect(s, op)) {
            AstBinOpPtr bin;
            location(s, create(bin));
            bin->left = ast;
            bin->op = op_type;
            ast = bin;
            if(!fun(s, bin->right)) {
                syntax_error(s, ast, "Expected expression after operator");
            }
        }
        return guard.commit();
    }
    return false;
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
                    syntax_error(s, ast, "Expected expression after operator");
                    return false;
                }
                p->values.push_back(tmp);
            }
        }
        return guard.commit();
    }
    return false;
}

bool dotted_as_names(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr lst;
    location(s, create(lst));
    ast = lst;
    AstExpr dotted;
    while(dotted_as_name(s, dotted)) {
        if(dotted->type == AstType::Alias) {
            AstAliasPtr a = std::static_pointer_cast<AstAlias>(dotted);
            if(!a->as_name) {
                lst->items.push_back(a->name);
            }
            else {
                lst->items.push_back(dotted);
            }
        }
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(lst->items.empty()) {
        return false;
    }
    if(lst->items.size() == 1) {
        ast = lst->items.front();
    }
    return  guard.commit();
}

bool import_as_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstAliasPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    if(get_name(s, ptr->name))
    {
        if(expect(s, Token::KeywordAs)) {
            if(!get_name(s, ptr->as_name)) {
                syntax_error(s, ast, "Expected identifier after `as`");
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
    // (expect(s, Token::KeywordTry) expect(s, TokenKind::Colon)
    // -> suite
    if(!(expect(s, Token::KeywordTry) && expect(s, TokenKind::Colon))) {
        return false;
    }
    if(!suite(s, try_except->body)) {
        return false;
    }
    // ((except_clause expect(s, TokenKind::Colon) suite)+
    AstExpr clause;
    while(except_clause(s, clause)) {
        assert(clause->type == AstType::Except);
        AstExceptPtr except = std::static_pointer_cast<AstExcept>(clause);
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, clause, "Expected `:`");
            return false;
        }
        if(!suite(s, except->body)) {
            return false;
        }
        try_except->handlers.push_back(except);
    }
    // [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    if(expect(s, Token::KeywordElse)) {
        if(try_except->handlers.empty()) {
            syntax_error(s, ast, "`else` cannot follow `try` without `except` handler");
            return false;
        }
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "expected `:`");
            return false;
        }
        if(!suite(s, try_except->orelse)) {
            return false;
        }
    }
    // [expect(s, Token::KeywordFinally) expect(s, TokenKind::Colon) suite]
    // ||expect(s, Token::KeywordFinally) expect(s, TokenKind::Colon) suite))
    if(is(s, Token::KeywordFinally)) {
        AstTryFinallyPtr ptr;
        location(s, create(ptr));
        expect(s, Token::KeywordFinally);
        ast = ptr;
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "expected `:`");
            return false;
        }
        ptr->body = try_except;
        if(!suite(s, ptr->final_body)) {
            return false;
        }
    }
    return guard.commit();
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
    return print_stmt(s, ast)
        || expr_stmt(s, ast)
        || del_stmt(s, ast)
        || pass_stmt(s, ast)
        || flow_stmt(s, ast)
        || import_stmt(s, ast)
        || global_stmt(s, ast)
        || exec_stmt(s, ast)
        || assert_stmt(s, ast)
    ;
}

bool augassign(State & s, AstBinOpType & op) {
    if(expect(s, TokenKind::PlusEqual))
        op = AstBinOpType::Add;
    else if(expect(s, TokenKind::MinusEqual))
        op = AstBinOpType::Sub;
    else if(expect(s, TokenKind::StarEqual))
        op = AstBinOpType::Mult;
    else if(expect(s, TokenKind::SlashEqual))
        op = AstBinOpType::Div;
    else if(expect(s, TokenKind::PercentEqual))
        op = AstBinOpType::Mod;
    else if(expect(s, TokenKind::BinAndEqual))
        op = AstBinOpType::BitAnd;
    else if(expect(s, TokenKind::BinOrEqual))
        op = AstBinOpType::BitOr;
    else if(expect(s, TokenKind::CircumFlexEqual))
        op = AstBinOpType::BitXor;
    else if(expect(s, TokenKind::LeftShiftEqual))
        op = AstBinOpType::LeftShift;
    else if(expect(s, TokenKind::RightShiftEqual))
        op = AstBinOpType::RightShift;
    else if(expect(s, TokenKind::DoubleStarEqual))
        op = AstBinOpType::Power;
    else if(expect(s, TokenKind::DoubleSlashEqual))
        op = AstBinOpType::FloorDiv;
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
            syntax_error(s, ast, "Expected name of module");
            return false;
        }
        //    expect(s, Token::KeywordImport)
        if(!expect(s, Token::KeywordImport)) {
            syntax_error(s, ast, "Expected 'import'");
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
        return guard.commit();
    }
    return false;
}

bool import_as_names(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // import_as_name (expect(s, TokenKind::Comma) import_as_name)* [expect(s, TokenKind::Comma)]
    AstExpr alias;
    while(import_as_name(s, alias)) {
        exprs->items.push_back(alias);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !exprs->items.empty() && guard.commit();
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

bool testlist_safe(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    AstExpr temp;
    // old_test [(expect(s, TokenKind::Comma) old_test)+ [expect(s, TokenKind::Comma)]]
    while(old_test(s, temp)) {
        exprs->items.push_back(temp);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    return !exprs->items.empty() && guard.commit();
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
    if(is(s, Token::KeywordFor)) {
        AstComprPtr compr;
        location(s, create(compr));
        ast = compr;
        compr->target = tmp;
        if(!comp_for(s, compr->iter)) {
            return false;
        }
    }
    else {
        exprs->items.push_back(tmp);
        for(;;) {
            if(!expect(s, TokenKind::Comma)) {
                break;
            }
            if(!test(s, tmp)) {
                break;
            }
            exprs->items.push_back(tmp);
        }
    }
    return guard.commit();
}

bool except_clause(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExceptPtr except;
    location(s, create(except));
    ast = except;
    // expect(s, Token::KeywordExcept) [test [(expect(s, Token::KeywordAs) || expect(s, TokenKind::Comma)) test]]
    if(!expect(s, Token::KeywordExcept)) {
        return false;
    }

    if(test(s, except->type)) {
        if(expect(s, Token::KeywordAs) || expect(s, TokenKind::Comma)) {
            if(!test(s, except->name)) {
                return false;
            }
        }
    }

    return guard.commit();
}

bool listmaker(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstListPtr ptr;
    location(s, create(ptr));
    // test ( list_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] )
    if(test(s, ast)) {
        if(is(s, Token::KeywordFor)) {
            AstListCompPtr comp;
            location(s, create(comp));
            comp->element = ast;
            ast = comp;
            if(!list_for(s, comp->generators)) {
                return false;
            }
        }
        else {
            ptr->elements.push_back(ast);
            ast = ptr;
            while(expect(s, TokenKind::Comma)) {
                AstExpr tmp;
                if(!test(s, tmp)) {
                    break;
                }
                ptr->elements.push_back(tmp);
            }
        }
    }
    // else empty list => OK
    return guard.commit();
}

bool old_test(State & s, AstExpr & ast) {
    // or_test || old_lambdef
    return or_test(s, ast)
        || old_lambdef(s, ast);
}

bool import_name(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstExpressionStatementPtr expr;
    location(s, create(expr));
    ast = expr;
    // expect(s, Token::KeywordImport) dotted_as_names
    if(!expect(s, Token::KeywordImport)) {
        return false;
    }
    if(!dotted_as_names(s, expr->expr)) {
        syntax_error(s, ast, "Expected module name to import");
    }
    return guard.commit();
}

bool break_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstBreakPtr brk;
    location(s, create(brk));
    ast = brk;
    if(!expect(s, Token::KeywordBreak)) {
        return false;
    }
    return guard.commit();
}

bool with_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstWithPtr with;
    location(s, create(with));
    ast = with;
    // expect(s, Token::KeywordWith) with_item (expect(s, TokenKind::Comma) with_item)*  expect(s, TokenKind::Colon) suite
    if(!expect(s, Token::KeywordWith)) {
        return false;
    }
    AstWithItemPtr item;
    while(with_item(s, item)) {
        with->items.push_back(item);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, with->body)) {
        return false;
    }
    return guard.commit();
}

bool comp_op(State & s, AstCompareOpType & op) {
    StateGuard guard(s);
    if(expect(s, TokenKind::Less)) {
        op = AstCompareOpType::Less;
    }
    else if(expect(s, TokenKind::Greater)) {
        op = AstCompareOpType::More;
    }
    else if(expect(s, TokenKind::EqualEqual)) {
        op = AstCompareOpType::Equals;
    }
    else if(expect(s, TokenKind::GreaterEqual)) {
        op = AstCompareOpType::MoreEqual;
    }
    else if(expect(s, TokenKind::LessEqual)) {
        op = AstCompareOpType::LessEqual;
    }
    else if(expect(s, TokenKind::NotEqual)) {
        op = AstCompareOpType::NotEqual;
    }
    else if(expect(s, Token::KeywordIn)) {
        op = AstCompareOpType::In;
    }
    else if(expect(s, Token::KeywordNot) && expect(s, Token::KeywordIn)) {
        op = AstCompareOpType::NotIn;
    }
    else if(expect(s, Token::KeywordIs)) {
        op = AstCompareOpType::Is;
        if(expect(s, Token::KeywordNot)) {
           op = AstCompareOpType::IsNot;
        }
    }
    else {
        return false;
    }
    return guard.commit();
}

bool raise_stmt(State & s, AstStmt & ast) {
    // expect(s, Token::KeywordRaise) [test [expect(s, TokenKind::Comma) test [expect(s, TokenKind::Comma) test]]]
    if(!expect(s, Token::KeywordRaise)) {
        return false;
    }
    StateGuard guard(s, ast);
    AstRaisePtr raise;
    location(s, create(raise));
    ast = raise;

    if(test(s, raise->arg0)) {
        if(expect(s, TokenKind::Comma)) {
            if(!test(s, raise->arg1)) {
                syntax_error(s, ast, "Expected 2nd argument after comma");
                return false;
            }
            if(expect(s, TokenKind::Comma)) {
                if(!test(s, raise->arg2)) {
                    syntax_error(s, ast, "Expected 3rd argument after comma");
                    return false;
                }
            }
        }
    }

    return guard.commit();
}

bool atom(State & s, AstExpr & ast) {
    StateGuard guard(s);
    // expect(s, TokenKind::LeftParen) [yield_expr||testlist_comp] expect(s, TokenKind::RightParen)
    if(expect(s, TokenKind::LeftParen)) {
        // Either or, both optional
        if(testlist_comp(s, ast)) {
            if(ast->type != AstType::Generator) {
                AstTuplePtr ptr;
                location(s, create(ptr));
                if(ast->type == AstType::Expressions) {
                    AstExpressions & e = *static_cast<AstExpressions*>(ast.get());
                    for(auto item : e.items) {
                        ptr->elements.push_back(item);
                    }
                }
                else {
                    ptr->elements.push_back(ast);
                }
                ast = ptr;
            }
        }
        else if(!yield_expr(s, ast)) {
            AstTuplePtr ptr;
            location(s, create(ptr));
            ast = ptr;
        }
        if(!expect(s, TokenKind::RightParen)) {
            syntax_error(s, ast, "Expected `)`");
            return false;
        }
    }
    // ||expect(s, TokenKind::LeftBracket) [listmaker] expect(s, TokenKind::RightBracket)
    else if(expect(s, TokenKind::LeftBracket)) {
        if(!listmaker(s, ast)) {
            AstListPtr ptr;
            location(s, create(ptr));
            ast = ptr;
        }
        if(!expect(s, TokenKind::RightBracket)) {
            syntax_error(s, ast, "Expected `]`");
            return false;
        }
    }
    // ||expect(s, TokenKind::LeftBrace) [dictorsetmaker] expect(s, TokenKind::RightBrace)
    else if(expect(s, TokenKind::LeftBrace)) {
        if(!dictorsetmaker(s, ast)) {
            AstDictPtr ptr;
            location(s, create(ptr));
            ast = ptr;
        }
        if(!expect(s, TokenKind::RightBrace)) {
            syntax_error(s, ast, "Expected `}`");
            return false;
        }
    }
    // ||expect(s, TokenKind::BackQuote) testlist1 expect(s, TokenKind::BackQuote)
    else if(expect(s, TokenKind::BackQuote)) {
        AstReprPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        if(!testlist(s, ptr->value)) {
            return false;
        }
        if(!expect(s, TokenKind::BackQuote)) {
            syntax_error(s, ast, "Expected ``` (back quote)");
            return false;
        }
    }
    // ||expect(s, Token::Identifier)
    else if(get_name(s, ast)) {
        // OK
    }
    // || NUMBER
    else if(is(s, TokenKind::Number)) {
        AstNumberPtr ptr;
        if(!number(s, ptr)) {
            return false;
        }
        ast = ptr;
    }
    // || STRING+
    else if(is(s, Token::String)) {
        AstExpressionsPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        while(is(s, Token::String)) {
            AstStrPtr str;
            location(s, create(str));
            str->value = top(s).value;
            ptr->items.push_back(str);
            expect(s, Token::String);
        }
    }
    else if(is(s, Token::KeywordTrue) || is(s, Token::KeywordFalse)) {
        AstBoolPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->value = is(s, Token::KeywordTrue);
        expect(s, Token::KeywordTrue) || expect(s, Token::KeywordFalse);
    }
    else if(is(s, Token::KeywordNone)) {
        AstNonePtr ptr;
        location(s, create(ptr));
        ast = ptr;
        expect(s, Token::KeywordNone);
    }
    else {
        return false;
    }
    return guard.commit();
}

bool parameters(State & s, AstArguments & ast) {
    StateGuard guard(s);
    location(s, ast);
    // expect(s, TokenKind::LeftParen) [varargslist] expect(s, TokenKind::RightParen)
    if(!expect(s, TokenKind::LeftParen)) {
        return false;
    }

    varargslist(s, ast);

    if(!expect(s, TokenKind::RightParen)) {
        syntax_error(s, ast, "Expected `)`");
        return false;
    }
    return guard.commit();
}

bool dotted_as_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstAliasPtr alias;
    location(s, create(alias));
    ast = alias;
    // dotted_name [expect(s, Token::KeywordAs) expect(s, Token::Identifier)]
    if(!dotted_name(s, alias->name)) {
        return false;
    }
    if(expect(s, Token::KeywordAs)) {
        if(!get_name(s, alias->as_name)) {
            syntax_error(s, ast, "Expected identifier after `as`");
            return false;
        }
    }
    return guard.commit();
}

bool arglist(State & s, AstArguments & ast) {
    StateGuard guard(s);
    // location(s, create(ast));
    // (argument expect(s, TokenKind::Comma))* (argument [expect(s, TokenKind::Comma)]||expect(s, TokenKind::Star) test (expect(s, TokenKind::Comma) argument)* [expect(s, TokenKind::Comma) expect(s, TokenKind::DoubleStar) test]||expect(s, TokenKind::DoubleStar) test)
    AstExpr item;
    while(!(is(s, TokenKind::Star) || is(s, TokenKind::DoubleStar)) && argument(s, item)) {
        ast.arguments.push_back(item);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    expect(s, TokenKind::Comma);
    while(expect(s, TokenKind::Star)) {
        if(!test(s, item)) {
            syntax_error(s, ast, "Expected expression after `*`");
            return false;
        }
        ast.args.push_back(item);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    while(!is(s, TokenKind::DoubleStar) && argument(s, item)) {
        if(item->type != AstType::Keyword) {
            syntax_error(s, ast, "Expected keyword argument");
            return false;
        }
        ast.keywords.push_back(item);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    expect(s, TokenKind::Comma);
    if(expect(s, TokenKind::DoubleStar)) {
        if(!test(s, ast.kwargs)) {
            syntax_error(s, ast, "Expected expression after `**`");
            return false;
        }
    }
    return guard.commit();
}

bool single_input(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::NewLine) || simple_stmt || compound_stmt expect(s, Token::NewLine)
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
        syntax_error(s, ast, "Expected expression");
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
        exprs->items.push_back(tmp);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(exprs->items.size() == 1) {
        ast = exprs->items.front();
    }
    return !exprs->items.empty() && guard.commit();
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
    if(suite_->items.size() == 1) {
        ast = tmp;
    }
    if(!expect(s, TokenKind::NewLine)) {
        syntax_error(s, ast, "Expected new line after statement");
        return false;
    }
    while(expect(s, TokenKind::NewLine));
    return guard.commit();
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
        syntax_error(s, ast, "Expected expression after `exec`");
        return false;
    }
    if(expect(s, Token::KeywordIn)) {
        if(!test(s, exec->globals)) {
            syntax_error(s, ast, "Expected expression after `in`");
            return false;
        }
        if(expect(s, TokenKind::Comma)) {
            if(!test(s, exec->locals)) {
                syntax_error(s, ast, "Expected expression after `,`");
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
                syntax_error(s, ast, "Expected expression after `if`");
                return false;
            }
            if(!expect(s, Token::KeywordElse)) {
                syntax_error(s, ast, "Expected `else`");
                return false;
            }
            if(!test(s, ifexpr->orelse)) {
                syntax_error(s, ast, "Expected expression after `else`");
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
    AstGlobalPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordGlobal) expect(s, Token::Identifier) (expect(s, TokenKind::Comma) expect(s, Token::Identifier))*
    if(expect(s, Token::KeywordGlobal)) {
        AstExpr item;
        if(!get_name(s, item)) {
            syntax_error(s, ast, "Expected identifier after `global`");
            return false;
        }
        assert(item->type == AstType::Name);
        if(item->type != AstType::Name) {
            return false;
        }
        ptr->names.push_back(std::static_pointer_cast<AstName>(item));
        while(expect(s, TokenKind::Comma)) {
            if(!get_name(s, item)) {
                syntax_error(s, ast, "Expected identifier after `,`");
                return false;
            }
            assert(item->type == AstType::Name);
            if(item->type != AstType::Name) {
                syntax_error(s, ast, "Expected identifier after `,`");
                return false;
            }
            ptr->names.push_back(std::static_pointer_cast<AstName>(item));
        }
        return guard.commit();
    }
    return false;
}

bool subscript(State & s, AstSliceKindPtr & ast) {
    StateGuard guard(s, ast);
    // expect(s, TokenKind::Dot) expect(s, TokenKind::Dot) expect(s, TokenKind::Dot) || test || [test] expect(s, TokenKind::Colon) [test] [sliceop]
    if(is(s, TokenKind::Dot)) {
        AstEllipsisPtr ellipsis;
        location(s, create(ellipsis));
        if(!(expect(s, TokenKind::Dot) && expect(s, TokenKind::Dot) && expect(s, TokenKind::Dot))) {
            syntax_error(s, ast, "Invalid syntax");
            return false;
        }
        ast = ellipsis;
    }
    else {
        AstIndexPtr index;
        location(s, create(index));
        test(s, index->value);
        if(expect(s, TokenKind::Colon)) {
            AstSlicePtr slice;
            location(s, create(slice));
            slice->line = index->line;
            slice->column = index->column;
            slice->lower = index->value;
            test(s, slice->upper);
            sliceop(s, slice->step);
            ast = slice;
        }
        else {
            if(!index->value) {
                syntax_error(s, ast, "Invalid syntax");
                return false;
            }
            ast = index;
        }
    }
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
            syntax_error(s, ast, "Expected expression after `as`");
            return false;
        }
    }
    return guard.commit();
}

bool decorators(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstDecoratorsPtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // decorator+
    AstExpr temp;
    while(decorator(s, temp)) {
        exprs->decorators.push_back(temp);
    }
    return !exprs->decorators.empty() && guard.commit();
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
    return break_stmt(s, ast)
        || continue_stmt(s, ast)
        || return_stmt(s, ast)
        || raise_stmt(s, ast)
        || yield_stmt(s, ast)
        ;
}


bool yield_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstYieldExprPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    if(!expect(s, Token::KeywordYield)) {
        return false;
    }
    testlist(s, ptr->args);
    return guard.commit();
}

bool power(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // location(s, create(ast));
    // atom trailer* [expect(s, TokenKind::DoubleStar) factor]
    if(atom(s, ast)) {
        AstExpr expr;
        while(trailer(s, expr, ast)) {
            ast = expr;
        }
        if(expr) {
            ast = expr;
        }

        if(expect(s, TokenKind::DoubleStar)) {
            AstBinOpPtr ptr;
            location(s, create(ptr));
            ptr->left = ast;
            ast = ptr;
            ptr->op = AstBinOpType::Power;
            if(!factor(s, ptr->right)) {
                syntax_error(s, ast, "Expected expression after `**`");
                return false;
            }
        }
        return guard.commit();
    }
    return false;
}

bool print_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstPrintPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    ptr->newline = true;
    // 'print' ( [ test (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] ] ||expect(s, TokenKind::RightShift) test [ (expect(s, TokenKind::Comma) test)+ [expect(s, TokenKind::Comma)] ] )
    if(top(s).value != "print") {
        return false;
    }
    // Consume 'print'
    pop(s);

    if(expect(s, TokenKind::RightShift)) {
        if(!test(s, ptr->destination)) {
            syntax_error(s, ast, "Expected expression after `>>`");
            return false;
        }
        if(expect(s, TokenKind::Comma)) {
            AstExpr value;
            if(!test(s, value)) {
                syntax_error(s, ast, "Expected expression after `,`");
                return false;
            }
            else {
                ptr->values.push_back(value);
            }
        }
    }
    else {
        AstExpr value;
        if(test(s, value)) {
            ptr->values.push_back(value);
        }
    }
    while(expect(s, TokenKind::Comma)) {
        AstExpr value;
        if(!test(s, value)) {
            ptr->newline = false;
            break;
        }
        else {
            ptr->values.push_back(value);
        }
    }
    return guard.commit();
}

bool subscriptlist(State & s, AstExtSlice & ast) {
    StateGuard guard(s);
    location(s, ast);
    // subscript (expect(s, TokenKind::Comma) subscript)* [expect(s, TokenKind::Comma)]
    AstSliceKindPtr item;
    if(!subscript(s, item)) {
        return false;
    }
    ast.dims.push_back(item);
    while(expect(s, TokenKind::Comma)) {
        if(!subscript(s, item)) {
            break;
        }
        ast.dims.push_back(item);
    }
    return guard.commit();
}

bool testlist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstExpressionsPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // test (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)]
    AstExpr temp;
    if(!test(s, temp)) {
        return false;
    }
    ptr->items.push_back(temp);
    while(expect(s, TokenKind::Comma)) {
        if(!test(s, temp)) {
            break;
        }
        ptr->items.push_back(temp);
    }
    if(ptr->items.size() == 1) {
        ast = ptr->items.front();
        ptr.reset();
    }
    return guard.commit();
}

bool classdef(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstClassDefPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordClass) expect(s, Token::Identifier)
    // [expect(s, TokenKind::LeftParen) [testlist] expect(s, TokenKind::RightParen)]
    // expect(s, TokenKind::Colon) suite
    if(!expect(s, Token::KeywordClass)) {
        return false;
    }
    if(!get_name(s, ptr->name)) {
        syntax_error(s, ast, "Expected identifier after `class`");
        return false;
    }
    if(expect(s, TokenKind::LeftParen)) {
        testlist(s, ptr->bases);
        if(!expect(s, TokenKind::RightParen)) {
            syntax_error(s, ast, "Expected `)`");
            return false;
        }
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, ptr->body)) {
        return false;
    }
    return guard.commit();
}

bool stmt(State & s, AstStmt & ast) {
    return simple_stmt(s, ast)
        || compound_stmt(s, ast);
}

bool argument(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // test [comp_for] || test expect(s, TokenKind::Equal) test
    AstExpr first;
    if(!test(s, first)) {
        return false;
    }
    if(!is(s, TokenKind::Equal)) {
        AstComprPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->target = first;
        if(!comp_for(s, ptr->iter)) {
            ast = first;
        }
    }
    else {
        expect(s, TokenKind::Equal);
        AstKeywordPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->name = first;
        if(!test(s, ptr->value)) {
            syntax_error(s, ast, "Expected expression after `=`");
            return false;
        }
    }
    return guard.commit();
}

bool assert_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstAssertPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordAssert) test [expect(s, TokenKind::Comma) test]
    if(!expect(s, Token::KeywordAssert)) return false;
    if(!test(s, ptr->test)) {
        syntax_error(s, ast, "Expected expression after `assert`");
        return false;
    }
    if(expect(s, TokenKind::Comma)) {
        if(!test(s, ptr->expression)) {
            syntax_error(s, ast, "Expected expression after `,`");
            return false;
        }
    }
    return guard.commit();
}

bool for_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstForPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) testlist expect(s, TokenKind::Colon) suite [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    if(!expect(s, Token::KeywordFor)) {
        return false;
    }
    if(!exprlist(s, ptr->target)) {
        syntax_error(s, ast, "Expected one or more expressions after `for`");
        return false;
    }
    if(!expect(s, Token::KeywordIn)) {
        syntax_error(s, ast, "Expected `in`");
        return false;
    }
    if(!testlist(s, ptr->iter)) {
        syntax_error(s, ast, "Expected expression after `in`");
        return false;
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, ptr->body)) {
        return false;
    }
    if(expect(s, Token::KeywordElse)) {
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:` after `else`");
            return false;
        }
        if(!suite(s, ptr->orelse)) {
            return false;
        }
    }
    return guard.commit();
}

bool and_test(State & s, AstExpr & ast) {
    // not_test (expect(s, ) not_test)*
    return generic_boolop_expr(s, ast, Token::KeywordAnd, AstBoolOpType::And, not_test);
}

bool lambdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstLambdaPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordLambda) [varargslist] expect(s, TokenKind::Colon) test
    if(!expect(s, Token::KeywordLambda)) {
        return false;
    }
    varargslist(s, ptr->arguments);
    if(! expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!test(s, ptr->body)) {
        syntax_error(s, ast, "Expected expression");
        return false;
    }
    return guard.commit();
}

bool suite(State & s, AstStmt & ast) {
    // simple_stmt || expect(s, Token::NewLine) expect(s, Token::Indent) stmt+ expect(s, Token::Dedent)
    if(expect(s, Token::NewLine)) {
        StateGuard guard(s, ast);
        AstSuitePtr suite_;
        location(s, create(suite_));
        ast = suite_;
        // Consume any new lines inbetween
        while(expect(s, Token::NewLine));
        if(expect(s, Token::Indent)) {
            AstStmt stmt_;
            if(stmt(s, stmt_)) {
                suite_->items.push_back(stmt_);
                stmt_.reset();
                while(stmt(s, stmt_)) {
                    suite_->items.push_back(stmt_);
                    stmt_.reset();
                }
                if(!expect(s, Token::Dedent)) {
                    indentation_error(s, ast);
                    return false;
                }
                return guard.commit();
            }
        }
    }
    if(simple_stmt(s, ast)) {
        return true;
    }
    return false;
}

bool funcdef(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstFunctionDefPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordDef) expect(s, Token::Identifier) parameters expect(s, TokenKind::Colon) suite
    if(!expect(s, Token::KeywordDef)) {
        return false;
    }
    if(!get_name(s, ptr->name)) {
        syntax_error(s, ast, "Expected identifier after `def`");
        return false;
    }
    if(!parameters(s, ptr->args)) {
        syntax_error(s, ast, "Expected parameter declaration");
        return false;
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, ptr->body)) {
        return false;
    }
    return guard.commit();
}

bool decorated(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstDecoratedPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // decorators (classdef || funcdef)
    if(!decorators(s, ptr->decorators)) {
        return false;
    }
    if(!classdef(s, ptr->cls_or_fun_def) && !funcdef(s, ptr->cls_or_fun_def)) {
        syntax_error(s, ast, "Expected `class` or `def` after decorator usage");
        return false;
    }
    return guard.commit();
}

bool expr_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s);
    AstExpressionStatementPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    AstExpr target;
    if(testlist(s, target)) {
        ptr->expr = target;
        AstBinOpType op{};
        if(augassign(s, op)) {
            AstAugAssignPtr ptr;
            location(s, create(ptr));
            ast = ptr;
            ptr->target = target;
            ptr->op = op;
            if(!yield_expr(s, ptr->value) && !testlist(s, ptr->value)) {
                syntax_error(s, ast, "Expected expression after augmented assignment operator");
                return false;
            }
        }
        else {
            if(is(s, TokenKind::Equal)) {
                AstAssignPtr ptr;
                location(s, create(ptr));
                ptr->targets = target;
                ast = ptr;
                while(expect(s, TokenKind::Equal)) {
                    AstExpr value;
                    if(!yield_expr(s, value) && !testlist(s, value)) {
                        syntax_error(s, ast, "Expected expression after assignment operator");
                        return false;
                    }
                    ptr->value.items.push_back(value);
                }
            }
        }
    }
    else {
        return false;
    }
    // testlist (augassign (yield_expr||testlist)
    // ||(expect(s, TokenKind::Equal) (yield_expr||testlist))*)
    return guard.commit();
}

bool old_lambdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstLambdaPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordLambda) [varargslist] expect(s, TokenKind::Colon) old_test
    if(!expect(s, Token::KeywordLambda)) {
        return false;
    }
    varargslist(s, ptr->arguments);
    if(! expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!old_test(s, ptr->body)) {
        syntax_error(s, ast, "Expected expression");
        return false;
    }
    return guard.commit();
}

bool continue_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstContinuePtr ptr;
    location(s, create(ptr));
    ast = ptr;
    if(!expect(s, Token::KeywordContinue)) {
        return false;
    }
    return guard.commit();
}

bool decorator(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstDecoratorPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, TokenKind::At) dotted_name
    // [ expect(s, TokenKind::LeftParen) [arglist] expect(s, TokenKind::RightParen) ] expect(s, Token::NewLine)
    if(!expect(s, TokenKind::At)) {
        return false;
    }
    if(!dotted_as_name(s, ast)) {
        syntax_error(s, ast, "Expected identifier after `@`");
        return false;
    }
    if(expect(s, TokenKind::LeftParen)) {
        arglist(s, ptr->arguments);
        if(!expect(s, TokenKind::RightParen)) {
            syntax_error(s, ast, "Expected `)`");
            return false;
        }
    }
    if(!expect(s, Token::NewLine)) {
        syntax_error(s, ast, "Expected new line after decorator usage");
        return false;
    }
    return guard.commit();
}

bool if_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstIfPtr if_;
    location(s, create(if_));
    ast = if_;
    // expect(s, Token::KeywordIf) test expect(s, TokenKind::Colon) suite
    if(!expect(s, Token::KeywordIf)) {
        return false;
    }
    if(!test(s, if_->test)) {
        syntax_error(s, ast, "Expected expression after `if`");
        return false;
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, if_->body)) {
        return false;
    }
    // (expect(s, Token::KeywordElIf) test expect(s, TokenKind::Colon) suite)*
    if(is(s, Token::KeywordElIf)) {
        AstExpressionsPtr lst;
        location(s, create(lst));
        while(expect(s, Token::KeywordElIf)) {
            AstElseIfPtr elif;
            location(s, create(elif));
            if(!test(s, elif->test)) {
                syntax_error(s, ast, "Expected expression after `elif`");
                return false;
            }
            if(!expect(s, TokenKind::Colon)) {
                syntax_error(s, ast, "Expected `:`");
                return false;
            }
            if(!suite(s, elif->body)) {
                return false;
            }
            lst->items.push_back(elif);
        }
        if_->elif = lst;
    }
    // [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    if(expect(s, Token::KeywordElse)) {
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:` after `else`");
            return false;
        }
        if(!suite(s, if_->orelse)) {
            return false;
        }
    }
    return guard.commit();
}

bool sliceop(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expect(s, TokenKind::Colon) [test]
    if(!expect(s, TokenKind::Colon)) {
        return false;
    }
    test(s, ast);
    return guard.commit();
}

bool comparison(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expr (comp_op expr)*
    if(!expr(s, ast)) {
        return false;
    }
    AstCompareOpType op;
    while(comp_op(s, op)) {
        AstComparePtr ptr;
        location(s, create(ptr));
        ptr->left = ast;
        ast = ptr;
        ptr->op = op;
        if(!expr(s, ptr->right)) {
            syntax_error(s, ast, "Expected expression after comparison operator");
            return false;
        }
    }
    return guard.commit();
}

bool term(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // factor (( factor)*
    if(factor(s, ast)) {
        TokenKind k = kind(top(s));
        while(expect(s, TokenKind::Star) || expect(s, TokenKind::Slash) || expect(s, TokenKind::Percent) || expect(s, TokenKind::DoubleSlash)) {
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
                syntax_error(s, ast, "Expected expression after operator");
                return false;
            }
        }
        return guard.commit();
    }
    return false;
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
    return generic_binop_expr(s, ast, TokenKind::CircumFlex, AstBinOpType::BitXor, and_expr);
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
    AstExpr first, second;
    // ((test expect(s, TokenKind::Colon) test (comp_for || (expect(s, TokenKind::Comma) test expect(s, TokenKind::Colon) test)* [expect(s, TokenKind::Comma)])) ||(test (comp_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)])))
    if(test(s, first)) {
        if(expect(s, TokenKind::Colon)) {
            // Dict
            AstDictPtr ptr;
            location(s, create(ptr));
            ast = ptr;
            if(!test(s, second)) {
                syntax_error(s, ast, "Expected expression after `:`");
                return false;
            }

            ptr->keys.push_back(first);
            ptr->values.push_back(second);

            if(is(s, Token::KeywordFor)) {
                ptr.reset();
                AstDictCompPtr comp;
                location(s, create(comp));
                ast = comp;
                comp->key = first;
                comp->value = second;
                comp_for(s, comp->generators);
                // Dict Comprehension
            }
            else if(is(s, TokenKind::Comma)) {
                first.reset();
                second.reset();
                // Dict definition
                while(expect(s, TokenKind::Comma)) {
                    if(!test(s, first)) {
                        break;
                    }
                    if(!expect(s, TokenKind::Colon)) {
                        syntax_error(s, ast, "Expected `:`");
                        return false;
                    }
                    if(!test(s, second)) {
                        syntax_error(s, ast, "Expected expression after `:`");
                        return false;
                    }
                    ptr->keys.push_back(first);
                    ptr->values.push_back(second);
                    first.reset();
                    second.reset();
                }
            }
            else {
                // OK, only one
            }
        }
        else {
            // Set
            if(is(s, Token::KeywordFor)) {
                // Set Comprehension
                AstSetCompPtr ptr;
                location(s, create(ptr));
                ast = ptr;
                ptr->element = first;
                if(!comp_for(s, ptr->generators)) {
                    return false;
                }
                // OK
            }
            else {
                // Set definition
                AstSetPtr ptr;
                location(s, create(ptr));
                ast = ptr;
                ptr->elements.push_back(first);
                while(expect(s, TokenKind::Comma)) {
                    if(!test(s, first)) {
                        syntax_error(s, ast, "Expected expression after `,`");
                        break;
                    }
                    ptr->elements.push_back(first);
                }
                // OK
            }
        }
    } else {
        // Empty Dict
        AstDictPtr ptr;
        location(s, create(ptr));
        ast = ptr;
    }
    return guard.commit();
}

bool expr(State & s, AstExpr & ast) {
    return generic_binop_expr(s, ast, TokenKind::BinOr, AstBinOpType::BitOr, xor_expr);
}

bool del_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstDeletePtr del;
    location(s, create(del));
    ast = del;
    // expect(s, Token::KeywordDel) exprlist
    if(!expect(s, Token::KeywordDel)) {
        return false;
    }
    if(!exprlist(s, del->targets)) {
        syntax_error(s, ast, "Expected expression(s) after `del`");
        return false;
    }
    return guard.commit();
}

bool while_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstWhilePtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordWhile) test expect(s, TokenKind::Colon) suite [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    if(!expect(s, Token::KeywordWhile)) {
        return false;
    }
    if(!test(s, ptr->test)) {
        syntax_error(s, ast, "Expected condition after `while`");
        return false;
    }
    if(!expect(s, TokenKind::Colon)) {
        syntax_error(s, ast, "Expected `:`");
        return false;
    }
    if(!suite(s, ptr->body)) {
        return false;
    }
    if(expect(s, Token::KeywordElse)) {
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:`");
            return false;
        }
        if(!suite(s, ptr->orelse)) {
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

bool fpdef(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    // expect(s, Token::Identifier) || expect(s, TokenKind::LeftParen) fplist expect(s, TokenKind::RightParen)
    if(!get_name(s, ast)) {
        if(expect(s, TokenKind::LeftParen)) {
            if(!fplist(s, ast)) {
                syntax_error(s, ast, "Expected parameter list");
                return false;
            }
            if(!expect(s, TokenKind::RightParen)) {
                syntax_error(s, ast, "Expected `)`");
                return false;
            }
        }
        else {
            return false;
        }
    }
    return guard.commit();
}

bool varargslist(State & s, AstArguments & ast) {
    StateGuard guard(s);
    location(s, ast);
    // ((fpdef [expect(s, TokenKind::Equal) test] expect(s, TokenKind::Comma))* (expect(s, TokenKind::Star) expect(s, Token::Identifier) [expect(s, TokenKind::Comma) expect(s, TokenKind::DoubleStar) expect(s, Token::Identifier)] || expect(s, TokenKind::DoubleStar) expect(s, Token::Identifier))
    //   ||fpdef [expect(s, TokenKind::Equal) test] (expect(s, TokenKind::Comma) fpdef [expect(s, TokenKind::Equal) test])* [expect(s, TokenKind::Comma)])

    // args, args=default, *args, **args
    while(!is(s, TokenKind::DoubleStar) && !is(s, TokenKind::Star)) {
        AstExpr arg;
        if(!fpdef(s, arg)) {
            break;
        }
        ast.arguments.push_back(arg);
        if(expect(s, TokenKind::Equal)) {
            if(!test(s, arg)) {
                syntax_error(s, ast, "Expected expression after `=`");
                return false;
            }
            ast.defaults.push_back(arg);
        }
        else {
            ast.defaults.push_back({});
        }
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(expect(s, TokenKind::Star)) {
        AstExpr arg;
        if(!get_name(s, arg)) {
            syntax_error(s, ast, "Expected identifier after `*`");
            return false;
        }
        ast.args.push_back(arg);
        expect(s, TokenKind::Comma);
    }

    if(expect(s, TokenKind::DoubleStar)) {
        if(!get_name(s, ast.kwargs)) {
            syntax_error(s, ast, "Expected identifier after `**`");
            return false;
        }
    }
    return guard.commit();
}

bool trailer(State & s, AstExpr & ast, AstExpr target) {
    StateGuard guard(s, ast);
    // expect(s, TokenKind::LeftParen) [arglist] expect(s, TokenKind::RightParen)
    if(is(s, TokenKind::LeftParen)) {
        AstCallPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->function = target;
        expect(s, TokenKind::LeftParen);
        arglist(s, ptr->arglist);
        if(!expect(s, TokenKind::RightParen)) {
            syntax_error(s, ast, "Expected `)`");
            return false;
        }
    }
    // || expect(s, TokenKind::LeftBracket) subscriptlist expect(s, TokenKind::RightBracket)
    else if(is(s, TokenKind::LeftBracket)) {
        AstSubscriptPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->value = target;
        expect(s, TokenKind::LeftBracket);
        AstExtSlicePtr slice_ptr;
        create(slice_ptr);
        ptr->slice = slice_ptr;
        if(!subscriptlist(s, *slice_ptr)) {
            syntax_error(s, ast, "Expected expression within `[]`");
            return false;
        }
        if(slice_ptr->dims.size() == 1) {
            ptr->slice = slice_ptr->dims.front();
        }
        if(!expect(s, TokenKind::RightBracket)) {
            syntax_error(s, ast, "Expected `]`");
            return false;
        }
    }
    // || expect(s, TokenKind::Dot) expect(s, Token::Identifier)
    else if(is(s, TokenKind::Dot)) {
        AstAttributePtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->value = target;
        expect(s, TokenKind::Dot);
        if(!get_name(s, ptr->attribute)) {
            syntax_error(s, ast, "Expected identifier after `.`");
            return false;
        }
    }
    else {
        return false;
    }

    return guard.commit();
}

bool import_stmt(State & s, AstStmt & ast) {
    // import_name || import_from
    return import_name(s, ast)
        || import_from(s, ast);
}

bool eval_input(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstExpressionStatementPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // testlist expect(s, Token::NewLine)* expect(s, Token::End)
    if(!testlist(s, ptr->expr)) {
        return false;
    }
    while(expect(s, Token::NewLine)) {
        // Nothing to be done, we just consume all NewLines
        // until there's none anymore
    }
    if(!expect(s, Token::End)) {
        syntax_error(s, ast, "Expected end of input");
        return false;
    }
    return guard.commit();
}

bool arith_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // term ((expect(s, TokenKind::Plus)||expect(s, TokenKind::Minus)) term)*
    if(!term(s, ast)) {
        return false;
    }
    if(is(s, TokenKind::Plus) || is(s, TokenKind::Minus)) {
        AstBinOpPtr ptr;
        location(s, create(ptr));
        ptr->left = ast;
        if(expect(s, TokenKind::Plus)) {
            ptr->op = AstBinOpType::Add;
        }
        else if(expect(s, TokenKind::Minus)) {
            ptr->op = AstBinOpType::Sub;
        }
        if(!arith_expr(s, ptr->right)) {
            syntax_error(s, ast, "Expected expression after operator");
            return false;
        }
    }
    return guard.commit();
}

bool list_iter(State & s, AstExpr & ast) {
    return list_for(s, ast)
        || list_if(s, ast);
}

bool list_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstComprPtr comp;
    location(s, create(comp));
    ast = comp;
    // expect(s, Token::KeywordIf) old_test [list_iter]
    if(!expect(s, Token::KeywordIf)) {
        return false;
    }
    if(!old_test(s, comp->target)) {
        syntax_error(s, ast, "Expected condition after `if`");
        return false;
    }
    list_iter(s, comp->iter);
    return guard.commit();
}

bool list_for(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstForExprPtr forexpr;
    location(s, create(forexpr));
    ast = forexpr;
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) testlist_safe [list_iter]
    if(!expect(s, Token::KeywordFor)) {
        return false;
    }
    if(!exprlist(s, forexpr->items)) {
        syntax_error(s, ast, "Expected expression after `for`");
        return false;
    }
    if(!expect(s, Token::KeywordIn)) {
        syntax_error(s, ast, "Expected `in`");
        return false;
    }
    if(!testlist_safe(s, forexpr->generators)) {
        syntax_error(s, ast, "Expected expression after `in`");
        return false;
    }
    list_iter(s, forexpr->iter);
    return guard.commit();
}

bool comp_for(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstForExprPtr forexpr;
    location(s, create(forexpr));
    ast = forexpr;
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) or_test [comp_iter]
    if(!expect(s, Token::KeywordFor)) {
        return false;
    }
    if(!exprlist(s, forexpr->items)) {
        syntax_error(s, ast, "Expected expression after `for`");
        return false;
    }
    if(!expect(s, Token::KeywordIn)) {
        syntax_error(s, ast, "Expected `in`");
        return false;
    }
    if(!or_test(s, forexpr->generators)) {
        syntax_error(s, ast, "Expected expression after `in`");
        return false;
    }
    comp_iter(s, forexpr->iter);
    return guard.commit();
}

bool comp_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstComprPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, Token::KeywordIf) old_test [comp_iter]
    if(!expect(s, Token::KeywordIf)) {
        return false;
    }
    if(!old_test(s, ptr->target)) {
        syntax_error(s, ast, "Expected condition after `if`");
        return false;
    }
    comp_iter(s, ptr->iter);
    return guard.commit();
}

bool comp_iter(State & s, AstExpr & ast) {
    return comp_for(s, ast)
        || comp_if(s, ast);
}

bool yield_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstYieldPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    if(!yield_expr(s, ptr->yield)) {
        return false;
    }
    return guard.commit();
}


bool Parser::parse(Lexer & lexer, AstModulePtr & ast) {
    State state{&lexer, {}, {}, {}, lexer.next()};
    return file_input(state, ast);
}

}
