#ifndef GUARD_PYPA_PARSER_PARSER_FWD_HH_INCLUDED
#define GUARD_PYPA_PARSER_PARSER_FWD_HH_INCLUDED

#include <pypa/ast/ast.hh>

namespace pypa {

    bool and_expr(State & s, AstExpr & ast);
    bool and_test(State & s, AstExpr & ast);
    bool arglist(State & s, AstArguments & ast);
    bool argument(State & s, AstExpr & ast);
    bool arith_expr(State & s, AstExpr & ast);
    bool assert_stmt(State & s, AstStmt & ast);
    bool atom(State & s, AstExpr & ast);
    bool augassign(State & s, AstExpr & ast);
    bool break_stmt(State & s, AstStmt & ast);
    bool classdef(State & s, AstStmt & ast);
    bool comp_for(State & s, AstExpr & ast);
    bool comp_if(State & s, AstExpr & ast);
    bool comp_iter(State & s, AstExpr & ast);
    bool comp_op(State & s, AstCompareOpType & op);
    bool comparison(State & s, AstExpr & ast);
    bool compound_stmt(State & s, AstStmt & ast);
    bool continue_stmt(State & s, AstStmt & ast);
    bool decorated(State & s, AstStmt & ast);
    bool decorator(State & s, AstExpr & ast);
    bool decorators(State & s, AstExpr & ast);
    bool del_stmt(State & s, AstStmt & ast);
    bool dictorsetmaker(State & s, AstExpr & ast);
    bool dotted_as_name(State & s, AstExpr & ast);
    bool dotted_as_names(State & s, AstExpr & ast);
    bool dotted_name(State & s, AstExpr & ast);
    bool eval_input(State & s, AstStmt & ast);
    bool except_clause(State & s, AstExpr & ast);
    bool exec_stmt(State & s, AstStmt & ast);
    bool expr(State & s, AstExpr & ast);
    bool expr_stmt(State & s, AstStmt & ast);
    bool exprlist(State & s, AstExpr & ast);
    bool factor(State & s, AstExpr & ast);
    bool file_input(State & s, AstModulePtr & ast);
    bool flow_stmt(State & s, AstStmt & ast);
    bool for_stmt(State & s, AstStmt & ast);
    bool fpdef(State & s, AstExpr & ast);
    bool fplist(State & s, AstExpr & ast);
    bool funcdef(State & s, AstStmt & ast);
    bool global_stmt(State & s, AstStmt & ast);
    bool if_stmt(State & s, AstStmt & ast);
    bool import_as_name(State & s, AstExpr & ast);
    bool import_as_names(State & s, AstExpr & ast);
    bool import_from(State & s, AstStmt & ast);
    bool import_name(State & s, AstStmt & ast);
    bool import_stmt(State & s, AstStmt & ast);
    bool lambdef(State & s, AstExpr & ast);
    bool list_for(State & s, AstExpr & ast);
    bool list_if(State & s, AstExpr & ast);
    bool list_iter(State & s, AstExpr & ast);
    bool listmaker(State & s, AstExpr & ast);
    bool not_test(State & s, AstExpr & ast);
    bool old_lambdef(State & s, AstExpr & ast);
    bool old_test(State & s, AstExpr & ast);
    bool or_test(State & s, AstExpr & ast);
    bool parameters(State & s, AstArguments & ast);
    bool pass_stmt(State & s, AstStmt & ast);
    bool power(State & s, AstExpr & ast);
    bool print_stmt(State & s, AstStmt & ast);
    bool raise_stmt(State & s, AstStmt & ast);
    bool return_stmt(State & s, AstStmt & ast);
    bool shift_expr(State & s, AstExpr & ast);
    bool simple_stmt(State & s, AstStmt & ast);
    bool single_input(State & s, AstStmt & ast);
    bool sliceop(State & s, AstExpr & ast);
    bool small_stmt(State & s, AstStmt & ast);
    bool stmt(State & s, AstStmt & ast);
    bool subscript(State & s, AstExpr & ast);
    bool subscriptlist(State & s, AstExpr & ast);
    bool suite(State & s, AstStmt & ast);
    bool term(State & s, AstExpr & ast);
    bool test(State & s, AstExpr & ast);
    bool testlist(State & s, AstExpr & ast);
    bool testlist_comp(State & s, AstExpr & ast);
    bool testlist_safe(State & s, AstExpr & ast);
    bool trailer(State & s, AstExpr & ast);
    bool try_stmt(State & s, AstStmt & ast);
    bool varargslist(State & s, AstArguments & ast);
    bool while_stmt(State & s, AstStmt & ast);
    bool with_item(State & s, AstWithItemPtr & ast);
    bool with_stmt(State & s, AstStmt & ast);
    bool xor_expr(State & s, AstExpr & ast);
    bool yield_expr(State & s, AstExpr & ast);
    bool yield_stmt(State & s, AstStmt & ast);

}

#endif //GUARD_PYPA_PARSER_PARSER_FWD_HH_INCLUDED
