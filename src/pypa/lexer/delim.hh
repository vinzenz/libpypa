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
#ifndef GUARD_PYPA_TOKENIZER_DELIM_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_DELIM_HH_INCLUDED

#include <pypa/lexer/tokendef.hh>
#include <cstddef>
#include <stdexcept>

namespace pypa {
    TokenDef DelimTokens[] = {
        TokenDef(Token::DelimBraceOpen, TokenString("{"), TokenKind::LeftBrace, TokenClass::Delimiter),
        TokenDef(Token::DelimBraceClose, TokenString("}"), TokenKind::RightBrace, TokenClass::Delimiter),
        TokenDef(Token::DelimComma, TokenString(","), TokenKind::Comma, TokenClass::Delimiter),
        TokenDef(Token::DelimColon, TokenString(":"), TokenKind::Colon, TokenClass::Delimiter),
        TokenDef(Token::DelimPeriod, TokenString("."), TokenKind::Dot, TokenClass::Delimiter),
        TokenDef(Token::DelimSemiColon, TokenString(";"), TokenKind::SemiColon, TokenClass::Delimiter),
        TokenDef(Token::DelimBracketOpen, TokenString("["), TokenKind::LeftBracket, TokenClass::Delimiter),
        TokenDef(Token::DelimBracketClose, TokenString("]"), TokenKind::RightBracket, TokenClass::Delimiter),
        TokenDef(Token::DelimParenOpen, TokenString("("), TokenKind::LeftParen, TokenClass::Delimiter),
        TokenDef(Token::DelimParenClose, TokenString(")"), TokenKind::RightParen, TokenClass::Delimiter),
        TokenDef(Token::DelimAt, TokenString("@"), TokenKind::At, TokenClass::Delimiter)
    };

    inline ConstArray<TokenDef const> Delims() {
        return { DelimTokens };
    }
}
#endif //GUARD_PYPA_TOKENIZER_DELIM_HH_INCLUDED

