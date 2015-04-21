# libpypa - A Python Parser Library in C++

[![Join the chat at https://gitter.im/vinzenz/libpypa](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/vinzenz/libpypa?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

- [Introduction](#introduction)
 - [Motivation](#introduction-motivation)
 - [Goal](#introduction-goal)
- [Example](#example)
- [Error Reporting](#error-reporting)
- [Requirements](#requirements)
- [Structure](#structure)
 - [Lexer](#structure-lexer)
 - [Parser](#structure-parser)
 - [AST](#structure-ast)
- [License](#license)

<a name="introduction">
## Introduction
**libpypa** is a Python parser implemented in pure *C++*. It neither uses any
tools like [flex][1], [yacc][2], [bison][3] etc, nor is it using any parser
framework like [Boost.Spirit][4]. It's implementation is pure C++ code.

<a name="introduction-motivation">
### Motivation
I started getting involved into the [pyston project][5] where it had an entry
in their getting involved list for implementing a parser for Python. Never
having properly tackled the problem of creating a parser library for any
language, I decided it might be worth a try, since most of the libraries I
found, where basically just using the builtin Python parser or where
implemented in Python itself.

<a name="introduction-goal">
### Goal
The first goal of the library is to support python 2.7 syntax, later on 3.x
syntax might be added.

<a name="example">
## Example

An example file:

    $cat hello_world.py
    #! /usr/bin/env python
    # -*- coding: utf-8 -*-
    #

    """
        A "Hello World" example for the pypa parser
    """
    import sys

    print >> sys.stdout, "Hello", "World!"


And here the output of the test parser:

    $ ./parser-test hello_world.py
    Parsing successfull

    [Module]
      - body:
        [Suite]
          - items: [
                [DocString]
                  - doc:
        A "Hello World" example for the pypa parser


                [Import]
                  - names:
                    [Alias]
                      - as_name: <NULL>
                      - name:
                        [Name]
                          - context: Load
                          - dotted: False
                          - id: sys

                [Print]
                  - destination:
                    [Attribute]
                      - attribute:
                        [Name]
                          - context: Load
                          - dotted: False
                          - id: stdout
                      - context: Load
                      - value:
                        [Name]
                          - context: Load
                          - dotted: False
                          - id: sys
                  - newline: True
                  - values: [
                        [Str]
                          - value: Hello

                        [Str]
                          - value: World!
                        ]
                ]
      - kind: Module

And here the parse tree of python: (astdump.py can be found in tools)

    [Module]
        - body: [

            [Expr]
                - value:
                [Str]
                    - s:
        A "Hello World" example for the pypa parser


            [Import]
                - names: [

                    [alias]
                        - asname: None
                        - name: sys
                ]

            [Print]
                - dest:
                [Attribute]
                    - attr: stdout
                    - ctx: Load
                    - value:
                    [Name]
                        - ctx: Load
                        - id: sys
                - nl: True
                - values: [

                    [Str]
                        - s: Hello

                    [Str]
                        - s: World!
                ]
        ]


<a name="error-reporting">
## Error Reporting
The parser supports also SyntaxError and IndentionError reporting:

Let's take a look at this file `syntax_error.py` which clearly has a
syntax error:

    #! /usr/bin/env python
    # -*- coding: utf-8 -*-
    """
        Syntax error example
    """

    print x y z

This is the output of the test parser:

    $./parser-test syntax_error.py
      File "syntax_error.py", line 7
        print x y z
                ^
    SyntaxError: Expected new line after statement
    -> Reported @pypa/parser/parser.cc:944 in bool pypa::simple_stmt(pypa::{anonymous}::State&, pypa::AstStmt&)

    Parsing failed

And this of cpython 2.7:

    $ python syntax_error.py
      File "syntax_error.py", line 7
        print x y z
                ^
    SyntaxError: invalid syntax

**libpypa** uses different error messages than python, however in the hopes that
that would increase the clarity.

<a name="requirements">
## Requirements
To be able using **libpypa**, you have to have a *C++11* compiler available.
**libpypa** was developed on top of *g++ 4.8.2* and it heavily uses *C++11*
features where seen fit.

**libpypa** currently does not depend on any other libraries than the *C++11*
standard library with the exception of the `class FileBuf` which currently
uses system libraries, but might be changed to just use `fopen`/`fread`/
`fclose`.

<a name="structure">
## Structure
**libpypa** currently consists of 3 major parts:

 1. `Lexer`
 2. `Parser`
 3. `AST`

<a name="structure-lexer">
### Lexer
The `Lexer` portion of the library tokenizes the input for the `Parser` and
distinguishes the different types of tokens for the `Parser`.

<a name="structure-parser">
### Parser
The `Parser` utilizes the `Lexer` to parse the input and generates a
preliminary `AST` from the input.

<a name="structure-ast">
### AST
The AST contains the definition of all syntax elements in the code. The main
parts of the definition are in `pypa/ast/ast.hh` which makes heavily use of
preprocessor macros to define typedefs, mappings for compile time type lookups
by AstType (enum class), and an implementation for a switch based visitor.

The AST types do not implement any methods, they are just structures with data.
The only thing which is in there for some of the bases is the constructor, to
set the type id value and initialize the line and column values.

<a name="license">
## License
    Copyright 2014 Vinzenz Feenstra

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


### License for src/double-conversion
    Copyright 2006-2011, the V8 project authors. All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials provided
          with the distribution.
        * Neither the name of Google Inc. nor the names of its
          contributors may be used to endorse or promote products derived
          from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  [1]: http://flex.sourceforge.net/
  [2]: http://invisible-island.net/byacc/byacc.html
  [3]: http://www.gnu.org/s/bison/
  [4]: http://boost-spirit.com
  [5]: http://github.com/dropbox/pyston

