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
#ifndef GUARD_PYPA_AST_TREE_WALKER_HH_INCLUDED
#define GUARD_PYPA_AST_TREE_WALKER_HH_INCLUDED

#include <pypa/ast/visitor.hh>
#include <type_traits>

namespace pypa {

template< typename AstT, typename F >
void walk_tree(AstT & t, F f, int depth = 0);

namespace detail {
    template< typename F >
    struct tree_walk_visitor {
        struct each {
            F * f_;
            int depth_;
            each(F * f, int depth) : f_(f), depth_(depth) {}

            void next(Ast & t) {
                visit(detail::tree_walk_visitor<F>(f_, depth_ + 1), t);
            }

            template< typename T >
            void next(std::shared_ptr<T> t) {
                if(t) visit(detail::tree_walk_visitor<F>(f_, depth_ + 1), *t);
            }

            template< typename T >
            void next(T) {}

            template< typename T >
            bool operator() (T t) {
                if((*f_)(t)) {
                    next(t);
                    return true;
                }
                return false;
            }

            template< typename T >
            void operator() (std::vector<T> & t) {
                for(auto & e : t) {
                    next(e);
                }
            }
        };

        F * f_;
        int depth_;
        tree_walk_visitor(F * f, int depth) : f_(f), depth_(depth){}
        template< typename T >
        void operator() (T  t) {
            ast_member_visit<AstIDByType<T>::Id>::apply(t, each(f_, depth_ + 1));
        }
    };
}

template< typename AstT, typename F >
void walk_tree(AstT & t, F f, int depth) {
    visit(detail::tree_walk_visitor<F>(&f, depth), t);
}

template< typename AstT, typename F >
void walk_tree(std::vector<AstT> & t, F f, int depth = 0) {
    for(auto & e : t) {
        visit(detail::tree_walk_visitor<F>(&f, depth), e);
    }
}

}

#endif //GUARD_PYPA_AST_TREE_WALKER_HH_INCLUDED
