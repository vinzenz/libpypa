#ifndef GUARD_PYPA_AST_AST_HH_INCLUDED
#define GUARD_PYPA_AST_AST_HH_INCLUDED

#include <memory>
#include <string>
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

enum class AstCompareType {
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
    Alias,
    Arguments,
    Assert,
    Assign,
    Attribute,
    AugAssign,
    BinOp,
    BoolOp,
    Break,
    Call,
    ClassDef,
    Compare,
    Comprehension,
    Continue,
    Decorated,
    Decorator,
    Decorators,
    Del,
    Delete,
    Dict,
    DictComp,
    DottedName,
    Ellipsis,
    Except,
    Exec,
    Expression,
    ExpressionStatement,
    ExtSlice,
    For,
    FunctionDef,
    Global,
    If,
    IfExpr,
    Import,
    ImportFrom,
    Index,
    Keyword,
    Lambda,
    List,
    ListComp,
    Module,
    Name,
    Number,
    Parameters,
    Pass,
    Print,
    Raise,
    Repr,
    Return,
    Set,
    SetComp,
    Slice,
    Statement,
    Str,
    Subscript,
    TryExcept,
    TryFinally,
    Tuple,
    UnaryOp,
    VarArgList,
    While,
    With,
    Yield,
};


struct Ast {
    AstType type;
    uint32_t line;
    uint32_t column;
};
typedef std::shared_ptr<Ast> AstPtr;
typedef std::vector<AstPtr> AstLst;

template<AstType Type, typename Base = Ast>
struct AstT : Base {
    static constexpr AstType TYPE = Type;
    AstT() : Base{Type, 0, 0} {}
};

template<AstType Type, typename Base>
constexpr AstType AstT<Type, Base>::TYPE;

struct AstExpression : AstT<AstType::Expression> {};
typedef std::shared_ptr<AstExpression> AstExpr;
typedef std::vector<AstExpr> AstExprList;


template<AstType Type>
struct AstExprT : AstT<Type, AstExpression> {};

struct AstStatement : AstT<AstType::Statement> {};
typedef std::shared_ptr<AstStatement> AstStmt;
typedef std::vector<AstStmt> AstStmtList;

template<AstType Type>
struct AstStmtT : AstT<Type, AstStatement> {};

struct AstArguments : AstT<AstType::Arguments> {
    AstExprList arguments;
    AstExprList defaults;
    String kwargs;
    String args;
};

struct AstKeyword : AstT<AstType::Keyword> {
    String name;
    AstExpr value;
};
typedef std::shared_ptr<AstKeyword> AstKeywordPtr;
typedef std::vector<AstKeywordPtr> AstKeywordList;

struct AstAlias : AstT<AstType::Alias> {
    String name;
    String as_name;
};
typedef std::vector<AstAlias> AstAliasList;

struct AstComprehension : AstT<AstType::Comprehension> {
    AstExpr target;
    AstExpr iter;
    AstExprList ifs;
};
typedef std::vector<AstComprehension> AstComprList;

struct AstSliceKind : Ast {};
typedef std::shared_ptr<AstSliceKind> AstSlicePtr;

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

struct AstEllipsis : AstSliceT<AstType::Ellipsis> {};

struct AstSlice : AstSliceT<AstType::Slice> {
    AstExpr lower;
    AstExpr upper;
    AstExpr step;
};

struct AstExtSlice : AstSliceT<AstType::ExtSlice> {
    AstExprList dims;
};

struct AstIndex : AstSliceT<AstType::Index> {
    AstExpr value;
};

struct AstModule : AstT<AstType::Module> {
    AstStmtList body;
};

#define PYPA_AST_STMT(AST_TYPE) struct Ast##AST_TYPE : AstStmtT<AstType::AST_TYPE>
#define PYPA_AST_EXPR(AST_TYPE) struct Ast##AST_TYPE : AstExprT<AstType::AST_TYPE>

PYPA_AST_STMT(Assert) {
    AstExpr expression;
    AstExpr test;
};

PYPA_AST_STMT(Assign) {
    AstExprList targets;
    AstExpr     value;
};

PYPA_AST_STMT(AugAssign) {
    AstExpr         value;
    AstExpr         target;
    AstBinOpType    op;
};

PYPA_AST_STMT(Attribute) {
    AstExpr     value;
    AstContext  context;
    String      attribute;
};

PYPA_AST_EXPR(BinOp) {
    AstBinOpType    op;
    AstExpr         left;
    AstExpr         right;
};

