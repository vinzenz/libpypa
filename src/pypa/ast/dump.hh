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
#ifndef GUARD_PYPA_AST_DUMP_HH_INCLUDED
#define GUARD_PYPA_AST_DUMP_HH_INCLUDED

#include <pypa/ast/types.hh>

namespace pypa {

    template<typename T>
    struct ast_member_dump;

    struct ast_member_dump_revisit {
        int depth_;
        ast_member_dump_revisit(int depth) : depth_(depth) {}
        template< typename T >
        bool operator() ( T const & v ) const {
            ast_member_dump<T>::dump(depth_, v);
            return true;
        }
    };

    // Forward declaration for the dump visit implementation
    struct Ast;

    namespace detail {
        void visit_dump_internal(int depth, Ast const & v);

        template< typename T, typename V, typename F>
        void apply_member(T & t, V T::*member, F f) {
            f(t.*member);
        }

        template< typename T, typename V, typename F>
        void apply_member(std::shared_ptr<T> t, V T::*member, F f) {
            f((*t).*member);
        }

        inline void print_padding(int depth) {
            while(depth--) printf(" ");
        }

        inline void dump_member_value(int depth, Ast const & v) {
            print_padding(depth);

            // This is used to dispatch it to the right ast_member_dump<T>
            // implementation using ast_member_dump_revisit
            visit_dump_internal(depth, v);
        }

        inline void dump_member_value(int depth, String const & v) {
            printf("%s\n", v.c_str());
        }

        inline void dump_member_value(int depth, char const * v) {
            printf("RAW BUFFER: %p\n", static_cast<void const*>(v));
        }

        inline void dump_member_value(int depth, bool v) {
            printf("%s\n", v ? "True" : "False");
        }

        inline void dump_member_value(int depth, int const & v) {
            printf("%d\n", int(v));
        }

        inline void dump_member_value(int depth, int64_t const & v) {
            long long int p = v;
            printf("%lld\n", p);
        }

        inline void dump_member_value(int depth, double const & v) {
            printf("%g\n", v);
        }

        inline void dump_member_value(int depth, AstBoolOpType const & v) {
            printf("%s\n", to_string(v));
        }

        inline void dump_member_value(int depth, AstUnaryOpType const & v) {
            printf("%s\n", to_string(v));
        }

        inline void dump_member_value(int depth, AstCompareOpType const & v) {
            printf("%s\n", to_string(v));
        }

        inline void dump_member_value(int depth, AstBinOpType const & v) {
            printf("%s\n", to_string(v));
        }

        inline void dump_member_value(int depth, AstModuleKind const & v) {
            printf("%s\n", to_string(v));
        }

        inline void dump_member_value(int depth, AstContext const & v) {
            printf("%s\n", to_string(v));
        }

        template< typename T >
        inline void dump_member_value(int depth, std::shared_ptr<T> const & v) {
            if(!v) {
                printf("<NULL>\n");
            }
            else {
                T const & v_ = *v;
                dump_member_value(int(depth), v_);
            }
        }

        template< typename T >
        inline void dump_padded_member(int depth, std::shared_ptr<T> const & v) {
            if(!v) { printf("\n"); print_padding(depth+4); }
            dump_member_value(depth+4, v);
        }

        template< typename T >
        inline void dump_padded_member(int depth, T const & v) {
            dump_member_value(depth+4, v);
        }

        template< typename T >
        inline void dump_member_value(int depth, std::vector<T> const & v) {
            if(v.empty()) {
                printf("[]\n");
            }
            else {
                printf("[");
                for(auto const & e : v) {
                    dump_padded_member(depth, e);
                }
                print_padding(depth+4);
                printf("]\n");
            }
        }
    }
}

#endif // GUARD_PYPA_AST_DUMP_HH_INCLUDED
