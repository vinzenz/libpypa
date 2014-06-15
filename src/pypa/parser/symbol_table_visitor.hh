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
#ifndef GUARD_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED
#define GUARD_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED

#include <pypa/parser/symbol_table.hh>
#include <pypa/ast/tree_walker.hh>

namespace pypa {

#define PYPA_ADD_SYMBOL_ERR(MSG, AST) add_error(MSG, AST, __LINE__, __FILE__, __PRETTY_FUNCTION__)

    struct symbol_table_visitor {
        SymbolTablePtr table;
        SymbolErrorReportFun push_error;

        void add_error(char const * message, Ast & o, int line = -1, char const * file = 0, char const * function = 0) {
            if(push_error) {
                push_error(message, o.line, o.column, line, file ? file : "", function ? function : "");
            }
        }

        String const & get_name(AstExpr const & expr) {
            assert(expr && expr->type == AstType::Name);
            return std::static_pointer_cast<AstName>(expr)->id;
        }

        // TODO: name mangeling

        void add_def(String const & name, uint32_t flags) {
        }

        bool operator() (AstFunctionDef & f) {
            String const & name = get_name(f.name);     // We can keep the ref
            add_def(name, SymbolFlag_Local);

            walk_tree(f.args.defaults, *this);

            table->push_entry(BlockType::Function, name, f.line);
            // TODO: Special arguments handling
            walk_tree(f.args.arguments, *this);
            walk_tree(f.args.keywords, *this);
            walk_tree(f.args.args, *this);
            walk_tree(f.args.kwargs, *this);
            walk_tree(f.body, *this);
            table->pop_entry();

            return false;
        }

        bool operator() (AstClassDef & c) {
            String const & name = get_name(c.name);     // We can keep the ref
            add_def(name, SymbolFlag_Local);

            walk_tree(c.bases, *this);

            table->push_entry(BlockType::Class, name, c.line);
            walk_tree(c.body, *this);
            table->pop_entry();

            return false;
        }

        bool operator() (AstModule & m) {
            return true;
        }

        bool operator() (AstLambda & l) {
            walk_tree(l.arguments.defaults, *this);
            table->push_entry(BlockType::Function, "lambda", l.line);
            // TODO: Special arguments handling
            walk_tree(l.arguments.arguments, *this);
            walk_tree(l.body, *this);
            table->pop_entry();
            return false;
        }

        bool operator() (AstYieldExpr & y) {
            table->current->is_generator = true;
            if(table->current->returns_value) {
                PYPA_ADD_SYMBOL_ERR("Return value in generator", y);
            }
        }

        bool operator() (AstName & n) {
            add_def(n.id, n.context == AstContext::Load ? SymbolFlag_Used : SymbolFlag_Local);
            return false;
        }

        template< typename T >
        bool operator ()(T)
        {
            return true;
        }
    };


}

#endif // GUARD_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED
