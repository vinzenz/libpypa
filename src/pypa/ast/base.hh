#ifndef GUARD_PYPA_AST_BASE_HH_INCLUDED
#define GUARD_PYPA_AST_BASE_HH_INCLUDED

#include <pypa/ast/types.hh>
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
    using Ast::Ast;
    AstT(AstType type = Type) : Base{type} {}
};

template<AstType Type, typename Base>
constexpr AstType AstT<Type, Base>::TYPE;

PYPA_AST_TYPE_DECL_DERIVED_ALIAS(Expression, AstExpr, AstExprList) {
    using AstT<AstType::Expression>::AstT;
};
DEF_AST_TYPE_BY_ID1(Expression);

template<AstType Type>
struct AstExprT : AstT<Type, AstExpression> {};

PYPA_AST_TYPE_DECL_DERIVED_ALIAS(Statement, AstStmt, AstStmtList) {
      using AstT<AstType::Statement>::AstT;
};
DEF_AST_TYPE_BY_ID1(Statement);

template<AstType Type>
struct AstStmtT : AstT<Type, AstStatement> { using AstT<AstType::Statement>::AstT;};

struct AstSliceKind : Ast {};
typedef std::shared_ptr<AstSliceKind> AstSliceKindPtr;

template<AstType Type>
struct AstSliceT : AstT<Type, AstSliceKind> {
    static_assert(
        Type == AstType::ExtSlice
     || Type == AstType::Index
     || Type == AstType::Ellipsis
     || Type == AstType::Slice,
     "Not passed type is not a SliceKind"
    );
};

}
#endif // GUARD_PYPA_AST_BASE_HH_INCLUDED
