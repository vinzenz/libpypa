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
#ifndef GUARD_PYPA_PARSER_ERROR_HH_INCLUDED
#define GUARD_PYPA_PARSER_ERROR_HH_INCLUDED

#include <pypa/ast/ast.hh>
#include <pypa/lexer/lexer.hh>

namespace pypa {

    enum class ErrorType {
        SyntaxError,
        SyntaxWarning,
        IndentationError
    };

    struct Error {
        ErrorType    type;
        String       message;
        String       file_name;
        TokenInfo    cur;
        AstPtr       ast;
        std::string  line;
        int          detected_line;
        char const * detected_file;
        char const * detected_function;
    };
}

#endif // GUARD_PYPA_PARSER_ERROR_HH_INCLUDED
