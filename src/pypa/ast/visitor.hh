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
#ifndef GUARD_PYPA_AST_VISITOR_HH_INCLUDED
#define GUARD_PYPA_AST_VISITOR_HH_INCLUDED

#include <pypa/ast/ast.hh>
#include <type_traits>
#include <cassert>

namespace pypa {

template<typename F>
inline void visit(F visitor, AstPtr v) {
    if(!v) return;
    switch(v->type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(std::static_pointer_cast<typename AstTypeByID<AstType::X>::Type>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
}

template<typename R, typename F>
inline R visit(F visitor, AstPtr v) {
    if(!v) { assert("Visit called with null pointer" && false); return R(); }
    switch(v->type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(std::static_pointer_cast<typename AstTypeByID<AstType::X>::Type>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
    return R();
}

template<typename R, typename F>
inline R visit(F visitor, Ast & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(static_cast<AstTypeByID<AstType::X>::Type&>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
    return R();
}

template<typename F>
inline void visit(F visitor, Ast & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(static_cast<AstTypeByID<AstType::X>::Type&>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
}

template<typename R, typename F>
inline R visit(F visitor, Ast const & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: return visitor(static_cast<AstTypeByID<AstType::X>::Type const &>(v));
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
    return R();
}

template<typename F>
inline void visit(F visitor, Ast const & v) {
    switch(v.type) {
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) case AstType::X: visitor(static_cast<AstTypeByID<AstType::X>::Type const &>(v)); break;
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
        default:
        assert("Invalid AST type received" && false);
            break;
    }
}

}

#endif // GUARD_PYPA_AST_VISITOR_HH_INCLUDED
