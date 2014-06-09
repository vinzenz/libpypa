#ifndef GUARD_PYPA_TOKENIZER_OP_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_OP_HH_INCLUDED

#include <pypa/lexer/tokendef.hh>
#include <cstddef>
#include <stdexcept>

namespace pypa {
    TokenDef OpTokens[] = {
        TokenDef(Token::OpAddAssign, TokenString("+="), TokenKind::PlusEqual, TokenClass::Operator),
        TokenDef(Token::OpSubAssign, TokenString("-="), TokenKind::MinusEqual, TokenClass::Operator),
        TokenDef(Token::OpMulAssign, TokenString("*="), TokenKind::StarEqual, TokenClass::Operator),
        TokenDef(Token::OpDivAssign, TokenString("/="), TokenKind::SlashEqual, TokenClass::Operator),
        TokenDef(Token::OpDivDivAssign, TokenString("//="), TokenKind::DoubleSlashEqual, TokenClass::Operator),
        TokenDef(Token::OpModAssign, TokenString("%="), TokenKind::PercentEqual, TokenClass::Operator),
        TokenDef(Token::OpAndAssign, TokenString("&="), TokenKind::BinAndEqual, TokenClass::Operator),
        TokenDef(Token::OpOrAssign, TokenString("|="), TokenKind::BinOrEqual, TokenClass::Operator),
        TokenDef(Token::OpXorAssign, TokenString("^="), TokenKind::CircumFlexEqual, TokenClass::Operator),
        TokenDef(Token::OpShiftLeftAssign, TokenString("<<="), TokenKind::LeftShiftEqual, TokenClass::Operator),
        TokenDef(Token::OpShiftRightAssign, TokenString(">>="), TokenKind::RightShiftEqual, TokenClass::Operator),
        TokenDef(Token::OpExpAssign, TokenString("**="), TokenKind::DoubleStarEqual, TokenClass::Operator),
        TokenDef(Token::OpExp, TokenString("**"), TokenKind::DoubleStar, TokenClass::Operator),
        TokenDef(Token::OpDivDiv, TokenString("//"), TokenKind::DoubleSlash, TokenClass::Operator),
        TokenDef(Token::OpInv, TokenString("~"), TokenKind::Tilde, TokenClass::Operator),
        TokenDef(Token::OpShiftLeft, TokenString("<<"), TokenKind::LeftShift, TokenClass::Operator),
        TokenDef(Token::OpShiftRight, TokenString(">>"), TokenKind::RightShift, TokenClass::Operator),
        TokenDef(Token::OpEqual, TokenString("=="), TokenKind::EqualEqual, TokenClass::Operator),
        TokenDef(Token::OpNotEqual, TokenString("!="), TokenKind::NotEqual, TokenClass::Operator),
        TokenDef(Token::OpNotEqual, TokenString("<>"), TokenKind::NotEqual, TokenClass::Operator),
        TokenDef(Token::OpMoreEqual, TokenString(">="), TokenKind::GreaterEqual, TokenClass::Operator),
        TokenDef(Token::OpLessEqual, TokenString("<="), TokenKind::LessEqual, TokenClass::Operator),
        TokenDef(Token::OpAssign, TokenString("="), TokenKind::Equal, TokenClass::Operator),
        TokenDef(Token::OpMore, TokenString(">"), TokenKind::Greater, TokenClass::Operator),
        TokenDef(Token::OpLess, TokenString("<"), TokenKind::Less, TokenClass::Operator),
        TokenDef(Token::OpAdd, TokenString("+"), TokenKind::Plus, TokenClass::Operator),
        TokenDef(Token::OpSub, TokenString("-"), TokenKind::Minus, TokenClass::Operator),
        TokenDef(Token::OpMul, TokenString("*"), TokenKind::Star, TokenClass::Operator),
        TokenDef(Token::OpDiv, TokenString("/"), TokenKind::Slash, TokenClass::Operator),
        TokenDef(Token::OpMod, TokenString("%"), TokenKind::Percent, TokenClass::Operator),
        TokenDef(Token::OpAnd, TokenString("&"), TokenKind::BinAnd, TokenClass::Operator),
        TokenDef(Token::OpOr, TokenString("|"), TokenKind::BinOr, TokenClass::Operator),
        TokenDef(Token::OpXor, TokenString("^"), TokenKind::CircumFlex, TokenClass::Operator),
        TokenDef(Token::DelimArrow, TokenString("->"), TokenKind::Arrow, TokenClass::Default)
    };

    inline ConstArray<TokenDef const> Ops() {
        return { OpTokens };
    }
}
#endif //GUARD_PYPA_TOKENIZER_OP_HH_INCLUDED

