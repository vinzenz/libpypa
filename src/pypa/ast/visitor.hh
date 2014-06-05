#ifndef GUARD_PYPA_AST_VISITOR_HH_INCLUDED
#define GUARD_PYPA_AST_VISITOR_HH_INCLUDED

#include <pypa/ast/ast.hh>
#include <type_traits>

namespace pypa {

template<typename F>
inline void visit(F visitor, AstPtr v) {
    switch(v->type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(std::static_pointer_cast<typename AstTypeByID<AstType::X>::Type>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

template<typename R, typename F>
inline R visit(F visitor, AstPtr v) {
    switch(v->type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(std::static_pointer_cast<typename AstTypeByID<AstType::X>::Type>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

template<typename R, typename F>
inline R visit(F visitor, Ast & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(static_cast<AstTypeByID<AstType::X>::Type&>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

template<typename F>
inline void visit(F visitor, Ast & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(static_cast<AstTypeByID<AstType::X>::Type&>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

template<typename R, typename F>
inline R visit(F visitor, Ast const & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(static_cast<AstTypeByID<AstType::X>::Type const &>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

template<typename F>
inline void visit(F visitor, Ast const & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(static_cast<AstTypeByID<AstType::X>::Type const &>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
    }
}

}

#endif // GUARD_PYPA_AST_VISITOR_HH_INCLUDED
