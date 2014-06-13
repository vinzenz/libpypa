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
#ifndef GUARD_PYPA_AST_TYPES_HH_INCLUDED
#define GUARD_PYPA_AST_TYPES_HH_INCLUDED

#include <string>
#include <memory>
#include <vector>

namespace pypa {
typedef std::string String;
typedef std::vector<String> StringList;

enum class AstContext {
    Load = 1, Store = 2, Del = 3, AugLoad = 4, AugStore = 5, Param = 6
};

enum class AstBinOpType {
    Add,
    BitAnd,
    BitOr,
    BitXor,
    Div,
    FloorDiv,
    LeftShift,
    Mod,
    Mult,
    Power,
    RightShift,
    Sub,
};

enum class AstUnaryOpType {
    Add,
    Invert,
    Not,
    Sub,
};

enum class AstBoolOpType {
    And,
    Or
};

enum class AstCompareOpType {
    Equals,
    In,
    Is,
    IsNot,
    Less,
    LessEqual,
    More,
    MoreEqual,
    NotEqual,
    NotIn,
};

enum class AstType {
    Invalid = -1,
#undef PYPA_AST_TYPE
#define PYPA_AST_TYPE(X) X,
#   include <pypa/ast/ast_type.inl>
#undef PYPA_AST_TYPE
};

template<AstType TypeID>
struct AstTypeByID;

template<typename T>
struct AstIDByType;

template<typename T>
struct AstIDByType< std::shared_ptr<T> > : AstIDByType<T>  {};

template<typename T>
struct AstIDByType< T const > : AstIDByType<T> {};

template<AstType TypeID>
struct AstTypePtrByID {
    typedef std::shared_ptr<typename AstTypeByID<TypeID>::Type> Type;
};

template<AstType>
struct ast_member_visit;

template<typename T>
struct ast_member_dump;

struct ast_member_dump_revisit {
    int depth_;
    ast_member_dump_revisit(int depth) : depth_(depth) {}
    template< typename T >
    void operator() ( T const & v ) const {
        ast_member_dump<T>::dump(depth_, v);
    }
};

struct Ast;
namespace detail {
    void visit_dump_internal(int depth, Ast const & v);

    template< typename T, typename V, typename F>
    void apply_member(T & t, V T::*member, F f) {
        f(t.*member);
    }

    template< typename T, typename V, typename F>
    void apply_member(std::shared_ptr<T> t, V T::*member, F f) {
        f((*t).*member);
    }
}

}

#endif // GUARD_PYPA_AST_TYPES_HH_INCLUDED
