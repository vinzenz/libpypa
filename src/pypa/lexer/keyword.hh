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
#ifndef GUARD_PYPA_TOKENIZER_KEYWORD_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_KEYWORD_HH_INCLUDED

#include <pypa/lexer/tokens.hh>
#include <pypa/lexer/tokendef.hh>


namespace pypa {
    static const TokenDef KeywordTokens[] = {
        TokenDef(Token::KeywordAnd, TokenString("and"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordAs, TokenString("as"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordAssert, TokenString("assert"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordBreak, TokenString("break"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordClass, TokenString("class"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordContinue, TokenString("continue"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordDef, TokenString("def"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordDel, TokenString("del"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordElIf, TokenString("elif"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordElse, TokenString("else"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordExcept, TokenString("except"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordExec, TokenString("exec"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordFinally, TokenString("finally"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordFor, TokenString("for"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordFrom, TokenString("from"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordGlobal, TokenString("global"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordIf, TokenString("if"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordImport, TokenString("import"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordIn, TokenString("in"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordIs, TokenString("is"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordLambda, TokenString("lambda"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordNonLocal, TokenString("nonlocal"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordNot, TokenString("not"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordOr, TokenString("or"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordPass, TokenString("pass"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordRaise, TokenString("raise"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordReturn, TokenString("return"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordTry, TokenString("try"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordWhile, TokenString("while"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordWith, TokenString("with"), TokenKind::Name, TokenClass::Keyword),
        TokenDef(Token::KeywordYield, TokenString("yield"), TokenKind::Name, TokenClass::Keyword),
        // Starting with capital
        // TokenDef(Token::KeywordFalse, TokenString("False"), TokenKind::Name, TokenClass::Keyword),
        // TokenDef(Token::KeywordNone, TokenString("None"), TokenKind::Name, TokenClass::Keyword),
        // TokenDef(Token::KeywordTrue, TokenString("True"), TokenKind::Name, TokenClass::Keyword)
    };

    inline ConstArray<TokenDef const> Keywords() {
        return { KeywordTokens };
    }
}
#endif // GUARD_PYPA_TOKENIZER_KEYWORD_HH_INCLUDED
