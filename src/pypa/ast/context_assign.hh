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
#ifndef GUARD_PYPA_AST_CONTEXT_ASSIGN_HH_INCLUDED
#define GUARD_PYPA_AST_CONTEXT_ASSIGN_HH_INCLUDED

#include <pypa/ast/visitor.hh>

namespace pypa {

struct context_assign {
    AstContext context;

    template< typename T >
    void operator() (std::shared_ptr<T> p) {
        if(p) (*this)(*p);
    }

    void operator() (AstTuple & tuple) {
        // If AstContext::Param then tuple and elements should be AstContext::Store
        // That's the behaviour of Python 2.7
        tuple.context = context == AstContext::Param ? AstContext::Store : context;
        for(auto & e : tuple.elements) {
            visit(context_assign{tuple.context}, e);
        }
    }

    void operator() (AstList & lst) {
        lst.context = context == AstContext::Param ? AstContext::Store : context;
        for(auto & e : lst.elements) {
            visit(context_assign{lst.context}, e);
        }
    }

    void operator() (AstName & name) {
        name.context = context;
    }

    void operator() (AstKeyword & k) {
        visit(context_assign{AstContext::Store}, k.name);
    }

    void operator() (AstAttribute & attribute) {
        attribute.context = context;
        visit(*this, attribute.attribute);
    }

    void operator() (AstSubscript & subscript) {
        subscript.context = context;
    }

    template< typename T >
    void operator() (T) {
        // Ignored
    }

};

}

#endif // GUARD_PYPA_AST_CONTEXT_ASSIGN_HH_INCLUDED
