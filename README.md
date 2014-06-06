# libpypa - A Python Parser Library

- [Introduction](#introduction)
 - [Motivation](#introduction-motivation)
 - [Goal](#introduction-goal)
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
parst of the definition are in `pypa/ast/ast.hh` which makes heavily use of
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




  [1]: http://flex.sourceforge.net/
  [2]: http://invisible-island.net/byacc/byacc.html
  [3]: http://www.gnu.org/s/bison/
  [4]: http://boost-spirit.com
  [5]: http://github.com/dropbox/pyston
