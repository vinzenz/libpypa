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
