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

#include <pypa/types.hh>

#include <string>
#include <memory>
#include <vector>

namespace pypa {

    enum class AstContext {
        Load,
        Store,
        Del,
        AugLoad,
        AugStore,
        Param
    };

    inline char const * to_string(AstContext o) {
        switch(o) {
        case AstContext::Load:      return "Load";
        case AstContext::Store:     return "Store";
        case AstContext::Del:       return "Del";
        case AstContext::AugLoad:   return "AugLoad";
        case AstContext::AugStore:  return "AugStore";
        case AstContext::Param:     return "Param";
        }
        return "UNKNOWN AstContext";
    }

    enum class AstBinOpType {
        Undefined,
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

    inline char const * to_string(AstBinOpType o) {
        switch(o) {
        case AstBinOpType::Undefined:   return "UNDEFINED";
        case AstBinOpType::Add:         return "+";
        case AstBinOpType::BitAnd:      return "&";
        case AstBinOpType::BitOr:       return "|";
        case AstBinOpType::BitXor:      return "^";
        case AstBinOpType::Div:         return "/";
        case AstBinOpType::FloorDiv:    return "//";
        case AstBinOpType::LeftShift:   return "<<";
        case AstBinOpType::Mod:         return "%";
        case AstBinOpType::Mult:        return "*";
        case AstBinOpType::Power:       return "**";
        case AstBinOpType::RightShift:  return ">>";
        case AstBinOpType::Sub:         return "-";
        }
        return "UNKNOWN AstBinOpType";
    }

    enum class AstUnaryOpType {
        Undefined,
        Add,
        Invert,
        Not,
        Sub,
    };

    inline char const * to_string(AstUnaryOpType o) {
        switch(o) {
        case AstUnaryOpType::Undefined: return "UNDEFINED";
        case AstUnaryOpType::Add:       return "+";
        case AstUnaryOpType::Invert:    return "~";
        case AstUnaryOpType::Not:       return "not";
        case AstUnaryOpType::Sub:       return "-";
        }
        return "UNKNOWN AstUnaryOpType";
    }

    enum class AstBoolOpType {
        Undefined,
        And,
        Or
    };

    inline char const * to_string(AstBoolOpType o) {
        switch(o) {
        case AstBoolOpType::Undefined:  return "UNDEFINED";
        case AstBoolOpType::And:        return "and";
        case AstBoolOpType::Or:         return "or";
        }
        return "UNKNOWN AstBoolOpType";
    }

    enum class AstCompareOpType {
        Undefined,
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

    inline char const * to_string(AstCompareOpType o) {
        switch(o) {
        case AstCompareOpType::Undefined:   return "UNDEFINED";
        case AstCompareOpType::Equals:      return "==";
        case AstCompareOpType::In:          return "in";
        case AstCompareOpType::Is:          return "is";
        case AstCompareOpType::IsNot:       return "is not";
        case AstCompareOpType::Less:        return "<";
        case AstCompareOpType::LessEqual:   return "<=";
        case AstCompareOpType::More:        return ">";
        case AstCompareOpType::MoreEqual:   return ">=";
        case AstCompareOpType::NotEqual:    return "!=";
        case AstCompareOpType::NotIn:       return "not in";
        }
        return "UNKNOWN AstCompareOpType";
    }

    enum class AstModuleKind {
        Module,
        Expression,
        Interactive,
        Suite
    };

    inline char const * to_string(AstModuleKind o) {
        switch(o) {
        case AstModuleKind::Module:         return "Module";
        case AstModuleKind::Expression:     return "Expression";
        case AstModuleKind::Interactive:    return "Interactive";
        case AstModuleKind::Suite:          return "Suite";
        }
        return "UNKNOWN AstModuleKind";
    }

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

}

#endif // GUARD_PYPA_AST_TYPES_HH_INCLUDED
