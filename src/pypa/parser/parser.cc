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
#include <gmp.h>
#include <pypa/parser/apply.hh>
#include <pypa/parser/parser_fwd.hh>
#include <double-conversion/src/double-conversion.h>
#include <pypa/ast/context_assign.hh>
#include <cassert>

namespace pypa {

String make_string(String const & input, bool & unicode, bool & raw);

template< typename Container >
void flatten(AstStmt s, Container & target) {
    for(auto e : std::static_pointer_cast<AstSuite>(s)->items) {
        if(e && e->type == AstType::Suite) {
            flatten(e, target);
        }
        else {
            target.push_back(e);
        }
    }
}

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
    if(s.options.error_handler) {
        s.options.error_handler(e);
    }

    if(!s.options.printerrors) {
        return;
    }
    unsigned long long cur_line = e.cur.line;
    fprintf(stderr, "  File \"%s\", line %llu\n    %s\n    ", e.file_name.c_str(), cur_line, e.line.c_str());
    if(e.cur.column == 0) ++e.cur.column;
    for(unsigned i = 0; i < e.cur.column - 1; ++i) {
        fputc(' ', stderr);
    }
    fprintf(stderr, "^\n%s: %s", e.type == ErrorType::SyntaxError ? "SyntaxError" : e.type == ErrorType::SyntaxWarning ? "SyntaxWarning" : "IndentationError", e.message.c_str());
    if(e.detected_line != -1 && s.options.printdbgerrors) {
        fprintf(stderr, "\n-> Reported @%s:%d in %s\n\n", e.detected_file, e.detected_line, e.detected_function);
    }
}

#if 0
void add_symbol_error(State & s, char const * message, int line, int column, int reported_line, char const * reported_file_name, char const * reported_function) {
    TokenInfo ti = s.tok_cur;
    ti.line = line;
    ti.column = column;
    s.errors.push({ErrorType::SyntaxError, message, s.lexer->get_name(), ti, {}, s.lexer->get_line(line), reported_line, reported_file_name, reported_function });
    report_error(s);
}
#endif

void syntax_error_dbg(State & s, AstPtr ast, char const * message, int line = -1, char const * file = 0, char const * function = 0) {
    TokenInfo cur = top(s);
    if(ast && cur.line < ast->line) {
        cur.line    = ast->line;
        cur.column  = ast->column;
    }
    s.errors.push({ErrorType::SyntaxError, message, s.lexer->get_name(), cur, ast, s.lexer->get_line(cur.line), line, file ? file : "", function ? function : ""});
    report_error(s);
}

void indentation_error_dbg(State & s, AstPtr ast, int line = -1, char const * file = 0, char const * function = 0) {
    TokenInfo cur = top(s);
    if(ast && cur.line > ast->line) {
        cur.line    = ast->line;
        cur.column  = ast->column;
    }
    s.errors.push({ErrorType::IndentationError, "", s.lexer->get_name(), cur, ast, s.lexer->get_line(cur.line), line, file ? file : "", function ? function : ""});
    report_error(s);
}

#define syntax_error(s, AST_ITEM, msg) syntax_error_dbg(s, error_transform(AST_ITEM), msg, __LINE__, __FILE__, __PRETTY_FUNCTION__)
#define indentation_error(s, AST_ITEM) indentation_error_dbg(s, error_transform(AST_ITEM), __LINE__, __FILE__, __PRETTY_FUNCTION__)

bool number_from_base(int64_t base, State & s, AstNumberPtr & ast) {
    String const & value = top(s).value;
    AstNumber & result = *ast;

    MP_INT integ;
    mpz_init_set_str(&integ, value.c_str(), base);

    result.num_type = AstNumber::Integer;
    pop(s);
    bool long_post_fix = (top(s).value == "L" || top(s).value == "l");
    if(!long_post_fix) {
        unpop(s);
    }
    if(long_post_fix || !mpz_fits_slong_p(&integ)) {
        result.num_type = AstNumber::Long;
    }

    if(result.num_type == AstNumber::Long) {
        result.str.resize(mpz_sizeinbase(&integ, 10) + 2, 0);
        mpz_get_str(&result.str[0], 10, &integ);
    }
    else {
        result.integer = mpz_get_si(&integ);
    }
    mpz_clear(&integ);
    return true;
}

bool string_to_double(String const & s, double & result) {
    double_conversion::StringToDoubleConverter conv(0, 0.0, 0.0, 0, 0);
    int length = int(s.size());
    int processed = 0;
    result = conv.StringToDouble(s.c_str(), length, &processed);
    return length == processed;
}

