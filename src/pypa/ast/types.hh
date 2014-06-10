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

template<AstType TypeID>
struct AstTypePtrByID {
    typedef std::shared_ptr<typename AstTypeByID<TypeID>::Type> Type;
};

}

#endif // GUARD_PYPA_AST_TYPES_HH_INCLUDED
