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
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include <pypa/ast/ast.hh>
#include <pypa/parser/future_features.hh>
#include <pypa/parser/error.hh>

namespace pypa {

enum class BlockType {
    Function,
    Class,
    Module
};

enum SymbolFlags {
    SymbolFlag_GlobalExplicit   = 1 << 0,
    SymbolFlag_GlobalImplicit   = 1 << 1,
    SymbolFlag_Global           = (SymbolFlag_GlobalImplicit | SymbolFlag_GlobalExplicit),
    SymbolFlag_Local            = 1 << 2,
    SymbolFlag_Param            = 1 << 3,
    SymbolFlag_Used             = 1 << 4,
    SymbolFlag_Free             = 1 << 5,
    SymbolFlag_FreeClass        = 1 << 6,
    SymbolFlag_Import           = 1 << 7,
    SymbolFlag_Cell             = 1 << 8,
    SymbolFlag_Bound            = (SymbolFlag_Import | SymbolFlag_Local | SymbolFlag_Param)
};

enum OptimizeFlags {
    OptimizeFlag_ImportStar     = 1 << 0,
    OptimizeFlag_Exec           = 1 << 1,
    OptimizeFlag_BareExec       = 1 << 2,
    OptimizeFlag_TopLevel       = 1 << 3, // Top level names including eval and exec
};

typedef std::shared_ptr<struct SymbolTableEntry> SymbolTableEntryPtr;
struct SymbolTableEntry {
    void *                                  id;
    BlockType                               type;
    String                                  name;

    std::unordered_map<String, uint32_t>    symbols;
    std::unordered_set<String>              variables;
    std::list<SymbolTableEntryPtr>          children;

    bool is_nested;             // true if nested
    bool returns_value;         // true if namespace uses return with an argument
    bool has_varargs;
    bool has_varkw;
    bool is_generator;          // true if namespace is a generator
    bool in_loop;               // true if current code path is inside a loop
    bool in_finally;            // true if current code path is inside a finally body

    bool has_free_vars;         // true if block has free variables
    bool child_has_free_vars;   // true if a child block has free vars,
                                // including free refs to globals
    int start_line;             // Line number where the block starts
    int temp_name_count;        // number of temporary variables (e.g. in list comp)

    uint32_t unoptimized;       // optimization flags
    int opt_last_line;          // last line of exec or import
};

struct SymbolTable {
    String file_name;

    String current_class;
    std::unordered_map<void *, SymbolTableEntryPtr> symbols;
    std::stack<SymbolTableEntryPtr> stack;

    SymbolTableEntryPtr module;
    SymbolTableEntryPtr current;

    FutureFeatures   future_features;

    void enter_block(BlockType type, String const & name, Ast &);
    void leave_block();
};

typedef std::shared_ptr<SymbolTable> SymbolTablePtr;
typedef std::function<void(pypa::Error)> SymbolErrorReportFun;
void create_from_ast(SymbolTablePtr p, Ast const & a, SymbolErrorReportFun add_err);
}

#endif // GUARD_PYPA_PARSER_SYMBOL_TABLE_HH_INCLUDED
