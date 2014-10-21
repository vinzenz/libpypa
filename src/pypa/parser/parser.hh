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
#ifndef GUARD_PYPA_PARSER_PARSER_HH_INCLUDED
#define GUARD_PYPA_PARSER_PARSER_HH_INCLUDED

#include <pypa/ast/ast.hh>
#include <pypa/lexer/lexer.hh>
#include <pypa/parser/symbol_table.hh>
#include <functional>

namespace pypa {

struct ParserOptions {
    ParserOptions()
    : python3only(false)
    , python3allowed(false)
    , docstrings(true)
    , printerrors(true)
    , printdbgerrors(false)
    , error_handler()
    {}

    bool python3only;
    bool python3allowed;
    bool docstrings;
    bool printerrors;
    bool printdbgerrors;
    std::function<void(pypa::Error)> error_handler;
};

bool parse(Lexer & lexer,
           AstModulePtr & ast,
           SymbolTablePtr & symbols,
           ParserOptions options = ParserOptions());

}

#endif // GUARD_PYPA_PARSER_PARSER_HH_INCLUDED

