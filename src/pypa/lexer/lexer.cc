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
#include <stack>
#include <cassert>
#include <fstream>

#include <pypa/lexer/lexer.hh>
#include <pypa/lexer/op.hh>
#include <pypa/lexer/keyword.hh>
#include <pypa/lexer/delim.hh>
#include <pypa/filebuf.hh>

namespace pypa {
    inline bool is_ident_char(char c, bool first = false) {
        return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c == '_')
            || (!first && (c >= '0' && c <= '9'));
    }

    inline bool is_digit(char c) {
        return (c >= '0' && c <= '9');
    }

    inline bool is_hex(char c) {
        return is_digit(c)
            || (c >= 'A' && c <= 'F')
            || (c >= 'a' && c <= 'f');
    }

    inline char hex2num(char c) {
        if(!is_hex(c)) {
            return '\xff';
        }
        if(c >= '0' && c <= '9') {
            return c - '0';
        }
        c = tolower(c);
        return (c - 'a') + 10;
    }

    inline bool is_number(char c0, char c1, char c2) {
        return is_digit(c0)
            || (c0 == '.' && is_digit(c1))
            || (c0 == '-' && is_digit(c1))
            || (c0 == '-' && c1 == '.' && is_digit(c2))
            ;
    }

    inline bool is_coding_char(char c) {
        return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9')
            || (c == '-')
            || (c == '_')
            || (c == '.');
    }

    std::string Lexer::get_name() const {
        return reader_->get_filename();
    }

    std::string Lexer::get_line(int idx) {
        return reader_->get_line(idx);
    }

    Lexer::Lexer(char const * file_path)
    : Lexer(std::unique_ptr<FileBufReader>(new FileBufReader(file_path)))
    {}

    Lexer::Lexer(std::unique_ptr<Reader> reader)
    : reader_(std::move(reader))
    , read_encoding_{false}
    , encoding_{"iso-8859-1"}
    , column_{0}
    , level_{0}
    , indent_{0}
    , indent_stack_{0}
    , alt_indent_stack_{0}
    , lex_buffer_{}
    , info_{}
    , first_indet_char{0}
    , token_buffer_{}
    , ignore_altindent_errors_{true}
    {}

    Lexer::~Lexer(){}

    std::list<LexerInfo> const & Lexer::info() {
        return info_;
    }

    TokenInfo Lexer::next() {
        char c0 = skip();
        if(!token_buffer_.empty()) {
            put_char(c0);
            TokenInfo tmp = token_buffer_.front();
            token_buffer_.pop_front();
            return tmp;
        }
        TokenInfo tok = {{
            (reader_->eof() ? Token::End : Token::Invalid),
            (reader_->eof() ? TokenKind::End : TokenKind::Error),
            TokenClass::Default},
            line(), column_, {} };
        char c1 = next_char();
        char c2 = next_char();
        // Pushing c2 first, is on purpose, think of the order!
        put_char(c2);
        put_char(c1);

        tok.value.clear();
        char ctmp = 0;
        switch (c0) {
        case '"': case '\'':
            return get_string(tok, c0);
        case 'r': case 'b': case 'u':
        case 'R': case 'B': case 'U':
            ctmp = c1;
            if((c1 == 'r' || c1 == 'R') && (c0 == 'u' || c0 == 'b' || c0 == 'U' || c0 == 'B')) {
                ctmp = c2;
            }
            if(ctmp == '\'' || ctmp == '"')
                return get_string(tok, next_char(), c0);
        }

        if (is_ident_char(c0, true)) {
            char c = c0;
            do {
                tok.value.push_back(c);
            } while (is_ident_char(c = next_char()));
            put_char(c);
            for (auto const & kw : Keywords()) {
                if (kw.value().size() == tok.value.size()) {
                    if (kw.value().c_str() == tok.value) {
                        return make_token(tok, kw.ident());
                    }
                }
            }
            return make_token(tok, Token::Identifier, TokenKind::Name);
        }
        for(auto const & delim : Delims()) {
            if(delim.value()[0] == c0) {
                tok.value = delim.value().c_str();
                bool abort = false;
                switch(c0) {
                case '[': case '{': case '(':
                    ++level_;
                    break;
                case ']': case '}': case ')':
                    --level_;
                    break;
                case '.': case '-': case '+':
                    if(isdigit(c1)) {
                        abort = true;
                    }
                    break;
                }
                if(abort) break;
                return make_token(tok, delim.ident());
            }
        }
        for(auto const & op : Ops()) {
            if(op.match3(c0, c1, c2)) {
                if (op.value().size() > 1) next_char();
                if(op.value().size() > 2) next_char();
                tok.value = op.value().c_str();
                return make_token(tok, op.ident());
            }
        }

        if (is_number(c0, c1, c2)) {
            return get_number(tok, c0);
        }

        return tok;
    }

    TokenInfo Lexer::get_string(TokenInfo & tok, char first, char prefix) {
        // bool binary = prefix == 'b';
        // bool unicode = prefix == 'u';
        // bool raw = prefix == 'r';
        char cur = first;
        int quote_count = 0;
        int end_quote_count = 0;
        if(prefix) {
            tok.value.push_back(prefix);
            if(first == 'b' || first == 'r') {
                tok.value.push_back(first);
                cur = first = next_char();
            }
        }
        while(cur == first && quote_count < 3) {
            ++quote_count;
            tok.value.push_back(cur);
            cur = next_char();
        }
        put_char(cur);
        if(quote_count != 2 && quote_count != 6) {
            while(end_quote_count != quote_count) {
                cur = next_char();
                if(cur == -1 && reader_->eof()) {
                    return make_token(tok,
                            Token::UnterminatedStringError,
                            TokenKind::Error);
                }
                tok.value.push_back(cur);
                if(cur == first) {
                    ++end_quote_count;
                }
                else {
                    end_quote_count = 0;
                    if(cur == '\n' || cur == '\x0c' || cur == '\r') {
                        if(quote_count == 1) {
                            put_char(cur);
                            return make_token(tok,
                                    Token::UnterminatedStringError,
                                    TokenKind::Error);
                        }
                        if(cur == '\r') continue;
                    }
                    if (cur == '\\') {
                        char c = next_char();
                        if (c == '\r')
                            tok.value.push_back(next_char());
                        else
                            tok.value.push_back(c);
                    }
                }
            };
        }

        return make_token(tok, Token::String, TokenKind::String,
                          TokenClass::Literal);
    }

    TokenInfo Lexer::get_number_binary(TokenInfo & tok, char first) {
        do {
            tok.value.push_back(first);
            first = next_char();
        } while (first == '0' || first == '1');
        put_char(first);
        tok.ident = {Token::NumberBinary, TokenKind::Number, TokenClass::Literal};
        if (tok.value.size() <= 2) {
            tok.ident = {Token::Invalid, TokenKind::Error, TokenClass::Default};
            add_error(tok);
        }
        tok.value.erase(0, 2);
        return tok;
    }

    TokenInfo Lexer::get_number_hex(TokenInfo & tok, char first) {
        do {
            tok.value.push_back(first);
            first = next_char();
        } while (is_hex(first));
        put_char(first);
        tok.ident = {Token::NumberHex, TokenKind::Number, TokenClass::Literal};
        if (tok.value.size() <= 2) {
            tok.ident = {Token::Invalid, TokenKind::Error, TokenClass::Default};
            add_error(tok);
        }
        tok.value.erase(0, 2);
        return tok;
    }

    TokenInfo Lexer::get_number_octal(TokenInfo & tok, char first) {
        if(first != 'o' && first != 'O' && !(first >= '0' && first < '8')) {
            put_char(first);
            tok.ident = {Token::NumberInteger, TokenKind::Number, TokenClass::Literal};
            return tok;
        }
        tok.value.push_back(first);
        first = next_char();
        while (first >= '0' && first < '8') {
            tok.value.push_back(first);
            first = next_char();
        }
        bool non_oct = false;
        if (is_digit(first)) {
            non_oct = true;
            do {
                tok.value.push_back(first);
            }
            while (is_digit(first = next_char()));
        }
        if (first == '.' || first == 'e' || first == 'E') {
            return get_number_float(tok, first);
        }
        if (first == 'j' || first == 'J') {
            return get_number_complex(tok, first);
        }
        put_char(first);
        tok.ident = {Token::NumberOct, TokenKind::Number, TokenClass::Literal};
        if (non_oct) {
            tok.ident = {Token::Invalid, TokenKind::Error, TokenClass::Default};
            add_error(tok);
        }
        if(tok.value.size() >= 2) {
            if(tok.value[1] == 'o' || tok.value[1] == 'O') {
                tok.value.erase(0, 2);
            }
        }
        return tok;
    }

    TokenInfo Lexer::get_number_float(TokenInfo & tok, char first) {
        switch (first) {
        case '.':
            do {
                tok.value.push_back(first);
            }
            while (is_digit(first = next_char()));
            if(first == 'j') {
                tok.value.push_back(first);
                tok.ident = {Token::NumberComplex, TokenKind::Number, TokenClass::Literal};
                break;
            }
            else if (first != 'e' && first != 'E') {
                put_char(first);
                tok.ident = {Token::NumberFloat, TokenKind::Number, TokenClass::Literal};
                break;
            }
            // fallthrough
        case 'e': case 'E':
            tok.ident = {Token::NumberFloat, TokenKind::Number, TokenClass::Literal};
            tok.value.push_back(first);
            first = next_char();
            if (is_digit(first) || first == '-' || first == '+') {
                do {
                    tok.value.push_back(first);
                } while (is_digit(first = next_char()));
                if(first == 'j' || first == 'J') {
                    tok.ident = {Token::NumberComplex, TokenKind::Number, TokenClass::Literal};
                }
                else {
                    put_char(first);
                }
            }
            else {
                put_char(first);
            }
        }
        return tok;
    }

    TokenInfo Lexer::get_number_integer(TokenInfo & tok, char first) {
        tok.value.push_back(first);
        while (is_digit(first = next_char())) {
            tok.value.push_back(first);
        }
        if (first == '.' || first == 'e' || first == 'E') {
            return get_number_float(tok, first);
        }
        if (first == 'j' || first == 'J') {
            return get_number_complex(tok, first);
        }
        put_char(first);
        return make_token(tok, Token::NumberInteger, TokenKind::Number);
    }

    TokenInfo Lexer::get_number_complex(TokenInfo & tok, char first) {
        tok.value.push_back(first);
        return make_token(tok, Token::NumberComplex, TokenKind::Number);
    }

    TokenInfo Lexer::get_number(TokenInfo & tok, char first) {
        tok.value.clear();
        if(first == '-') {
            tok.value.push_back(first);
            first = next_char();
        }
        if (first == '0') {
            tok.value.push_back(first);
            char c1 = next_char();
            switch (c1) {
            case 'b': case 'B':
                return get_number_binary(tok, c1);
            case 'x': case 'X':
                return get_number_hex(tok, c1);
            case 'o': case 'O':
                return get_number_octal(tok, c1);
            case 'j': case 'J':
                return get_number_complex(tok, c1);
            case '.': case 'e': case 'E':
                return get_number_float(tok, c1);
            }
            return get_number_octal(tok, c1);
        }
        if (first == '.') {
            return get_number_float(tok, first);
        }
        return get_number_integer(tok, first);
    }

    char Lexer::next_char() {
        column_++;
        if (lex_buffer_.empty()) {
            std::string line = reader_->next_line();
            if (line.empty() && reader_->eof())
                return -1;
            lex_buffer_.insert(lex_buffer_.end(), line.begin(), line.end());
        }
        char c = lex_buffer_.front();
        lex_buffer_.pop_front();
        return c;
    }

    void Lexer::put_char(char c) {
        column_--;
        lex_buffer_.push_front(c);
    }

    char Lexer::skip_comment() {
        char c = 0;
        do {
            c = next_char();
        } while(!reader_->eof() && c != '\n');
        return c;
    }

    char Lexer::skip_comment_check_coding() {
        std::string line_str;
        char c = 0;
        do {
            c = next_char();
            line_str.push_back(c);
        } while(!reader_->eof() && c != '\n');

        size_t pos = 0;
        for(;;) {
            pos = line_str.find("coding", pos);

            if(pos == std::string::npos)
                break;

            pos += 6 /* = strlen("coding")*/;

            if(line_str[pos] != ':' && line_str[pos] != '=')
                continue;

            do {
                pos++;
            } while(line_str[pos] == '\x20' || line_str[pos] == '\t');

            std::string coding;
            while(is_coding_char(line_str[pos])) {
                coding.push_back(line_str[pos]);
                ++pos;
            }

            if (!coding.empty()) {
                encoding_ = coding;
                if(!reader_->set_encoding(coding)) {
                    token_buffer_.push_back({
                        {Token::EncodingError,
                         TokenKind::Error,
                         TokenClass::Default},
                        line(),
                        column_,
                        "encoding problem: " + coding});
                }
                read_encoding_ = true;
                break;
            }
        }
        return c;
    }

    char Lexer::skip() {
        char c = -1;
        bool at_bol = false;
        bool continuation = false;
        for(;;) {
            c = next_char();
            switch(c) {
            case '#':
                if(!read_encoding_ && line() < 3)
                    c = skip_comment_check_coding();
                else
                    c = skip_comment();
                if(reader_->eof()) {
                    return c;
                }
                // It should now continue with newline
                put_char(c);
                break;
            case '\\':
                c = next_char();
                continuation = true;
                if(c == 'x') {
                    put_char(c);
                    // decode hex
                    std::stack<char> decoded;
                    do {
                        c = next_char();
                        if(c != 'x') break;
                        char result = 0;
                        c = next_char();
                        if(!is_hex(c)) {
                            put_char(c);
                            c = 'x';
                            break;
                        }
                        result = hex2num(c) << 4;
                        char c1 = next_char();
                        if(!is_hex(c1)) {
                            put_char(c1);
                            put_char(c);
                            c = 'x';
                            break;
                        }
                        result |= hex2num(c1);
                        decoded.push(result);
                    } while((c = next_char()) == '\\');

                    while(!decoded.empty()) {
                        put_char(decoded.top());
                        decoded.pop();
                    }
                }
                if(c == '\r') c = next_char();
                if(c != '\n' && c != '\x0c') {
                    token_buffer_.push_back({
                            {Token::LineContinuationError, TokenKind::Error, TokenClass::Default},
                            line(), column_, {"LineContinationError"}});
                    return c;
                }
                // It should now continue with newline
                put_char(c);
                break;
            case '`':
                token_buffer_.push_back({
                        {Token::BackQuote, TokenKind::BackQuote, TokenClass::Default},
                        line(), column_,
                        {"BackQuote"}});
                return next_char();
            case ' ':
            case '\t':
                at_bol = column_ == 1;
                handle_whitespace(c);
                if(at_bol) {
                    put_char(c);
                    if(!handle_indentation()) {
                        return next_char();
                    }
                }
                else {
                }
                break;
            case '\x0c': // Allow formfeed as \n
            case '\n':
                column_ = 0;
                if(!continuation && level_ == 0) {
                    token_buffer_.push_back({
                            { Token::NewLine, TokenKind::NewLine, TokenClass::Default },
                            line(), column_,
                            {"NewLine"}});
                }
                if(!handle_indentation(continuation)) {
                    return next_char();
                }
                continuation = false;
                break;
            case '\r':
                break; // we're ignoring \r
            default:
                return c;
            }
        }
        return c;
    }

    void Lexer::handle_whitespace(char c) {
        if (!first_indet_char) {
            first_indet_char = c;
        }
        if (first_indet_char != c) {
            add_warning(make_invalid(&c, &c+1, Token::MixedIndentError));
        }
        if(c == '\t') {
            column_ += 7;
        }
    }

    bool Lexer::handle_indentation(bool continuation) {
        const int tab_size = 8;
        int col = 0, alt_col = 0;
        char c = 0;
        int changes = 0;
        for(;;) {
            c = next_char();
            if(c == ' ') {
                ++col; ++alt_col;
            }
            else if(c == '\t') {
                col = (col/tab_size+1)*tab_size;
                alt_col += 1;
            }
            else {
                 break;
            }
        }
        put_char(c);
        if(continuation || level_ != 0 || c == '#' || c == '\n' || c == '\r' || c == '\x0c') {
            if(c == '#' || c == '\n') {
                // If this line is a commented line or an empty line, don't emit NewLine
                if(!token_buffer_.empty() && token_buffer_.back().ident.id() == Token::NewLine) {
                    token_buffer_.pop_back();
                }
            }
            return true;
        }

        if(col == indent_stack_[indent_]) {
            if(!ignore_altindent_errors_ && alt_col != alt_indent_stack_[indent_]) {
                return add_indent_error();
            }
        }
        else if(col > indent_stack_[indent_]) {
            if(!ignore_altindent_errors_ && alt_col <= alt_indent_stack_[indent_]) {
                return add_indent_error();
            }
            ++indent_;
            ++changes;
            assert(indent_stack_.size() >= size_t(indent_));
            if(indent_stack_.size() == size_t(indent_)) {
                alt_indent_stack_.push_back(alt_col);
                indent_stack_.push_back(col);
            }
            else {
                alt_indent_stack_[indent_] = alt_col;
                indent_stack_[indent_] = col;
            }
        }
        else {
            while(indent_ > 0 && col < indent_stack_[indent_]) {
                --changes;
                --indent_;
            }
            if(col != indent_stack_[indent_]) {
                return add_indent_error(true);
            }
            if(!ignore_altindent_errors_ && alt_col != alt_indent_stack_[indent_]) {
                return add_indent_error();
            }
        }

        TokenInfo info {{ changes < 0 ? Token::Dedent : Token::Indent,
                          changes < 0 ? TokenKind::Dedent : TokenKind::Indent,
                          TokenClass::Default},
                          line(),
                          1,
                          {changes < 0 ? "Dedent" : "Indent"}};
        info.line = line();
        info.column = 1;
        while(changes != 0) {
            changes += changes > 0 ? -1 : 1;
            token_buffer_.push_back(info);
        }
        return true;
    }

    bool Lexer::add_indent_error(bool dedent) {
        token_buffer_.push_back({
            {dedent ? Token::DedentationError : Token::IndentationError,
             TokenKind::Error,
             TokenClass::Default},
            line(),
            column_,
            dedent ? "Dedentation Error" : "Indentation Error"});
        return false;
    }

    TokenInfo Lexer::make_token(TokenInfo & tok, TokenIdent ident) {
        tok.ident = ident;
        return tok;
    }

    TokenInfo Lexer::make_token(TokenInfo & tok, Token id, TokenKind kind, TokenClass cls) {
        return make_token(tok, { id, kind, cls });
    }

    void Lexer::add_info_item(LexerInfoLevel level, TokenInfo const & t) {
        info_.push_back({t, level, {}});
    }

    TokenInfo Lexer::make_invalid(char const * begin, char const * end, Token id, TokenKind kind, TokenClass cls) {
        return { {id, kind, cls},line(), column_, std::string(begin, end) };
    }
}
