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
#ifndef GUARD_PYPA_PARSER_STATE_HH_INCLUDED
#define GUARD_PYPA_PARSER_STATE_HH_INCLUDED

#include <pypa/parser/parser.hh>
#include <pypa/parser/future_features.hh>
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
        std::stack<Error>       errors;
        ParserOptions           options;
        FutureFeatures          future_features;
    };

    inline TokenInfo pop(State & s) {
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

    inline void unpop(State & s) {
        s.tokens.push(s.tok_cur);
        s.tok_cur = s.popped.top();
        s.popped.pop();
    }

    inline void save(State & s) {
        s.savepoints.push(s.popped.size());
    }

    inline void revert(State & s) {
        if(!s.savepoints.empty()) {
            while(s.popped.size() != s.savepoints.top()) {
                unpop(s);
            }
            s.savepoints.pop();
        }
    }

    inline void pop_savepoint(State & s) {
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

    inline void commit(State & s) {
        s.popped = std::stack<TokenInfo>();
    }

    inline TokenInfo const & top(State & s) {
        return s.tok_cur;
    }

    inline TokenKind kind(TokenInfo const & tok) {
        return tok.ident.kind();
    }

    inline TokenClass cls(TokenInfo const & tok) {
        return tok.ident.cls();
    }

    inline Token token(TokenInfo const & tok) {
        return tok.ident.id();
    }

    template< typename T >
    inline std::shared_ptr<T> & create(std::shared_ptr<T> & t) {
        return (t = std::make_shared<T>());
    }

    template< typename U, typename T >
    inline std::shared_ptr<U> create(std::shared_ptr<T> & t) {
        return std::static_pointer_cast<U>(t = std::make_shared<U>());
    }

    inline void location(State & s, AstPtr a) {
        a->line = top(s).line;
        a->column = top(s).column;
    }

    inline void location(State & s, Ast & a) {
        a.line = top(s).line;
        a.column = top(s).column;
    }

    inline void clone_location(Ast & source, Ast & target) {
        target.column = source.column;
        target.line = source.line;
    }

    inline void clone_location(AstPtr source, AstPtr target) {
        if(source && target) {
            clone_location(*source, *target);
        }
    }

    inline bool is(TokenInfo const & info, Token tok) {
        return token(info) == tok;
    }

    inline bool is(TokenInfo const & info, TokenKind k) {
        return kind(info) == k;
    }

    inline bool is(TokenInfo const & info, TokenClass c) {
        return cls(info) == c;
    }

    inline bool is(State & s, Token tok) {
        return is(top(s), tok);
    }

    inline bool is(State & s, TokenKind k) {
        return is(top(s), k);
    }

    inline bool is(State & s, TokenClass c) {
        return is(top(s), c);
    }

    inline bool end(State & s) {
        return is(s, Token::End);
    }

    template< typename T >
    inline bool expect(State & s, T t) {
        if(is(top(s), t)) {
            pop(s);
            return true;
        }
        return false;
    }

    template< typename T >
    inline bool consume_value(State & s, T t, String & v) {
        if(is(s, t)) {
            v = top(s).value;
            pop(s);
            return true;
        }
        return false;
    }
}}


#endif // GUARD_PYPA_PARSER_STATE_HH_INCLUDED
