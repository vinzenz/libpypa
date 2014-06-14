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
#include <cassert>

namespace pypa {
    void SymbolTable::push_entry(BlockType type, String name, bool nested, int line) {
        auto e          = std::make_shared<SymbolTableEntry>();
        e->id           = ++last_id;
        e->type         = type;
        e->name         = name;
        e->is_nested    = nested;
        e->start_line   = line;
        symbols[e->id] = e;
        if(!module) {
            module = e;
        }
        if(nested) {
            current->children.push_back(e);
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
}
