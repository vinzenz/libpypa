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
#include <stdio.h>
#include <pypa/lexer/lexer.hh>
#include <string>



int main(int argc, char const ** argv) {
    int errors = 0;
    int warnings = 0;
    for(int i = 0 + (argc == 1 ? 0 : 1); i < argc; ++i) {
        char const * file = i != 0 ? argv[i] : "test.py";
        pypa::Lexer l(file);
        for(;;) {
            auto t = l.next();
            if(t.ident.id() == pypa::Token::End) break;
            if(t.ident.id() == pypa::Token::Invalid) {
//                printf("########### INVALID TOKEN: (%s:%d:%d) %d - %s\n", file, t.line, t.column, int(t.ident.id()), t.value.c_str());
                ++errors;
            }
//            printf("%s Token[%d] %s\n", file, int(t.ident.id()), t.value.c_str());
        }
        if(!l.info().empty()) {
            printf("LexerInfo:\n");
            char const *level[] = {"Info", "Warn", "Error"};
            for(auto const & i : l.info()) {
                ++warnings;
                printf("\t%s: %d:%d %s\n", level[int(i.level)], int(i.info.line), int(i.info.column), i.value.c_str());
            }
        }
//        printf("%s current line: %d\n", file, l.line_);
        if(warnings || errors) return 0;
    }
    printf("%d lexing errors\n", errors);
    printf("%d lexing warnings\n", warnings);
}
