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
#include <pypa/parser/symbol_table.hh>
#include <pypa/ast/visitor.hh>
#include <cassert>

namespace pypa {
    void SymbolTable::push_entry(BlockType type, String name, int line) {
        auto e          = std::make_shared<SymbolTableEntry>();
        e->id           = ++last_id;
        e->type         = type;
        e->name         = name;
        e->is_nested    = false;
        e->start_line   = line;
        symbols[e->id] = e;

        if(!module) {
            module = e;
        }

        if(current && (current->is_nested || type == BlockType::Function)) {
            e->is_nested = true;
        }

        stack.push(e);
        current = e;
    }

    void SymbolTable::pop_entry() {
        assert(!stack.empty());
        stack.pop();
        if(!stack.empty()) {
            current = stack.top();
        }
        else {
            current = module; // This should never be necessary but who knows
        }
    }

    SymbolTablePtr create_from_ast(AstPtr const a, FutureFeatures const & future_features) {
        if(!a) {
            return SymbolTablePtr();
        }
        return create_from_ast(*a, future_features);
    }

    SymbolTablePtr create_from_ast(Ast const & a, FutureFeatures const & future_features) {
        SymbolTablePtr table = std::make_shared<SymbolTable>();
        table->future_features = future_features;

        return table;
    }

}