PYPA_AST_EXPR(BoolOp) {
    AstType     op;
    AstExprList values;
};

PYPA_AST_STMT(Break) {};

PYPA_AST_STMT(Call) {
    AstExpr         function;
    AstExpr         args;
    AstExpr         kwargs;
    AstExprList     arguments;
    AstKeywordList  keywords;
};

PYPA_AST_STMT(Compare) {
    std::vector<AstCompareType> ops;
    AstExprList                 comperators;
    AstExpr                     left;
};

PYPA_AST_STMT(ClassDef) {
    AstExprList bases;
    AstExprList decorators;
    AstStmtList body;
    String      name;
};

PYPA_AST_STMT(Continue) {};

PYPA_AST_EXPR(Dict) {
    AstExprList keys;
    AstExprList values;
};

PYPA_AST_EXPR(DictComp) {
    AstExpr         key;
    AstExpr         value;
    AstComprList    generators;
};

PYPA_AST_STMT(Delete) {
    AstExprList targets;
};

PYPA_AST_STMT(Exec) {
    AstExpr body;
    AstExpr globals;
    AstExpr locals;
};

PYPA_AST_STMT(Except) {
    AstStmtList body;
    AstExpr type;
    AstExpr name;
};
typedef std::shared_ptr<AstExcept> AstExceptPtr;
typedef std::vector<AstExceptPtr> AstExceptList;

PYPA_AST_STMT(ExpressionStatement) {
    AstExpr expr;
};

PYPA_AST_STMT(For) {
    AstStmtList body;
    AstStmtList orelse;
    AstExpr     target;
    AstExpr     iter;
};

PYPA_AST_STMT(FunctionDef) {
   String       name;
   AstArguments args;
   AstStmtList  body;
   AstExprList  decorators;
};

PYPA_AST_STMT(Global) {
    StringList names;
};

PYPA_AST_STMT(If) {
    AstStmtList body;
    AstStmtList orelse;
    AstExpr     test;
};

PYPA_AST_EXPR(IfExpr) {
    AstExpr body;
    AstExpr orelse;
    AstExpr test;
};

PYPA_AST_STMT(Import) {
    AstAliasList names;
};

PYPA_AST_STMT(ImportFrom) {
    String          module;
    AstAliasList    names;
    int             level;
};

PYPA_AST_EXPR(Lambda) {
    AstArguments    arguments;
    AstExpr         body;
};

PYPA_AST_EXPR(List) {
    AstExprList elements;
    AstContext  context;
};

PYPA_AST_EXPR(ListComp) {
    AstComprList generators;
    AstExpr      element;
};

PYPA_AST_EXPR(Name) {
    AstContext  context;
    String      id;
};

PYPA_AST_EXPR(Number) {
    enum Type {
        Integer,
        Float
    };
    union {
        double  floating;
        int64_t integer;
    };
};

PYPA_AST_EXPR(Repr) {
    AstExpr value;
};

PYPA_AST_STMT(Pass) {};

PYPA_AST_STMT(Print) {
    AstExpr         destination;
    bool            newline;
    AstExprList     values;
};

PYPA_AST_STMT(Raise) {
    AstExpr type;
    AstExpr instance;
    AstExpr traceback;
};

PYPA_AST_STMT(Return) {
    AstExpr value;
};

PYPA_AST_EXPR(Set) {
    AstExprList elements;
};

PYPA_AST_EXPR(SetComp) {
    AstExpr      element;
    AstComprList generators;
};

PYPA_AST_EXPR(Str) {
    String value;
    AstContext context;
};

PYPA_AST_EXPR(Subscript) {
    AstExpr     value;
    AstSlicePtr slice;
    AstContext  context;
};

PYPA_AST_STMT(TryExcept) {
    AstStmtList     body;
    AstStmtList     orelse;
    AstExceptList   handlers;
};

PYPA_AST_STMT(TryFinally) {
    AstStmtList body;
    AstStmtList final_body;
};

PYPA_AST_EXPR(Tuple) {
    AstExprList elements;
    AstContext  context;
};

PYPA_AST_EXPR(UnaryOp) {
    AstExpr         operand;
    AstUnaryOpType  op;
};

PYPA_AST_STMT(While) {
    AstExpr     test;
    AstStmtList body;
    AstStmtList orelse;
};

PYPA_AST_STMT(With) {
    AstExpr     vars;
    AstExpr     context;
    AstStmtList body;
};

PYPA_AST_STMT(Yield) {
    AstExpr value;
};

}
#endif // GUARD_PYPA_AST_AST_HH_INCLUDED
