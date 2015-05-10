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
#ifndef GUARD_PYPA_TOKENIZER_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_HH_INCLUDED

namespace pypa {
    enum class Token {
        Invalid,
        End,
        NewLine,
        Indent,
        Dedent,

        NotMultipleOfFourIndentError,
        MixedIndentError,
        IndentationError,
        DedentationError,
        LineContinuationError,
        UnterminatedStringError,
        EncodingError,

        Comment,
        BackQuote,
        Identifier,
        NumberFloat,
        NumberInteger,
        NumberHex,
        NumberOct,
        NumberBinary,
        NumberComplex,
        String,

        KeywordAnd,
        KeywordAs,
        KeywordAssert,
        KeywordBreak,
        KeywordClass,
        KeywordContinue,
        KeywordDef,
        KeywordDel,
        KeywordElIf,
        KeywordElse,
        KeywordExcept,
        KeywordExec,
        //KeywordFalse,
        KeywordFinally,
        KeywordFor,
        KeywordFrom,
        KeywordGlobal,
        KeywordIf,
        KeywordImport,
        KeywordIn,
        KeywordIs,
        KeywordLambda,
        KeywordNonLocal,
        KeywordNone,
        KeywordNot,
        KeywordOr,
        KeywordPass,
        KeywordRaise,
        KeywordReturn,
        //KeywordTrue,
        KeywordTry,
        KeywordWhile,
        KeywordWith,
        KeywordYield,

        Ellipsis,

        OpAdd,
        OpSub,
        OpMul,
        OpDiv,
        OpExp,
        OpMod,
        OpAnd,
        OpOr,
        OpXor,
        OpDivDiv,
        OpInv,
        OpShiftLeft,
        OpShiftRight,
        OpEqual,
        OpNotEqual,
        OpMoreEqual,
        OpLessEqual,
        OpMore,
        OpLess,
        OpAssign,
        OpAddAssign,
        OpSubAssign,
        OpMulAssign,
        OpDivAssign,
        OpDivDivAssign,
        OpModAssign,
        OpAndAssign,
        OpOrAssign,
        OpXorAssign,
        OpShiftLeftAssign,
        OpShiftRightAssign,
        OpExpAssign,

        DelimBraceOpen,
        DelimBraceClose,
        DelimComma,
        DelimColon,
        DelimPeriod,
        DelimSemiColon,
        DelimBracketOpen,
        DelimBracketClose,
        DelimParenOpen,
        DelimParenClose,
        DelimAt,
        DelimArrow,
    };
}

#endif //GUARD_PYPA_TOKENIZER_HH_INCLUDED

