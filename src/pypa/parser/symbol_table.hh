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
#ifndef GUARD_PYPA_PARSER_SYMBOL_TABLE_HH_INCLUDED
#define GUARD_PYPA_PARSER_SYMBOL_TABLE_HH_INCLUDED

#include <list>
#include <memory>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <pypa/ast/ast.hh>
#include <pypa/parser/future_features.hh>

namespace pypa {

enum class BlockType {
    Function,
    Class,
    Module
};


struct SymbolFlags {
    bool global_explicit;
    bool global_implicit;
    bool local;
    bool param;
    bool used;
    bool free;
    bool free_class;
    bool import;
    bool cell;

    bool global() const {
        return global_explicit || global_implicit;
    }

    bool bound() const {
        return local || param || import;
    }
};

typedef std::shared_ptr<struct SymbolTableEntry> SymbolTableEntryPtr;
struct SymbolTableEntry {
    uint64_t                                id;
    BlockType                               type;
    String                                  name;

    std::unordered_map<String, SymbolFlags> symbols;
    std::unordered_set<String>              variables;
    std::list<SymbolTableEntryPtr>          children;

    bool is_nested;             // true if nested
    bool returns_value;         // true if namespace uses return with an argument
    bool has_varargs;
    bool has_varkw;
    bool is_generator;          // true if namespace is a generator

    bool has_free_vars;         // true if block has free variables
    bool child_has_free_vars;   // true if a child block has free vars,
                                // including free refs to globals
    int start_line;             // Line number where the block starts
    int temp_name_count;        // number of temporary variables (e.g. in list comp)
};

struct SymbolTable {
    String file_name;
    uint64_t last_id;

    std::unordered_map<uint64_t, SymbolTableEntryPtr> symbols;
    std::stack<SymbolTableEntryPtr> stack;

    SymbolTableEntryPtr module;
    SymbolTableEntryPtr current;

    FutureFeatures   future_features;

    void push_entry(BlockType type, String name, int line);
    void pop_entry();
};

typedef std::shared_ptr<SymbolTable> SymbolTablePtr;

SymbolTablePtr create_from_ast(AstPtr const a, FutureFeatures const & future_features);
SymbolTablePtr create_from_ast(Ast const & a, FutureFeatures const & future_features);

}

#endif // GUARD_PYPA_PARSER_SYMBOL_TABLE_HH_INCLUDED
