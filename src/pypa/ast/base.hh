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
#ifndef GUARD_PYPA_AST_BASE_HH_INCLUDED
#define GUARD_PYPA_AST_BASE_HH_INCLUDED

#include <pypa/ast/dump.hh>
#include <pypa/ast/macros.hh>

namespace pypa {

PYPA_AST_TYPE_DECL_ALIAS(Ast, AstPtr, AstPtrList) {
    static constexpr AstType TYPE = AstType::Invalid;
    Ast(AstType type) : type(type), line(0), column(0) {}
    AstType type;
    uint32_t line;
    uint32_t column;
};

template<AstType Type, typename Base = Ast>
struct AstT : Base {
    static constexpr AstType TYPE = Type;
    AstT(AstType type = Type) : Base{type} {}
};
template<>
struct AstIDByType<Ast> { static constexpr AstType Id = AstType::Invalid; };

template<AstType Type, typename Base>
constexpr AstType AstT<Type, Base>::TYPE;

PYPA_AST_TYPE_DECL_DERIVED_ALIAS(Expression, AstExpr, AstExprList) {
    using AstT<AstType::Expression>::AstT;
};
DEF_AST_TYPE_BY_ID1(Expression);
PYPA_AST_MEMBERS0(Expression);

template<AstType Type>
struct AstExprT : AstT<Type, AstExpression> {
    AstExprT()
    : AstT<Type, AstExpression>(Type)
    {}
};

PYPA_AST_TYPE_DECL_DERIVED_ALIAS(Statement, AstStmt, AstStmtList) {
      using AstT<AstType::Statement>::AstT;
};
DEF_AST_TYPE_BY_ID1(Statement);
PYPA_AST_MEMBERS0(Statement);

template<AstType Type>
struct AstStmtT : AstT<Type, AstStatement> {
    AstStmtT() : AstT<Type, AstStatement>(Type){}
};

PYPA_AST_STMT(Suite) {
    AstStmtList items;
};
PYPA_AST_MEMBERS1(Suite, items);

PYPA_AST_TYPE_DECL_DERIVED_ALIAS(SliceType, AstSliceTypePtr, AstSliceTypeList) {
    using AstT<AstType::SliceType>::AstT;
};
DEF_AST_TYPE_BY_ID1(SliceType);
PYPA_AST_MEMBERS0(SliceType);

template<AstType Type>
struct AstSliceTypeT : AstT<Type, AstSliceType> {
    AstSliceTypeT() : AstT<Type, AstSliceType>(Type){}
};

}
#endif // GUARD_PYPA_AST_BASE_HH_INCLUDED
