#ifndef GUARD_PYPA_PARSER_STATE_HH_INCLUDED
#define GUARD_PYPA_PARSER_STATE_HH_INCLUDED

#include <pypa/parser/parser.hh>
#include <string>
#include <stack>

namespace pypa {
namespace {
    struct State {
        Lexer *                 lexer;
        std::stack<TokenInfo>   tokens;
        std::stack<TokenInfo>   popped;
        std::stack<std::size_t> savepoints;
        TokenInfo               tok_cur;
    };

    TokenInfo pop(State & s) {
        s.popped.push(s.tok_cur);
        if(s.tokens.empty()) {
            s.tok_cur = s.lexer->next();
        }
        else {
            s.tok_cur = s.tokens.top();
            s.tokens.pop();
        }

        return s.tok_cur;
    }

    void unpop(State & s) {
        s.tokens.push(s.tok_cur);
        s.tok_cur = s.popped.top();
        s.popped.pop();
    }

    void save(State & s) {
        s.savepoints.push(s.popped.size());
    }

    void revert(State & s) {
        if(!s.savepoints.empty()) {
            while(s.popped.size() != s.savepoints.top()) {
                unpop(s);
            }
            s.savepoints.pop();
        }
    }

    void pop_savepoint(State & s) {
        if(!s.savepoints.empty()) {
            s.savepoints.pop();
        }
    }

    struct StateGuard {
        StateGuard(State & s) : reset_(), s_(&s) { save(s); }
        template< typename T >
        StateGuard(State & s, std::shared_ptr<T> & r) : reset_([&r](){r.reset();}), s_(&s) { save(s); }
        ~StateGuard() { if(s_) revert(*s_); if(reset_) reset_(); }
        bool commit() { if(s_) pop_savepoint(*s_); s_ = 0; reset_ = {}; return true; }
    private:
        std::function<void()> reset_;
        State * s_;
    };

    void commit(State & s) {
        s.popped = {};
    }

    TokenInfo const & top(State & s) {
        return s.tok_cur;
    }

    TokenKind kind(TokenInfo const & tok) {
        return tok.ident.kind();
    }

    TokenClass cls(TokenInfo const & tok) {
        return tok.ident.cls();
    }

    Token token(TokenInfo const & tok) {
        return tok.ident.id();
    }

    template< typename T >
    std::shared_ptr<T> & create(std::shared_ptr<T> & t) {
        return (t = std::make_shared<T>());
    }

    template< typename U, typename T >
    std::shared_ptr<U> create(std::shared_ptr<T> & t) {
        return std::static_pointer_cast<U>(t = std::make_shared<U>());
    }

    void location(State & s, AstPtr a) {
        a->line = top(s).line;
        a->column = top(s).column;
    }

    void location(State & s, Ast & a) {
        a.line = top(s).line;
        a.column = top(s).column;
    }

    bool is(TokenInfo const & info, Token tok) {
        return token(info) == tok;
    }

    bool is(TokenInfo const & info, TokenKind k) {
        return kind(info) == k;
    }

    bool is(TokenInfo const & info, TokenClass c) {
        return cls(info) == c;
    }

    bool is(State & s, Token tok) {
        return is(top(s), tok);
    }

    bool is(State & s, TokenKind k) {
        return is(top(s), k);
    }

    bool is(State & s, TokenClass c) {
        return is(top(s), c);
    }

    bool end(State & s) {
        return is(s, Token::End);
    }

    template< typename T >
    bool expect(State & s, T t) {
        if(is(top(s), t)) {
            pop(s);
            return true;
        }
        return false;
    }

    template< typename T >
    bool consume_value(State & s, T t, String & v) {
        if(is(s, t)) {
            v = top(s).value;
            pop(s);
            return true;
        }
        return false;
    }
}}


#endif // GUARD_PYPA_PARSER_STATE_HH_INCLUDED
