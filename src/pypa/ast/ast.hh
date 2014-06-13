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
#ifndef GUARD_PYPA_AST_AST_HH_INCLUDED
#define GUARD_PYPA_AST_AST_HH_INCLUDED

#include <pypa/ast/base.hh>


namespace pypa {

PYPA_AST_EXPR(Expressions) {
    AstExprList items;
};
PYPA_AST_MEMBERS1(Expressions, items);

PYPA_AST_EXPR(Name) {
    AstContext  context;
    String      id;
};
PYPA_AST_MEMBERS2(Name, context, id);

PYPA_AST_EXPR(Keyword) {
    AstExpr name;
    AstExpr value;
};
PYPA_AST_MEMBERS2(Keyword, name, value);


PYPA_AST_EXPR(Alias) {
    AstExpr name;
    AstExpr as_name;
};
PYPA_AST_MEMBERS2(Alias, name, as_name);

PYPA_AST_STMT(Assert) {
    AstExpr expression;
    AstExpr test;
};
PYPA_AST_MEMBERS2(Assert, test, expression);

PYPA_AST_STMT(Assign) {
    AstExpr targets;
    AstExpressions value; // since this can be chained tgt = value = value...
};
PYPA_AST_MEMBERS2(Assign, targets, value);

PYPA_AST_TYPE_DECL_DERIVED(Arguments) {
    AstExprList arguments;
    AstExprList defaults;
    AstExpr     kwargs;
    AstExprList args;
    AstExprList keywords;
};
DEF_AST_TYPE_BY_ID1(Arguments);
PYPA_AST_MEMBERS5(Arguments, arguments, defaults, kwargs, args, keywords);

PYPA_AST_STMT(AugAssign) {
    AstExpr         value;
    AstExpr         target;
    AstBinOpType    op;
};
PYPA_AST_MEMBERS3(AugAssign, value, target, op);

PYPA_AST_EXPR(Attribute) {
    AstExpr     value;
    AstContext  context;
    AstExpr     attribute;
};
PYPA_AST_MEMBERS3(Attribute, value, context, attribute);

PYPA_AST_EXPR(BinOp) {
    AstExpr         left;
    AstBinOpType    op;
    AstExpr         right;
};
PYPA_AST_MEMBERS3(BinOp, left, op, right);

PYPA_AST_EXPR(Bool) {
    bool value;
};
PYPA_AST_MEMBERS1(Bool, value);

PYPA_AST_EXPR(BoolOp) {
    AstBoolOpType op;
    AstExprList   values;
};
PYPA_AST_MEMBERS2(BinOp, op, values);

PYPA_AST_STMT(Break) {};
PYPA_AST_MEMBERS0(Break);

PYPA_AST_EXPR(Call) {
    AstExpr      function;
    AstArguments arglist;
};
PYPA_AST_MEMBERS2(Call, function, arglist);

PYPA_AST_STMT(ClassDef) {
    AstExpr name;
    AstExpr bases;
    AstStmt body;
};
PYPA_AST_MEMBERS3(BinOp, name, bases, body);

PYPA_AST_STMT(Continue) {};
PYPA_AST_MEMBERS0(Continue);

PYPA_AST_EXPR(Compare) {
    AstExpr left;
    AstCompareOpType op;
    AstExpr right;
};
PYPA_AST_MEMBERS3(Compare, left, op, right);

PYPA_AST_EXPR(Complex) {
    String complex;
};
PYPA_AST_MEMBERS1(Complex, complex);

PYPA_AST_EXPR(Comprehension) {
    AstExpr target;
    AstExpr iter;
};
typedef AstComprehensionPtr      AstComprPtr;
typedef std::vector<AstComprPtr> AstComprList;
PYPA_AST_MEMBERS2(Comprehension, target, iter);

PYPA_AST_EXPR(Decorator) {
    AstExpr      name;
    AstArguments arguments;
};
PYPA_AST_MEMBERS2(Decorator, name, arguments);

PYPA_AST_EXPR(Decorators) {
    AstExprList decorators;
};
PYPA_AST_MEMBERS1(Decorators, decorators);

PYPA_AST_STMT(Decorated) {
    AstExpr decorators;
    AstStmt cls_or_fun_def;
};
PYPA_AST_MEMBERS2(Decorated, decorators, cls_or_fun_def);

PYPA_AST_STMT(Delete) {
    AstExpr targets;
};
PYPA_AST_MEMBERS1(Delete, targets);

PYPA_AST_EXPR(Dict) {
    AstExprList keys;
    AstExprList values;
};
PYPA_AST_MEMBERS2(Dict, keys, values);

PYPA_AST_EXPR(DictComp) {
    AstExpr key;
    AstExpr value;
    AstExpr generators;
};
PYPA_AST_MEMBERS3(DictComp, key, value, generator);

PYPA_AST_TYPE_DECL_SLICE_KIND(Ellipsis) {};
DEF_AST_TYPE_BY_ID1(Ellipsis);
PYPA_AST_MEMBERS0(Ellipsis);

PYPA_AST_EXPR(ElseIf) {
    AstExpr test;
    AstStmt body;
};
PYPA_AST_MEMBERS2(ElseIf, body, test);

PYPA_AST_STMT(Exec) {
    AstExpr body;
    AstExpr globals;
    AstExpr locals;
};
PYPA_AST_MEMBERS3(Exec, body, globals, locals);

PYPA_AST_EXPR(Except) {
    AstExpr type;
    AstExpr name;
    AstStmt body;
};
typedef std::vector<AstExceptPtr> AstExceptList;
PYPA_AST_MEMBERS3(Except, type, name, body);

PYPA_AST_STMT(ExpressionStatement) {
    AstExpr expr;
};
PYPA_AST_MEMBERS1(ExpressionStatement, expr);

PYPA_AST_TYPE_DECL_SLICE_KIND(ExtSlice) {
    AstSliceKindList dims;
};
DEF_AST_TYPE_BY_ID1(ExtSlice);
PYPA_AST_MEMBERS1(ExtSlice, dims);

PYPA_AST_STMT(For) {
    AstStmt body;
    AstStmt orelse;
    AstExpr target;
    AstExpr iter;
};
PYPA_AST_MEMBERS4(For, body, orelse, target, iter);

PYPA_AST_EXPR(ForExpr) {
    AstExpr items;
    AstExpr generators;
    AstExpr iter;
};
PYPA_AST_MEMBERS3(ForExpr, items, generators, iter);

PYPA_AST_STMT(FunctionDef) {
   AstExpr      name;
   AstArguments args;
   AstStmt      body;
};
PYPA_AST_MEMBERS3(FunctionDef, name, args, body);

PYPA_AST_EXPR(Generator) {
    AstExpr expression;
    AstExpr for_expr;
};
PYPA_AST_MEMBERS2(Generator, expression, for_expr);

PYPA_AST_STMT(Global) {
    std::vector<AstNamePtr> names;
};
PYPA_AST_MEMBERS1(Global, names);

PYPA_AST_STMT(If) {
    AstExpr     test;
    AstStmt     body;
    AstExpr     elif;
    AstStmt     orelse;
};
PYPA_AST_MEMBERS4(If, test, body, elif, orelse);

PYPA_AST_EXPR(IfExpr) {
    AstExpr body;
    AstExpr orelse;
    AstExpr test;
};
PYPA_AST_MEMBERS3(IfExpr, body, orelse, test);

PYPA_AST_TYPE_DECL_SLICE_KIND(Index) {
    AstExpr value;
};
DEF_AST_TYPE_BY_ID1(Index);
PYPA_AST_MEMBERS1(Index, value);

PYPA_AST_STMT(Import) {
    AstExpr names;
};
PYPA_AST_MEMBERS1(Import, names);

PYPA_AST_STMT(ImportFrom) {
    AstExpr         module;
    AstExpr         names;
    int             level;
};
PYPA_AST_MEMBERS3(ImportFrom, module, names, level);

PYPA_AST_EXPR(Lambda) {
    AstArguments    arguments;
    AstExpr         body;
};
PYPA_AST_MEMBERS2(Lambda, arguments, body);

PYPA_AST_EXPR(List) {
    AstExprList elements;
    AstContext  context;
};
PYPA_AST_MEMBERS2(List, elements, context);

PYPA_AST_EXPR(ListComp) {
    AstExpr element;
    AstExpr generators;
};
PYPA_AST_MEMBERS2(ListComp, element, generators);

PYPA_AST_EXPR(None) {};

PYPA_AST_EXPR(Number) {
    enum Type {
        Integer,
        Long,
        Float
    } num_type;
    union {
        double  floating;
        int64_t integer;
        char    data[sizeof(floating) > sizeof(integer) ? sizeof(floating) : sizeof(integer)];
    };
};
PYPA_AST_MEMBERS2(Number, num_type, data);

PYPA_AST_TYPE_DECL_DERIVED(Module) {
    AstSuitePtr body;
};
DEF_AST_TYPE_BY_ID1(Module);
PYPA_AST_MEMBERS1(Module, body);

PYPA_AST_STMT(Pass) {
};
PYPA_AST_MEMBERS0(Pass);

PYPA_AST_STMT(Print) {
    AstExpr         destination;
    bool            newline;
    AstExprList     values;
};
PYPA_AST_MEMBERS3(Print, destination, newline, values);


PYPA_AST_EXPR(Repr) {
    AstExpr value;
};
PYPA_AST_MEMBERS1(Repr, value);

PYPA_AST_STMT(Raise) {
    AstExpr arg0;
    AstExpr arg1;
    AstExpr arg2;
};
PYPA_AST_MEMBERS3(Raise, arg0, arg1, arg2);

PYPA_AST_STMT(Return) {
    AstExpr value;
};
PYPA_AST_MEMBERS1(Return, value);

PYPA_AST_EXPR(Set) {
    AstExprList elements;
};
PYPA_AST_MEMBERS1(Set, elements);

PYPA_AST_EXPR(SetComp) {
    AstExpr element;
    AstExpr generators;
};
PYPA_AST_MEMBERS2(SetComp, element, generators);

PYPA_AST_TYPE_DECL_SLICE_KIND(Slice) {
    AstExpr lower;
    AstExpr upper;
    AstExpr step;
};
DEF_AST_TYPE_BY_ID1(Slice);
PYPA_AST_MEMBERS3(Slice, lower, upper, step);

PYPA_AST_EXPR(Str) {
    String value;
    AstContext context;
};
PYPA_AST_MEMBERS2(Str, value, context);

PYPA_AST_EXPR(Subscript) {
    AstExpr         value;
    AstSliceKindPtr slice;
    AstContext      context;
};
PYPA_AST_MEMBERS3(Slice, value, slice, context);

PYPA_AST_STMT(TryExcept) {
    AstStmt         body;
    AstStmt         orelse;
    AstExceptList   handlers;
};
PYPA_AST_MEMBERS3(TryExcept, body, orelse, handlers);

PYPA_AST_STMT(TryFinally) {
    AstStmt body;
    AstStmt final_body;
};
PYPA_AST_MEMBERS2(TryFinally, body, final_body);

PYPA_AST_EXPR(Tuple) {
    AstExprList elements;
    AstContext  context;
};
PYPA_AST_MEMBERS2(Tuple, elements, context);

PYPA_AST_EXPR(UnaryOp) {
    AstExpr         operand;
    AstUnaryOpType  op;
};
PYPA_AST_MEMBERS2(UnaryOp, operand, op);

PYPA_AST_STMT(While) {
    AstExpr test;
    AstStmt body;
    AstStmt orelse;
};
PYPA_AST_MEMBERS3(While, test, body, orelse);

PYPA_AST_TYPE_DECL_DERIVED(WithItem) {
    AstExpr context;
    AstExpr optional;
};
DEF_AST_TYPE_BY_ID1(WithItem);
PYPA_AST_MEMBERS2(WithItem, context, optional);

PYPA_AST_STMT(With) {
    AstWithItemList items;
    AstStmt         body;
};
PYPA_AST_MEMBERS2(With, items, body);


PYPA_AST_EXPR(YieldExpr) {
    AstExpr args;
};
PYPA_AST_MEMBERS1(YieldExpr, args);

PYPA_AST_STMT(Yield) {
    AstExpr yield;
};
PYPA_AST_MEMBERS1(Yield, yield);

}
#endif // GUARD_PYPA_AST_AST_HH_INCLUDED
