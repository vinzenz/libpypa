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

#include <cstdio>

#include <pypa/parser/parser.hh>

namespace pypa {
    void dump(AstPtr);
}

int main(int argc, char const ** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <python_file_path>\n", argv[0]);
        return 1;
    }
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    // options.python3allowed = true;
    options.printerrors = true;
    options.printdbgerrors = true;
    pypa::Lexer lexer(argv[1]);
    if(pypa::parse(lexer, ast, symbols, options)) {
        printf("Parsing successfull\n");
        dump(ast);
    }
    else {
        fprintf(stderr, "Parsing failed\n");
        return 1;
    }
    return 0;
}
