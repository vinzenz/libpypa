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


PYPA_AST_EXPR(Tuple) {
    AstExprList elements;
    AstContext  context;
};
PYPA_AST_MEMBERS2(Tuple, context, elements);

PYPA_AST_EXPR(Name) {
    AstContext  context;
    bool        dotted;
    String      id;
};
PYPA_AST_MEMBERS3(Name, context, dotted, id);

PYPA_AST_EXPR(Keyword) {
    AstExpr name;
    AstExpr value;
};
PYPA_AST_MEMBERS2(Keyword, name, value);


PYPA_AST_EXPR(Alias) {
    AstExpr name;
    AstExpr as_name;
};
PYPA_AST_MEMBERS2(Alias, as_name, name);

PYPA_AST_STMT(Assert) {
    AstExpr expression;
    AstExpr test;
};
PYPA_AST_MEMBERS2(Assert, expression, test);

PYPA_AST_STMT(Assign) {
    AstExprList targets; // since this can be chained tgt = value = value...
    AstExpr     value;
};
PYPA_AST_MEMBERS2(Assign, targets, value);

PYPA_AST_TYPE_DECL_DERIVED(Arguments) {
    AstExprList arguments;
    AstExprList defaults;
    AstExpr     kwargs;
    AstExpr     args;
    AstExprList keywords;
};
DEF_AST_TYPE_BY_ID1(Arguments);
PYPA_AST_MEMBERS5(Arguments, args, arguments, defaults, kwargs, keywords);

PYPA_AST_STMT(AugAssign) {
    AstExpr         value;
    AstExpr         target;
    AstBinOpType    op;
};
PYPA_AST_MEMBERS3(AugAssign, op, target, value);

PYPA_AST_EXPR(Attribute) {
    AstExpr     value;
    AstContext  context;
    AstExpr     attribute;
};
PYPA_AST_MEMBERS3(Attribute, attribute, context, value);

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
PYPA_AST_MEMBERS2(BoolOp, op, values);

PYPA_AST_STMT(Break) {};
PYPA_AST_MEMBERS0(Break);

PYPA_AST_EXPR(Call) {
    AstExpr      function;
    AstArguments arglist;
};
PYPA_AST_MEMBERS2(Call, arglist, function);

PYPA_AST_STMT(ClassDef) {
    AstExprList decorators;
    AstExpr     name;
    AstExpr     bases;
    AstStmt     body;
};
PYPA_AST_MEMBERS4(ClassDef, bases, body, decorators, name);

PYPA_AST_STMT(Continue) {};
PYPA_AST_MEMBERS0(Continue);

PYPA_AST_EXPR(Compare) {
    AstExprList comparators;
    AstExpr left;
    std::vector<AstCompareOpType> operators;
};
PYPA_AST_MEMBERS3(Compare, comparators, left, operators);

PYPA_AST_EXPR(Comprehension) {
    AstExpr     target;
    AstExpr     iter;
    AstExprList ifs;
};
typedef AstComprehensionPtr      AstComprPtr;
typedef std::vector<AstComprPtr> AstComprList;
PYPA_AST_MEMBERS3(Comprehension, ifs, iter, target);

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
    AstExpr     key;
    AstExpr     value;
    AstExprList generators;
};
PYPA_AST_MEMBERS3(DictComp, generators, key, value);

PYPA_AST_STMT(DocString) {
    String doc;
    bool unicode;
};
PYPA_AST_MEMBERS2(DocString, doc, unicode);

PYPA_AST_EXPR(EllipsisObject) {};
PYPA_AST_MEMBERS0(EllipsisObject);

PYPA_AST_SLICE(Ellipsis) {};
PYPA_AST_MEMBERS0(Ellipsis);

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
PYPA_AST_MEMBERS3(Except, body, name, type);

PYPA_AST_STMT(ExpressionStatement) {
    AstExpr expr;
};
PYPA_AST_MEMBERS1(ExpressionStatement, expr);

PYPA_AST_SLICE(ExtSlice) {
    AstSliceTypeList dims;
};
PYPA_AST_MEMBERS1(ExtSlice, dims);

