#ifndef GUARD_PYPA_TOKENIZER_LEXER_HH_INCLUDED
#define GUARD_PYPA_TOKENIZER_LEXER_HH_INCLUDED

#include <pypa/filebuf.hh>
#include <pypa/lexer/tokendef.hh>
#include <string>
#include <deque>
#include <list>
#include <vector>
#include <stdint.h>

namespace pypa {

struct TokenInfo {
    TokenIdent ident;
    uint64_t line;
    uint64_t column;
    std::string value;
};

enum class LexerInfoLevel {
    Information,
    Warning,
    Error
};

struct LexerInfo {
    TokenInfo info;
    LexerInfoLevel level;
    std::string value;
};

class Lexer {
public:
    enum {
        TabSize = 8,
        AltTabSize = 1,
    };

    pypa::FileBuf file_;
    std::string input_path_;

    uint64_t line_;
    uint64_t column_;
    int level_;
    int indent_;
    std::vector<int> indent_stack_;
    std::vector<int> alt_indent_stack_;
    std::deque<char> lex_buffer_;

    std::list<LexerInfo> info_;
    char first_indet_char;
    std::deque<TokenInfo> token_buffer_;
public:
    Lexer(char const * file_path);
    ~Lexer();

    std::list<LexerInfo> const & info();

    TokenInfo next();

    char skip();
    char skip_comment();

    char next_char();
    void put_char(char c);
    TokenInfo get_string(TokenInfo & tok, char first, char prefix=0);
    TokenInfo get_number(TokenInfo & tok, char first);
    TokenInfo get_number_binary(TokenInfo & tok, char first);
    TokenInfo get_number_hex(TokenInfo & tok, char first);
    TokenInfo get_number_octal(TokenInfo & tok, char first);
    TokenInfo get_number_float(TokenInfo & tok, char first);
    TokenInfo get_number_integer(TokenInfo & tok, char first);
    TokenInfo get_number_complex(TokenInfo & tok, char first);

    bool handle_indentation(bool continuation = false);
    void handle_whitespace(char c);

    TokenInfo make_token(TokenInfo & tok, Token id, TokenKind kind,
                         TokenClass cls = TokenClass::Default);
    TokenInfo make_token(TokenInfo & tok, TokenIdent ident);
    TokenInfo make_invalid(char const * begin, char const * end,
                           Token id = Token::Invalid,
                           TokenKind kind = TokenKind::Error,
                           TokenClass cls = TokenClass::Default);

    bool add_indent_error(bool dedent = false);
    void add_info_item(LexerInfoLevel level, TokenInfo const & t);

    inline void add_information(TokenInfo const & t) {
        add_info_item(LexerInfoLevel::Information, t);
    }

    inline void add_warning(TokenInfo const & t) {
        add_info_item(LexerInfoLevel::Warning, t);
    }

    inline void add_error(TokenInfo const & t) {
        add_info_item(LexerInfoLevel::Error, t);
    }
};

}

#endif //GUARD_PYPA_TOKENIZER_LEXER_HH_INCLUDED

