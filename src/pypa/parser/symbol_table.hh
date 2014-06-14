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

#include <memory>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <pypa/types.hh>
#include <pypa/parser/future_features.hh>

namespace pypa {

enum class BlockType {
    Function,
    Class,
    Module
};


struct SymbolFlags {

};

typedef std::shared_ptr<struct SymbolTableEntry> SymbolTableEntryPtr;
struct SymbolTableEntry {
    uint64_t                                id;
    BlockType                               type;
    std::unordered_map<String, SymbolFlags> symbols;
    String                                  name;
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
    std::stack<SymbolTableEntryPtr> stack;

    SymbolTableEntry module;
    SymbolTableEntry current;

    FutureFeatures   future_features;
};

}

#endif // GUARD_PYPA_PARSER_SYMBOL_TABLE_HH_INCLUDED
