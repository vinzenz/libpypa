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

#include <functional>

#include <pypa/ast/ast.hh>
#include <pypa/lexer/lexer.hh>
#include <pypa/parser/symbol_table.hh>
#include <pypa/types.hh>

namespace pypa {

struct ParserOptions {
    ParserOptions()
    : python3only(false)
    , python3allowed(false)
    , docstrings(true)
    , printerrors(true)
    , printdbgerrors(false)
    , handle_future_errors(true)
    , error_handler()
    , perform_inline_optimizations(false)
    {}

    bool python3only;          // If it is parsing python3
    bool python3allowed;       // Whether or not python3 constructs are allowed
    bool docstrings;           // Converts docstring strings to AstDocString
                               // entries
    bool printerrors;          // Prints errors itself
    bool printdbgerrors;       // Prints internal debug information
    bool handle_future_errors; // Handles unknown __future__ features
                               // by reporting an error
    FutureFeatures initial_future_features;

    std::function<void(pypa::Error)> error_handler;
    std::function<pypa::String(pypa::String const & value,
                               pypa::String const & encoding,
                               bool unicode,
                               bool raw_prefix,
                               bool & error
                              )> escape_handler;
    bool perform_inline_optimizations; // If inline optimizations should be
                                       // performed
};

bool parse(Lexer & lexer,
           AstModulePtr & ast,
           SymbolTablePtr & symbols,
           ParserOptions options = ParserOptions());

}

#endif // GUARD_PYPA_PARSER_PARSER_HH_INCLUDED

