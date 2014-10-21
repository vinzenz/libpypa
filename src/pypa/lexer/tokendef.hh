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
#ifndef GUARD_PYPA_TOKENIZER_TOKENDEF_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_TOKENDEF_HH_INCLUDED

#include <pypa/lexer/tokens.hh>
#include <cstddef>
#include <stdexcept>

namespace pypa {

    template< typename ValueType, size_t InitSizeDiff = 0>
    class ConstArray {
    private:
            ValueType * const data_;
            const std::size_t size_;
    public:
        template<std::size_t N>
        ConstArray(ValueType(&data)[N])
        : data_(data)
        , size_(N-InitSizeDiff)
        {}

        char operator[](std::size_t index) const {
            return index < size_ ? data_[index] : throw std::out_of_range("");
        }

        std::size_t size() const {
            return size_;
        }

        ValueType const * data() const {
            return data_;
        }

        bool empty() const {
            return size_ == 0;
        }

        ValueType const * begin() const {
            return data();
        }

        ValueType const * end() const {
            return data() + size();
        }
    };

    class TokenString : public ConstArray<char const, 1>
    {
    public:
        template<std::size_t N>
        TokenString(char const (&u)[N])
        : ConstArray(u)
        {}

        char const * c_str() const {
            return data();
        }

        bool empty() const {
            return size() == 0 || data()[0] == '\0';
        }
    };
    enum class TokenClass {
        Default,
        Operator,
        Keyword,
        Delimiter,
        Literal
    };

    enum class TokenKind {
        End             = 0,
        Name            = 1,
        Number          = 2,
        String          = 3,
        NewLine         = 4,
        Indent          = 5,
        Dedent          = 6,
        LeftParen       = 7,
        RightParen      = 8,
        LeftBracket     = 9,
        RightBracket    = 10,
        Colon           = 11,
        Comma           = 12,
        SemiColon       = 13,
        Plus            = 14,
        Minus           = 15,
        Star            = 16,
        Slash           = 17,
        BinOr           = 18,
        BinAnd          = 19,
        Less            = 20,
        Greater         = 21,
        Equal           = 22,
        Dot             = 23,
        Percent         = 24,
        BackQuote       = 25,
        LeftBrace       = 26,
        RightBrace      = 27,
        EqualEqual      = 28,
        NotEqual        = 29,
        LessEqual       = 30,
        GreaterEqual    = 31,
        Tilde           = 32,
        CircumFlex      = 33,
        LeftShift       = 34,
        RightShift      = 35,
        DoubleStar      = 36,
        PlusEqual       = 37,
        MinusEqual      = 38,
        StarEqual       = 39,
        SlashEqual      = 40,
        PercentEqual    = 41,
        BinAndEqual     = 42,
        BinOrEqual      = 43,
        CircumFlexEqual = 44,
        LeftShiftEqual  = 45,
        RightShiftEqual = 46,
        DoubleStarEqual = 47,
        DoubleSlash     = 48,
        DoubleSlashEqual= 49,
        At              = 50,
        Operator        = 51,
        Error           = 52,

        // Let this be at the end
        KindCount,
        Arrow
    };

    class TokenIdent {
    private:
        Token id_;
        TokenKind kind_;
        TokenClass cls_;
    public:
        TokenIdent(Token id, TokenKind kind, TokenClass cls)
        : id_(id)
        , kind_(kind)
        , cls_(cls)
        {}

        TokenIdent()
        : TokenIdent(Token::Invalid, TokenKind::Error, TokenClass::Default)
        {}

        Token id() const {
            return id_;
        }

        TokenKind kind() const {
            return kind_;
        }

        TokenClass cls() const {
            return cls_;
        }
    };

    class TokenDef {
    private:
        TokenIdent ident_;
        TokenString value_;
    public:
        TokenDef(Token id, TokenString value, TokenKind kind, TokenClass cls)
        : ident_(id, kind, cls)
        , value_(value)
        {}

        TokenIdent ident() const {
            return ident_;
        }

        TokenString value() const {
            return value_;
        }

        inline bool match3(char c0, char c1, char c2) const {
            return (value().size() == 3 && (c0 == value()[0] && c1 == value()[1] && c2 == value()[2]))
                || (value().size() == 2 && (c0 == value()[0] && c1 == value()[1]))
                || (value().size() == 1 && (c0 == value()[0]));
        }

        inline bool match2(char c0, char c1) const {
            return (value().size() == 2 && (c0 == value()[0] && c1 == value()[1]))
                || (value().size() == 1 && (c0 == value()[0]));
        }
    };
}
#endif // GUARD_PYPA_TOKENIZER_TOKENDEF_HH_INCLUDED