PYPA_AST_STMT(For) {
    AstStmt body;
    AstStmt orelse;
    AstExpr target;
    AstExpr iter;
};
PYPA_AST_MEMBERS4(For, body, iter, orelse, target);

PYPA_AST_STMT(FunctionDef) {
   AstExprList  decorators;
   AstExpr      name;
   AstArguments args;
   AstStmt      body;
};
PYPA_AST_MEMBERS4(FunctionDef, args, body, decorators, name);

PYPA_AST_EXPR(Generator) {
    AstExpr     element;
    AstExprList generators;
};
PYPA_AST_MEMBERS2(Generator, element, generators);

PYPA_AST_STMT(Global) {
    std::vector<AstNamePtr> names;
};
PYPA_AST_MEMBERS1(Global, names);

PYPA_AST_STMT(If) {
    AstExpr     test;
    AstStmt     body;
    AstStmt     orelse;
};
PYPA_AST_MEMBERS3(If, body, orelse, test);

PYPA_AST_EXPR(IfExpr) {
    AstExpr body;
    AstExpr orelse;
    AstExpr test;
};
PYPA_AST_MEMBERS3(IfExpr, body, orelse, test);

PYPA_AST_SLICE(Index) {
    AstExpr value;
};
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
PYPA_AST_MEMBERS3(ImportFrom, level, module, names);

PYPA_AST_EXPR(Lambda) {
    AstArguments    arguments;
    AstExpr         body;
};
PYPA_AST_MEMBERS2(Lambda, arguments, body);

PYPA_AST_EXPR(List) {
    AstExprList elements;
    AstContext  context;
};
PYPA_AST_MEMBERS2(List, context, elements);

PYPA_AST_EXPR(ListComp) {
    AstExpr     element;
    AstExprList generators;
};
PYPA_AST_MEMBERS2(ListComp, element, generators);

PYPA_AST_EXPR(None) {};
PYPA_AST_MEMBERS0(None);

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
    String str;
};
PYPA_AST_MEMBERS5(Number, data, floating, integer, num_type, str);


PYPA_AST_EXPR(Complex) {
    AstNumberPtr real;
    String imag;
};
PYPA_AST_MEMBERS2(Complex, imag, real);

PYPA_AST_TYPE_DECL_DERIVED(Module) {
    AstSuitePtr     body;
    AstModuleKind   kind;
};
DEF_AST_TYPE_BY_ID1(Module);
PYPA_AST_MEMBERS2(Module, body, kind);

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
    AstExpr arg0; // type
    AstExpr arg1; // inst
    AstExpr arg2; // tback
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
    AstExpr     element;
    AstExprList generators;
};
PYPA_AST_MEMBERS2(SetComp, element, generators);

PYPA_AST_SLICE(Slice) {
    AstExpr lower;
    AstExpr upper;
    AstExpr step;
};
PYPA_AST_MEMBERS3(Slice, lower, step, upper);

PYPA_AST_EXPR(Str) {
    String value;
    bool unicode;
};
PYPA_AST_MEMBERS2(Str, value, unicode);

PYPA_AST_EXPR(Subscript) {
    AstExpr         value;
    AstSliceTypePtr slice;
    AstContext      context;
};
PYPA_AST_MEMBERS3(Subscript, context, slice, value);

PYPA_AST_STMT(TryExcept) {
    AstStmt         body;
    AstStmt         orelse;
    AstExceptList   handlers;
};
PYPA_AST_MEMBERS3(TryExcept, body, handlers, orelse);

PYPA_AST_STMT(TryFinally) {
    AstStmt body;
    AstStmt final_body;
};
PYPA_AST_MEMBERS2(TryFinally, body, final_body);

PYPA_AST_EXPR(UnaryOp) {
    AstExpr         operand;
    AstUnaryOpType  op;
};
PYPA_AST_MEMBERS2(UnaryOp, op, operand);

PYPA_AST_STMT(While) {
    AstExpr test;
    AstStmt body;
    AstStmt orelse;
};
PYPA_AST_MEMBERS3(While, body, orelse, test);

PYPA_AST_STMT(With) {
    AstExpr         context;
    AstExpr         optional;
    AstStmt         body;
};
PYPA_AST_MEMBERS3(With, body, context, optional);


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