bool number(State & s, AstNumberPtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    int base = 0;
    if(is(s, Token::NumberFloat)) {
        String const & dstr = top(s).value;
        double result = 0;
        if(!string_to_double(dstr, result)) {
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
    AstTuplePtr lst;
    location(s, create(lst));
    ast = lst;
    AstExpr dotted;
    while(dotted_as_name(s, dotted)) {
        lst->elements.push_back(dotted);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(lst->elements.empty()) {
        return false;
    }
    if(lst->elements.size() == 1) {
        ast = lst->elements.front();
    }
    return  guard.commit();
}

bool import_as_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstAliasPtr alias;
    location(s, create(alias));
    ast = alias;
    if(get_name(s, alias->name))
    {
        if(expect(s, Token::KeywordAs)) {
            if(!get_name(s, alias->as_name)) {
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
        if(try_except->handlers.empty()
           && !try_except->orelse
           && try_except->body
           && try_except->body->type == AstType::Suite) {
            ptr->body = try_except->body;
        }
        else {
            ptr->body = try_except;
        }
        if(!suite(s, ptr->final_body)) {
            return false;
        }
    }
    return guard.commit();
}

bool dotted_name_list(State & s, AstExpr & ast) {
    std::vector<AstExpr> names;
    do {
        AstExpr name;
        if(!get_name(s, name)) {
            if(!names.empty()) {
                syntax_error(s, names.back(), "Expected identifier after `.`");
                return false;
            }
            break;
        }
        assert(name && name->type == AstType::Name);
        visit(context_assign{AstContext::Load}, name);
        names.push_back(std::move(name));
    } while(expect(s, TokenKind::Dot));
    if(names.empty()) {
        return false;
    }
    ast = names[0];
    for(auto it = names.begin() + 1; it != names.end(); ++it) {
        AstAttributePtr attr;
        location(s, create(attr));
        attr->attribute = *it;
        attr->value = ast;
        ast = attr;
    }
    return true;
}

bool dotted_name(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    if(get_name(s, ast)) {
        if(expect(s, TokenKind::Dot)) {
            assert(ast && ast->type == AstType::Name);
            AstNamePtr name = std::static_pointer_cast<AstName>(ast);
            AstExpr trailing_name;
            if(dotted_name(s, trailing_name)) {
                assert(trailing_name && trailing_name->type == AstType::Name);
                AstName & trail = *std::static_pointer_cast<AstName>(trailing_name);
                name->id += "." + trail.id;
                name->dotted = true;
                return guard.commit();
            }
            else {
                syntax_error(s, name, "Expected identifier after `.`");
            }
        }
        else {
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
    StateGuard guard(s, ast);
    AstUnaryOpPtr result;
    location(s, create(result));
    // expect(s, Token::KeywordNot) not_test || comparison
    if(expect(s, Token::KeywordNot)) {
        result->op = AstUnaryOpType::Not;
        if(!not_test(s, result->operand)) {
            return false;
        }
        ast = result;
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
        AstTuplePtr exprs;
        clone_location(ast, create(exprs));
        ast = exprs;
        while(expect(s, TokenKind::Comma)) {
            AstExpr temp;
            if(!test(s, temp)) {
                return false;
            }
            exprs->elements.push_back(temp);
        }
        if(exprs->elements.size() == 1) {
            ast = exprs->elements.front();
        }
    }
    return guard.commit();
}

bool testlist_safe(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstTuplePtr exprs;
    location(s, create(exprs));
    ast = exprs;
    AstExpr temp;
    // old_test [(expect(s, TokenKind::Comma) old_test)+ [expect(s, TokenKind::Comma)]]
    while(old_test(s, temp)) {
        exprs->elements.push_back(temp);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(exprs->elements.size() == 1) {
        ast = exprs->elements.front();
        return guard.commit();
    }
    return !exprs->elements.empty() && guard.commit();
}

bool testlist_comp(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstTuplePtr exprs;
    location(s, create(exprs));
    ast = exprs;
    AstExpr tmp;
    // test ( comp_for || (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)] )
    if(!test(s, tmp)) {
        return false;
    }
    if(is(s, Token::KeywordFor)) {
        AstGeneratorPtr gener;
        location(s, create(gener));
        ast = gener;
        gener->element = tmp;
        if(!comp_for(s, gener->generators)) {
            return false;
        }
    }
    else {
        bool last_was_comma = false;
        exprs->elements.push_back(tmp);
        for(;;) {
            if(!expect(s, TokenKind::Comma)) {
                break;
            }
            if(!test(s, tmp)) {
                last_was_comma = true;
                break;
            }
            exprs->elements.push_back(tmp);
        }

        if(!last_was_comma && exprs->elements.size() == 1) {
            ast = exprs->elements.front();
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
            visit(context_assign{AstContext::Store}, except->name);
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

bool with_stmt(State & s, AstStmt & ast, bool is_inner) {
    StateGuard guard(s, ast);
    AstWithPtr with;
    location(s, create(with));
    ast = with;
    // expect(s, Token::KeywordWith) with_item (expect(s, TokenKind::Comma) with_item)*  expect(s, TokenKind::Colon) suite
    if(!is_inner && !expect(s, Token::KeywordWith)) {
        return false;
    }

    // test [expect(s, Token::KeywordAs) expr]
    if(!test(s, with->context)) {
        return false;
    }
    if(expect(s, Token::KeywordAs)) {
        if(!expr(s, with->optional)) {
            syntax_error(s, ast, "Expected expression after `as`");
            return false;
        }
        visit(context_assign{AstContext::Store}, with->optional);
    }

    if(expect(s, TokenKind::Comma)) {
        if(!with_stmt(s, with->body, true)) {
            return false;
        }
    }
    else {
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:`");
            return false;
        }
        if(!suite(s, with->body)) {
            return false;
        }
    }
    return guard.commit();
}

bool with_stmt(State & s, AstStmt & ast) {
    return with_stmt(s, ast, false);
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
        if(!listmaker(s, ast) || !ast) {
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
        if(!testlist1(s, ptr->value)) {
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
        if(is(s, Token::NumberComplex)) {
            AstComplexPtr ptr;
            location(s, create(ptr));
            ast = ptr;
            if(!consume_value(s, Token::NumberComplex, ptr->imag)) {
                assert("This should not happen at this point" && false);
                return false;
            }
            if(!ptr->imag.empty() && ptr->imag.back() == 'j') {
                ptr->imag.pop_back();
            }
        }
        else {
            AstNumberPtr ptr;
            if(!number(s, ptr)) {
                return false;
            }
            ast = ptr;
            if(is(s, TokenKind::Plus) || is(s, Token::NumberComplex)) {
                bool plus = false;
                if(expect(s, TokenKind::Plus)) {
                    plus = true;
                }
                if(!is(s, Token::NumberComplex)) {
                    unpop(s);
                }
                else {
                    AstComplexPtr cplx;
                    location(s, create(cplx));
                    ast = cplx;
                    cplx->real = ptr;
                    if(!consume_value(s, Token::NumberComplex, cplx->imag)) {
                        assert("This should not happen at this point" && false);
                        return false;
                    }
                    if(plus) {
                        cplx->imag = "+" + cplx->imag;
                    }
                    ast = cplx;
                }
            }
        }
    }
    // || STRING+
    else if(is(s, Token::String)) {
        AstStrPtr str;
        location(s, create(str));
        ast = str;
        str->unicode = s.future_features.unicode_literals;
        while(is(s, Token::String)) {
            bool raw_string = false;
            String value = make_string(top(s).value, str->unicode, raw_string);
            if (str->unicode) {
                bool error = false;
                assert(s.options.unicode_escape_handler && "unicode_escape_handler not set!");
                value = s.options.unicode_escape_handler(value, raw_string, error);
                if (error)
                    syntax_error(s, ast, value.c_str());
            }
            str->value.append(value);
            expect(s, Token::String);
        }
    }
    /*
    else if(is(s, Token::KeywordTrue) || is(s, Token::KeywordFalse)) {
        AstBoolPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->value = is(s, Token::KeywordTrue);
        expect(s, Token::KeywordTrue) || expect(s, Token::KeywordFalse);
    }*/
    /*else if(is(s, Token::KeywordNone)) {
        AstNonePtr ptr;
        location(s, create(ptr));
        ast = ptr;
        expect(s, Token::KeywordNone);
    }*/
    else if((s.options.python3allowed || s.options.python3only) && is(s, TokenKind::Dot)) {
        expect(s, TokenKind::Dot);
        if(expect(s, TokenKind::Dot)) {
            if(expect(s, TokenKind::Dot)) {
                AstEllipsisObjectPtr ptr;
                location(s, create(ptr));
                ast = ptr;
            }
            else {
                unpop(s); unpop(s);
                return false;
            }
        }
        else {
            unpop(s);
            return false;
        }
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

    visit(context_assign{AstContext::Param}, ast.args);
    for(auto & a : ast.arguments) {
        visit(context_assign{AstContext::Param}, a);
    }
    visit(context_assign{AstContext::Param}, ast.kwargs);

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
        if(item->type != AstType::Keyword) {
            if(!ast.keywords.empty()) {
                syntax_error(s, ast, "Expected keyword argument");
                return false;
            }
            ast.arguments.push_back(item);
        }
        else {
            ast.keywords.push_back(item);
        }
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    expect(s, TokenKind::Comma);
    while(expect(s, TokenKind::Star)) {
        if(!test(s, ast.args)) {
            syntax_error(s, ast, "Expected expression after `*`");
            return false;
        }
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

    for(auto & d : ast.keywords) {
        // Even though we're saying store, it'll set Load for the value
        // and Store for the key
        visit(context_assign{AstContext::Store}, d);
    }

    return guard.commit();
}

bool shift_expr(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // arith_expr ((expect(s, TokenKind::LeftShift)||expect(s, TokenKind::RightShift)) arith_expr)*
    if(!arith_expr(s, ast)) {
        return false;
    }
    while(is(s, TokenKind::LeftShift) || is(s, TokenKind::RightShift)) {
        AstBinOpPtr bin;
        location(s, create(bin));
        bin->left = ast;
        ast = bin;
        if(expect(s, TokenKind::LeftShift)) {
            bin->op = AstBinOpType::LeftShift;
        }
        else if(expect(s, TokenKind::RightShift)) {
            bin->op = AstBinOpType::RightShift;
        }
        if(!arith_expr(s, bin->right)){
            syntax_error(s, ast, "Expected expression");
            return false;
        }
    }
    return guard.commit();
}

bool exprlist(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstTuplePtr exprs;
    location(s, create(exprs));
    exprs->context = AstContext::Store;
    ast = exprs;
    // expr (expect(s, TokenKind::Comma) expr)* [expect(s, TokenKind::Comma)]
    AstExpr tmp;
    while(expr(s, tmp)) {
        exprs->elements.push_back(tmp);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(exprs->elements.size() == 1) {
        ast = exprs->elements.front();
    }
    return !exprs->elements.empty() && guard.commit();
}

bool simple_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstSuitePtr suite_;
    location(s, create(suite_));
    ast = suite_;
    // small_stmt (expect(s, TokenKind::SemiColon) small_stmt)* [expect(s, TokenKind::SemiColon)] expect(s, Token::NewLine)
    AstStmt tmp;
    if(!small_stmt(s, tmp)) {
        return false;
    }
    if(tmp && tmp->type == AstType::Suite) {
       flatten(tmp, suite_->items);
    }
    else {
       suite_->items.push_back(tmp);
    }
    if(expect(s, TokenKind::SemiColon)) {
        while(small_stmt(s, tmp)) {
            if(tmp && tmp->type == AstType::Suite) {
               flatten(tmp, suite_->items);
            }
            else {
               suite_->items.push_back(tmp);
            }
            if(!expect(s, TokenKind::SemiColon)) {
                break;
            }
        }
    }
    if(suite_->items.size() == 1) {
        ast = suite_->items.front();
    }
    if(is(s, Token::End)) {
        return guard.commit();
    }

    if(!expect(s, TokenKind::NewLine)/* && !is(s, Token::End)*/) {
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
    ast = exec;
    // expect(s, Token::KeywordExec) expr [expect(s, Token::KeywordIn) test [expect(s, TokenKind::Comma) test]]
    if(!expect(s, Token::KeywordExec)) {
        return false;
    }
    if(!expr(s, exec->body)) {
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
        if(factor(s, unary->operand)) {
            // Translating (-1) immediately to -1
            if(unary->operand && unary->op == AstUnaryOpType::Sub) {
                if(unary->operand->type == AstType::Number) {
                    AstNumberPtr p = std::static_pointer_cast<AstNumber>(unary->operand);
                    switch(p->num_type) {
                    case AstNumber::Float:
                        p->floating *= -1.;
                        break;
                    case AstNumber::Integer:
                        p->integer *= -1;
                        break;
                    case AstNumber::Long:
                        p->str = '-' + p->str;
                        break;
                    }
                    ast = p;
                }
                if(unary->operand->type == AstType::Complex) {
                    AstComplexPtr p = std::static_pointer_cast<AstComplex>(unary->operand);
                    if(p->real) {
                        switch(p->real->num_type) {
                        case AstNumber::Float:
                            p->real->floating *= -1.;
                            break;
                        case AstNumber::Integer:
                            p->real->integer *= -1;
                            break;
                        case AstNumber::Long:
                            p->real->str = '-' + p->real->str;
                            break;
                        }
                        ast = p;
                    }
                    else {
                        p->imag = '-' + p->imag;
                        ast = p;
                    }
                }
            }
            return guard.commit();
        }
        return false;
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

bool subscript(State & s, AstExpr & ast) {
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
            clone_location(index, create(slice));
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

bool decorated(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstExprList dec;
    // decorators (classdef || funcdef)
    if(!decorators(s, dec)) {
        return false;
    }
    AstStmt cls_or_fun;
    if(funcdef(s, cls_or_fun)) {
        assert(cls_or_fun && cls_or_fun->type == AstType::FunctionDef);
        AstFunctionDef & fun = *std::static_pointer_cast<AstFunctionDef>(cls_or_fun);
        fun.decorators.swap(dec);
    }
    else if(classdef(s, cls_or_fun)) {
        assert(cls_or_fun && cls_or_fun->type == AstType::ClassDef);
        AstClassDef & cls = *std::static_pointer_cast<AstClassDef>(cls_or_fun);
        cls.decorators.swap(dec);
    }
    else {
        return false;
    }
    ast = cls_or_fun;
    return guard.commit();
}

bool decorators(State & s, AstExprList & decorators) {
    // decorator+
    AstExpr temp;
    while(decorator(s, temp)) {
        decorators.push_back(temp);
    }
    return !decorators.empty();
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

    // if(is(s, TokenKind::LeftParen)) {
    //     // Normal function call
    //     return false;
    // }

    if(s.options.python3only || s.future_features.print_function) { // Unsupported
        return false;
    }

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
    AstExpr item;
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
    AstTuplePtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // test (expect(s, TokenKind::Comma) test)* [expect(s, TokenKind::Comma)]
    AstExpr temp;
    if(!test(s, temp)) {
        return false;
    }
    ptr->elements.push_back(temp);
    bool last_was_comma = false;
    while(expect(s, TokenKind::Comma)) {
        last_was_comma = true;
        if(!test(s, temp)) {
            break;
        }
        last_was_comma = false;
        ptr->elements.push_back(temp);
    }
    if(ptr->elements.size() == 1 && !last_was_comma) {
        ast = ptr->elements.front();
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
    visit(context_assign{AstContext::Store}, ptr->name);
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
        AstGeneratorPtr ptr;
        location(s, create(ptr));
        ast = ptr;
        ptr->element = first;
        if(!comp_for(s, ptr->generators)) {
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
    visit(context_assign{AstContext::Store}, ptr->target);
    return guard.commit();
}

bool and_test(State & s, AstExpr & ast) {
    // not_test (expect(s, Token::KeywordAnd) not_test)*
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

void make_docstring(State & s, AstSuitePtr & suite_) {
    if(s.options.docstrings && !suite_->items.empty() && suite_->items.front()) {
        if(suite_->items.front()->type == AstType::ExpressionStatement) {
            AstExpressionStatementPtr exprstmt = std::static_pointer_cast<AstExpressionStatement>(suite_->items.front());
            if(exprstmt->expr && exprstmt->expr->type == AstType::Str) {
                AstStrPtr txt = std::static_pointer_cast<AstStr>(exprstmt->expr);
                AstDocStringPtr ptr;
                clone_location(txt, create(ptr));
                ptr->doc = txt->value;
                ptr->unicode = txt->unicode;
                suite_->items[0] = ptr;
            }
        }
    }
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
                if(stmt_->type == AstType::Suite) {
                    flatten(stmt_, suite_->items);
                }
                else {
                    suite_->items.push_back(stmt_);
                }
                stmt_.reset();
                while(stmt(s, stmt_)) {
                    if(stmt_->type == AstType::Suite) {
                        flatten(stmt_, suite_->items);
                    }
                    else {
                        suite_->items.push_back(stmt_);
                    }
                    stmt_.reset();
                }
                make_docstring(s, suite_);
                if(!expect(s, Token::Dedent) && !is(s, Token::End)) {
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
    visit(context_assign{AstContext::Store}, ptr->name);
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
            switch(target->type) {
            case AstType::Name:
            case AstType::Attribute:
            case AstType::Subscript:
                break;
            default:
                syntax_error(s, target, "Illegal expression for augmented assignment");
                return false;
            }

            AstAugAssignPtr ptr;
            location(s, create(ptr));
            ast = ptr;
            ptr->target = target;
            ptr->op = op;
            if(!yield_expr(s, ptr->value) && !testlist(s, ptr->value)) {
                syntax_error(s, ast, "Expected expression after augmented assignment operator");
                return false;
            }
            visit(context_assign{AstContext::AugStore}, target);
            visit(context_assign{AstContext::AugLoad}, ptr->value);
        }
        else {
            if(is(s, TokenKind::Equal)) {
                AstAssignPtr ptr;
                location(s, create(ptr));
                ptr->targets.push_back(target);
                visit(context_assign{AstContext::Store}, target);
                ast = ptr;
                while(expect(s, TokenKind::Equal)) {
                    if(ptr->value) {
                        visit(context_assign{AstContext::Store}, ptr->value);
                        ptr->targets.push_back(ptr->value);
                        ptr->value.reset();
                    }
                    AstExpr value;
                    if(!yield_expr(s, value) && !testlist(s, value)) {
                        syntax_error(s, ast, "Expected expression after assignment operator");
                        return false;
                    }
                    ptr->value = value;
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
    AstCallPtr ptr;
    location(s, create(ptr));
    ast = ptr;
    // expect(s, TokenKind::At) dotted_name
    // [ expect(s, TokenKind::LeftParen) [arglist] expect(s, TokenKind::RightParen) ] expect(s, Token::NewLine)
    if(!expect(s, TokenKind::At)) {
        return false;
    }
    if(!dotted_name_list(s, ptr->function)) {
        syntax_error(s, ast, "Expected identifier after `@`");
        return false;
    }
    if(expect(s, TokenKind::LeftParen)) {
        arglist(s, ptr->arglist);
        if(!expect(s, TokenKind::RightParen)) {
            syntax_error(s, ast, "Expected `)`");
            return false;
        }
    }
    else {
        ast = ptr->function;
    }
    if(!expect(s, Token::NewLine)) {
        syntax_error(s, ast, "Expected new line after decorator usage");
        return false;
    }
    return guard.commit();
}

bool else_stmt(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    // (expect(s, Token::KeywordElIf) test expect(s, TokenKind::Colon) suite)*
    if(expect(s, Token::KeywordElIf)) {
        AstIfPtr if_;
        location(s, create(if_));
        ast = if_;

        if(!test(s, if_->test)) {
            syntax_error(s, ast, "Expected expression after `elif`");
            return false;
        }
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:`");
            return false;
        }
        if(!suite(s, if_->body)) {
            syntax_error(s, ast, "Expected statement block after `elif <expression>:`");
            return false;
        }
        if(!else_stmt(s, if_->orelse)) {
            // If this fails it should have been reported already
            return false;
        }
    }
    // [expect(s, Token::KeywordElse) expect(s, TokenKind::Colon) suite]
    else if(expect(s, Token::KeywordElse)) {
        if(!expect(s, TokenKind::Colon)) {
            syntax_error(s, ast, "Expected `:` after `else`");
            return false;
        }
        if(!suite(s, ast)) {
            return false;
        }
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
    return else_stmt(s, if_->orelse) && guard.commit();
}

bool sliceop(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expect(s, TokenKind::Colon) [test]
    if(!expect(s, TokenKind::Colon)) {
        return false;
    }
    if(!test(s, ast)) {
        location(s, create<AstNone>(ast));
    }
    return guard.commit();
}

bool comparison(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expr (comp_op expr)*
    if(!expr(s, ast)) {
        return false;
    }
    AstCompareOpType op;
    AstComparePtr ptr;
    clone_location(ast, create(ptr));
    ptr->left = ast;
    ast = ptr;
    while(comp_op(s, op)) {
        ptr->operators.push_back(op);
        AstExpr right;
        if(!expr(s, right)) {
            syntax_error(s, ast, "Expected expression after comparison operator");
            return false;
        }
        ptr->comparators.push_back(right);
    }
    if(ptr->operators.empty() && ptr->comparators.empty()) {
        ast = ptr->left;
    }
    return guard.commit();
}

bool term(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // factor (( factor)*
    if(factor(s, ast)) {
        while(is(s, TokenKind::Star) || is(s, TokenKind::Slash) || is(s, TokenKind::Percent) || is(s, TokenKind::DoubleSlash)) {
            TokenKind k = kind(top(s));
            pop(s);

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
    visit(context_assign{AstContext::Del}, del->targets);
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
    AstTuplePtr tuple;
    location(s, create(tuple));
    ast = tuple;
    // fpdef (expect(s, TokenKind::Comma) fpdef)* [expect(s, TokenKind::Comma)]
    AstExpr temp;
    if(fpdef(s, temp)) {
        tuple->elements.push_back(temp);
        bool any_commas = false;
        while(expect(s, TokenKind::Comma)) {
            any_commas = true;
            if(!fpdef(s, temp)) {
                break;
            }
            tuple->elements.push_back(temp);
        }
        if(tuple->elements.size() == 1 && !any_commas) {
            ast = tuple->elements.front();
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
        if(!get_name(s, ast.args)) {
            syntax_error(s, ast, "Expected identifier after `*`");
            return false;
        }
        expect(s, TokenKind::Comma);
    }

    if(expect(s, TokenKind::DoubleStar)) {
        if(!get_name(s, ast.kwargs)) {
            syntax_error(s, ast, "Expected identifier after `**`");
            return false;
        }
    }
    visit(context_assign{AstContext::Param}, ast.args);
    for(auto & a : ast.arguments) {
        visit(context_assign{AstContext::Param}, a);
    }
    for(auto & k : ast.keywords) {
        visit(context_assign{AstContext::Param}, k);
    }
    visit(context_assign{AstContext::Param}, ast.kwargs);
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

struct future_feature_mapping {
    char const * name;
    bool *      value;
};

bool import_from(State & s, AstStmt & ast) {
    future_feature_mapping future_mapping[] = {
        {"nested_scopes", &s.future_features.nested_scopes },
        {"generators", &s.future_features.generators },
        {"division", &s.future_features.division },
        {"absolute_import", &s.future_features.absolute_imports },
        {"with_statement", &s.future_features.with_statement },
        {"print_function", &s.future_features.print_function },
        {"unicode_literals", &s.future_features.unicode_literals },
        {0, 0}
    };

    StateGuard guard(s, ast);
    AstImportFromPtr impfrom;
    location(s, create(impfrom));
    ast = impfrom;
    impfrom->level = 0;
    bool is_future_import = false;
    if(expect(s, Token::KeywordFrom)) {
        // (expect(s, TokenKind::Dot)* dotted_name || expect(s, TokenKind::Dot)+)
        if(is(s, TokenKind::Dot)) {
            while(expect(s, TokenKind::Dot)) ++impfrom->level;
        }
        if(!dotted_name(s, impfrom->module) && impfrom->level == 0) {
            syntax_error(s, ast, "Expected name of module");
            return false;
        }
        if(impfrom->level == 0) {
            assert(impfrom->module->type == AstType::Name);
            is_future_import = std::static_pointer_cast<AstName>(impfrom->module)->id == "__future__";
        }
        //    expect(s, Token::KeywordImport)
        if(!expect(s, Token::KeywordImport)) {
            syntax_error(s, ast, "Expected 'import'");
            return false;
        }
        // expect(s, TokenKind::Star)
        if(is(s, TokenKind::Star)) {
            if(is_future_import) {
                syntax_error(s, ast, "future feature * is not defined");
                return false;
            }
            AstNamePtr ptr;
            location(s, create(ptr));
            expect(s, TokenKind::Star);
            ptr->id = "*";
            AstAliasPtr alias;
            clone_location(ptr, create(alias));
            alias->name = ptr;
            impfrom->names = alias;
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
        if(is_future_import && impfrom->names) {
            auto future_check = [&s, &future_mapping](AstExpr e) -> bool {
                future_feature_mapping * iter = future_mapping;
                if(!e) {
                    assert("Invalid parameter" && false);
                    return false;
                }
                assert(e->type == AstType::Alias || e->type == AstType::Name);
                auto n = e->type == AstType::Name ? e : std::static_pointer_cast<AstAlias>(e)->name;
                if(n) {
                    assert(n->type == AstType::Name);
                    auto & name = *std::static_pointer_cast<AstName>(n);
                    bool found = false;
                    while(iter && iter->name) {
                        if(name.id == iter->name) {
                            *iter->value = found = true;
                        }
                        ++iter;
                    }
                    if(!found) {
                        if(s.options.handle_future_errors) {
                            syntax_error(s, e, ("future feature " + name.id + " is not defined").c_str());
                        }
                    }
                    return found;
                }
                return false;
            };

            bool failure = false;
            if(impfrom->names->type == AstType::Tuple) {
                for(auto e : std::static_pointer_cast<AstTuple>(impfrom->names)->elements) {
                    if(!future_check(e)) {
                        if(s.options.handle_future_errors) {
                            failure = true;
                        }
                    }
                }
            }
            else {
                if(!future_check(impfrom->names)) {
                    if(s.options.handle_future_errors) {
                        failure = true;
                    }
                }
            }
            if(failure) {
                return false;
            }
        }

        return guard.commit();
    }
    return false;
}

bool import_as_names(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    AstTuplePtr exprs;
    location(s, create(exprs));
    ast = exprs;
    // import_as_name (expect(s, TokenKind::Comma) import_as_name)* [expect(s, TokenKind::Comma)]
    AstExpr alias;
    while(import_as_name(s, alias)) {
        exprs->elements.push_back(alias);
        if(!expect(s, TokenKind::Comma)) {
            break;
        }
    }
    if(!is(s, TokenKind::NewLine) && !is(s, TokenKind::SemiColon) && !is(s, TokenKind::RightParen) && !is(s, Token::End)) {
        syntax_error(s, ast, "Unexpected token");
        return false;
    }
    if(exprs->elements.size() == 1) {
        ast = exprs->elements.front();
    }
    return !exprs->elements.empty() && guard.commit();
}

bool import_name(State & s, AstStmt & ast) {
    StateGuard guard(s, ast);
    AstImportPtr imp;
    location(s, create(imp));
    ast = imp;
    // expect(s, Token::KeywordImport) dotted_as_names
    if(!expect(s, Token::KeywordImport)) {
        return false;
    }
    if(!dotted_as_names(s, imp->names)) {
        syntax_error(s, ast, "Expected module name to import");
    }
    return guard.commit();
}

bool import_stmt(State & s, AstStmt & ast) {
    // import_name || import_from
    return import_name(s, ast)
        || import_from(s, ast);
}

bool arith_expr(State & s, AstExpr & ast) {
    StateGuard guard(s);
    // term ((expect(s, TokenKind::Plus)||expect(s, TokenKind::Minus)) term)*
    if(!term(s, ast)) {
        return false;
    }
    while(is(s, TokenKind::Plus) || is(s, TokenKind::Minus)) {
        AstBinOpPtr ptr;
        location(s, create(ptr));
        ptr->left = ast;
        ast = ptr;
        if(expect(s, TokenKind::Plus)) {
            ptr->op = AstBinOpType::Add;
        }
        else if(expect(s, TokenKind::Minus)) {
            ptr->op = AstBinOpType::Sub;
        }
        if(!term(s, ptr->right)) {
            syntax_error(s, ast, "Expected expression after operator");
            return false;
        }
        // Translating Number (+|-) Complex => Complex instead of BinOp
        if(ptr->left && ptr->left->type == AstType::Number) {
            if(ptr->right && ptr->right->type == AstType::Complex) {
                AstNumberPtr real = std::static_pointer_cast<AstNumber>(ptr->left);
                AstComplexPtr p = std::static_pointer_cast<AstComplex>(ptr->right);
                if(!p->real && !p->imag.empty() && p->imag[0] != '-' && p->imag[0] != '+') {
                    p->real = real;
                    p->imag = ((ptr->op == AstBinOpType::Sub) ? '-' : '+') + p->imag;
                    ast = p;
                }
            }
        }
    }
    return guard.commit();
}

bool list_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expect(s, Token::KeywordIf) old_test [list_iter]
    if(!expect(s, Token::KeywordIf)) {
        return false;
    }
    if(!old_test(s, ast)) {
        syntax_error(s, ast, "Expected condition after `if`");
        return false;
    }
    return guard.commit();
}

bool list_for(State & s, AstExprList & ast) {
    StateGuard guard(s);
    if(!is(s, Token::KeywordFor)) {
        return false;
    }
    AstComprPtr compr;
    location(s, create(compr));
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) testlist_safe [list_iter]
    while(expect(s, Token::KeywordFor)) {
        if(!exprlist(s, compr->target)) {
            syntax_error(s, compr, "Expected expression after `for`");
            return false;
        }
        if(!expect(s, Token::KeywordIn)) {
            syntax_error(s, compr, "Expected `in`");
            return false;
        }
        if(!testlist_safe(s, compr->iter)) {
            syntax_error(s, compr, "Expected expression after `in`");
            return false;
        }
        visit(context_assign{AstContext::Store}, compr->target);
        AstExpr if_expr;
        while(list_if(s, if_expr)) {
            compr->ifs.push_back(if_expr);
        }

        ast.push_back(compr);
        compr.reset();
        location(s, create(compr));
    }
    return guard.commit();
}

bool comp_for(State & s, AstExprList & ast) {
    StateGuard guard(s);
    // expect(s, Token::KeywordFor) exprlist expect(s, Token::KeywordIn) or_test [comp_iter]
    if(!is(s, Token::KeywordFor)) {
        return false;
    }
    AstComprPtr compr;
    location(s, create(compr));
    while(expect(s, Token::KeywordFor)) {
        if(!exprlist(s, compr->target)) {
            syntax_error(s, compr, "Expected expression after `for`");
            return false;
        }
        if(!expect(s, Token::KeywordIn)) {
            syntax_error(s, compr, "Expected `in`");
            return false;
        }
        if(!or_test(s, compr->iter)) {
            syntax_error(s, compr, "Expected expression after `in`");
            return false;
        }
        visit(context_assign{AstContext::Store}, compr->target);

        AstExpr if_expr;
        while(comp_if(s, if_expr)) {
            compr->ifs.push_back(if_expr);
        }

        ast.push_back(compr);
        compr.reset();
        location(s, create(compr));
    }
    return guard.commit();
}

bool comp_if(State & s, AstExpr & ast) {
    StateGuard guard(s, ast);
    // expect(s, Token::KeywordIf) old_test [comp_iter]
    if(!expect(s, Token::KeywordIf)) {
        return false;
    }
    if(!old_test(s, ast)) {
        syntax_error(s, ast, "Expected condition after `if`");
        return false;
    }
    return guard.commit();
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

#if 0
bool eval_input(State & s, AstModulePtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    location(s, create(ast->body));
    AstExpressionStatementPtr expr;
    location(s, create(expr));
    ast->body->items.push_back(expr);
    ast->kind = AstModuleKind::Expression;

    // testlist expect(s, Token::NewLine)* expect(s, Token::End)
    if(!testlist(s, expr->expr)) {
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

bool single_input(State & s, AstModulePtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    location(s, create(ast->body));
    ast->kind = AstModuleKind::Interactive;
    // expect(s, Token::NewLine) || simple_stmt || compound_stmt expect(s, Token::NewLine)
    if(expect(s, Token::NewLine)) {
        return guard.commit();
    }
    AstStmt stmt;
    if(simple_stmt(s, stmt)) {
        ast->body->items.push_back(stmt);
        return guard.commit();
    }
    if(compound_stmt(s, stmt)) {
        ast->body->items.push_back(stmt);
        return expect(s, Token::NewLine)
            && guard.commit();
    }
    return false;
}
#endif

bool file_input(State & s, AstModulePtr & ast) {
    StateGuard guard(s, ast);
    location(s, create(ast));
    location(s, create(ast->body));
    ast->kind = AstModuleKind::Module;
    // (expect(s, Token::NewLine) || stmt)* expect(s, Token::End)
    while(!is(s, Token::End)) {
        AstStmt statement;
        if(expect(s, Token::NewLine)) {
            continue;
        }
        else if(stmt(s, statement)) {
            if(statement && statement->type == AstType::Suite) {
                // Flatten potential sub-suites
                flatten(statement, ast->body->items);
            }
            else {
                ast->body->items.push_back(statement);
            }
        }
        else {
            syntax_error(s, ast, "invalid syntax");
            return false;
        }
    }
    if(ast) {
        make_docstring(s, ast->body);
    }
    return guard.commit();
}

SymbolTablePtr create_symbol_table(AstPtr const & a, State & s) {
    if(!a) {
        return {};
    }

    SymbolTablePtr table = std::make_shared<SymbolTable>();
    table->future_features = s.future_features;
    table->file_name = s.lexer->get_name();
    create_from_ast(table, *a,
                    [&s](Error e) {
                        e.file_name = s.lexer->get_name();
                        e.line = s.lexer->get_line(e.cur.line);
                        s.errors.push(e);
                        report_error(s);
                    });
    return table;
}

bool parse(Lexer & lexer,
           AstModulePtr & ast,
           SymbolTablePtr & symbols,
           ParserOptions options /*= ParserOptions()*/) {
    State state;
    state.lexer = &lexer;
    state.tok_cur = lexer.next();
    state.options = options;

    if(file_input(state, ast)) {
        symbols = create_symbol_table(ast, state);
        return true;
    }
    return false;
}

}
