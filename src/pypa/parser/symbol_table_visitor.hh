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
#ifndef GUARD_PYPA_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED
#define GUARD_PYPA_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED

#include <pypa/parser/symbol_table.hh>
#include <pypa/ast/tree_walker.hh>

namespace pypa {

#define PYPA_ADD_SYMBOL_ERR(MSG, AST) add_error(MSG, AST, __LINE__, __FILE__, __PRETTY_FUNCTION__)
#define PYPA_ADD_SYMBOL_WARN(MSG, AST) add_warn(MSG, AST, __LINE__, __FILE__, __PRETTY_FUNCTION__)

    struct symbol_table_visitor {
        SymbolTablePtr table;
        SymbolErrorReportFun push_error;

        void add_error(char const * message, Ast & o, int line = -1, char const * file = 0, char const * function = 0) {
            add_error(ErrorType::SyntaxError, message, o, line, file, function);
        }

        void add_warn(char const * message, Ast & o, int line = -1, char const * file = 0, char const * function = 0) {
            add_error(ErrorType::SyntaxWarning, message, o, line, file, function);
        }

        void add_error(ErrorType type, char const * message, Ast & o, int line = -1, char const * file = 0, char const * function = 0) {
            if(push_error) {
                assert(message);
                push_error({
                    type,
                    message,
                    {},
                    {{}, o.line, o.column, {}},
                    {},
                    {},
                    line,
                    file ? file : "",
                    function ? function : ""
                });
            }
        }

        uint32_t lookup(String const & name) {
            String mangled = mangle(name);
            uint32_t result = 0;
            if(table->current->symbols.count(name)) {
                result = table->current->symbols[name];
            }
            return result;
        }

        void implicit_arg(uint32_t pos, Ast & a) {
            char buffer[32]{};
            if(::snprintf(buffer, sizeof(buffer), ".%u", pos) > 0) {
                add_def(buffer, SymbolFlag_Param, a);
            }
        }

        void params_nested(AstExprList & args) {
            for(AstExpr & e : args) {
                assert(e && "Expected valid AstExpr instance");
                if(e->type == AstType::Tuple) {
                    AstTuple & t = *std::static_pointer_cast<AstTuple>(e);
                    params(t.elements, false);
                }
            }
        }        

        void params(AstExprList & args, bool toplevel) {
            for(size_t i = 0; i < args.size(); ++i) {
                AstExpr & e = args[i];
                assert(e && "Expected valid AstExpr instance");
                switch(e->type) {
                case AstType::Name: {
                    AstName &  n = *std::static_pointer_cast<AstName>(e);
                    assert(n.context == AstContext::Param || (n.context == AstContext::Store && !toplevel));
                    add_def(n.id, SymbolFlag_Param, n);
                    break;
                }
                case AstType::Tuple: {
                    AstTuple & t = *std::static_pointer_cast<AstTuple>(e);
                    assert(t.context == AstContext::Store);
                    if(toplevel) {
                        implicit_arg(uint32_t(i), t);
                    }
                    break;
                }
                default:
                    PYPA_ADD_SYMBOL_ERR("Invalid expression in parameter list", *e);
                    break;
                }
            }
            if(toplevel) {
                params_nested(args);
            }
        }

        void arguments(AstArguments & args) {
            params(args.arguments, true);
            if(args.args) {
                add_def(get_name(args.args), SymbolFlag_Param, *args.args);
                table->current->has_varargs = true;
            }
            if(args.kwargs) {
                add_def(get_name(args.kwargs), SymbolFlag_Param, *args.kwargs);
                table->current->has_varkw = true;
            }
            assert(args.keywords.empty()
                   && "There must be no keyword items in a lambda or function definition");
        }

        String const & get_name(AstExpr const & expr) {
            assert(expr && expr->type == AstType::Name);
            return std::static_pointer_cast<AstName>(expr)->id;
        }

        String mangle(String name) {
            // No class no private variable
            if(!table->current_class.empty()) {
                return name;
            }
            // Must start with 2 underscores
            if(name.find_first_not_of('_') != 2) {
                return name;
            }
            // NO items in __<SOMETHING>__ form should be mangled
            if(name.size() > 2 && name.find_last_not_of('_', 2) != name.size() - 2) {
                return name;
            }
            // Don't mangle names with dots
            if(name.find_first_of('.') != String::npos) {
                return name;
            }
            char const * classname = table->current_class.c_str();
            // strip leading _ from the classname
            while(*classname == '_') {
                ++classname;
            }
            // Don't mangle just underscore classes
            if(!*classname) {
                return name;
            }
            name = "_" + (classname + name);
            return name;
        }

        void add_def(String name, uint32_t flags, Ast & a) {
            String mangled = mangle(name);
            uint32_t symflags = 0;
            if(table->current->symbols.count(name)) {
                symflags = table->current->symbols[name];
                if((flags & SymbolFlag_Param) && (flags & SymbolFlag_Param)) {
                    String errmsg = "Duplicated argument '" + name + "'in function definition";
                    PYPA_ADD_SYMBOL_ERR(errmsg.c_str(), a);
                    return;
                }
                symflags |= flags;
            }
            else {
                symflags = flags;
            }
            table->current->symbols[mangled] = symflags;
            if(flags & SymbolFlag_Param) {
                table->current->variables.insert(mangled);
            }
            else if(flags & SymbolFlag_Global) {
                if(table->module) {
                    table->module->symbols[mangled] |= flags;
                }
            }
        }

        bool operator() (AstFunctionDef & f) {
            String const & name = get_name(f.name);     // We can keep the ref
            add_def(name, SymbolFlag_Local, f);

            walk_tree(f.args.defaults, *this);
            walk_tree(f.decorators, *this);
            table->enter_block(BlockType::Function, name, f);
            // TODO: Special arguments handling
            arguments(f.args);
            walk_tree(*f.body, *this);

            table->leave_block();

            return false;
        }

        bool operator() (AstGlobal & g) {
            for(auto name : g.names) {
                assert(name);
                AstName & n = *name;
                uint32_t flags = lookup(n.id);
                if(flags & (SymbolFlag_Local | SymbolFlag_Used)) {
                    String errmsg = "Name '" + n.id + "' is ";
                    if(flags & SymbolFlag_Local) {
                        errmsg += "is assigned to before global declaration";
                        PYPA_ADD_SYMBOL_WARN(errmsg.c_str(), n);
                    }
                    else {
                        errmsg += "is used prior to global declaration";
                        PYPA_ADD_SYMBOL_WARN(errmsg.c_str(), n);
                    }
                }
                add_def(n.id, SymbolFlag_Global, n);
            }
            return false;
        }

        void handle_comprehension(bool generator, String const & scope_name, Ast & e,
                                  AstExprList & generators, AstExpr element, AstExpr value) {
            bool needs_tmp = !generator;

            assert(!generators.empty() && generators.front());

            AstComprehension & outermost = *std::static_pointer_cast<AstComprehension>(generators.front());
            walk_tree(*outermost.iter, *this);

            table->enter_block(BlockType::Function, scope_name, e);

            table->current->is_generator = generator;
            implicit_arg(0, e);

            if(needs_tmp) {
                char tmpname[32]{};
                snprintf(tmpname, sizeof(tmpname), "_[%d]", ++table->current->temp_name_count);
                add_def(tmpname, SymbolFlag_Local, e);
            }
            walk_tree(outermost.target, *this);
            walk_tree(outermost.ifs, *this);
            for(size_t i = 1; i < generators.size(); ++i) {
                walk_tree(*generators[i], *this);
            }
            if(value) {
                walk_tree(*value, *this);
            }
            walk_tree(*element, *this);

            table->leave_block();
        }

        bool operator() (AstDictComp & d) {
            handle_comprehension(false, "dictcomp", d, d.generators, d.key, d.value);
            return false;
        }

        bool operator() (AstSetComp & s) {
            handle_comprehension(false, "setcomp", s, s.generators, s.element, {});
            return false;
        }

        bool operator() (AstGenerator & g) {
            handle_comprehension(false, "genexpr", g, g.generators, g.element, {});
            return false;
        }

        bool operator() (AstClassDef & c) {
            String const & name = get_name(c.name);     // We can keep the ref
            add_def(name, SymbolFlag_Local, c);

            walk_tree(c.bases, *this);
            walk_tree(c.decorators, *this);
            table->enter_block(BlockType::Class, name, c);
            String current_class;
            current_class.swap(table->current_class);
            table->current_class = name;

            walk_tree(*c.body, *this);

            table->leave_block();
            current_class.swap(table->current_class);

            return false;
        }

        bool operator() (AstLambda & l) {
            walk_tree(l.arguments.defaults, *this);
            table->enter_block(BlockType::Function, "<lambda>", l);
            arguments(l.arguments);
            walk_tree(l.body, *this);
            table->leave_block();
            return false;
        }

        bool operator() (AstReturn & r) {
            if(r.value) {
                table->current->returns_value = true;
                if(table->current->is_generator) {
                    PYPA_ADD_SYMBOL_ERR("Return value in generator", r);
                }
            }
            return true;
        }

        bool operator() (AstModule & m) {
            table->enter_block(BlockType::Module, table->file_name, m);
            table->current->unoptimized = OptimizeFlag_TopLevel;
            walk_tree(m.body, *this);
            table->leave_block();
            return false;
        }

        bool operator() (AstYieldExpr & y) {
            table->current->is_generator = true;
            if(table->current->returns_value) {
                PYPA_ADD_SYMBOL_ERR("Return value in generator", y);
            }
            return true;
        }

        bool operator() (AstImportFrom & i) {
            if(i.names) visit(*this, *i.names);
            return true;
        }

        bool operator() (AstImport & i) {
            if(i.names) visit(*this, *i.names);
            return false;
        }

        bool operator() (AstAlias & a) {
            AstName & n = *std::static_pointer_cast<AstName>(a.as_name ? a.as_name : a.name);
            String name = n.id;
            if(n.dotted) {
                size_t pos = name.find_first_of('.');
                assert(String::npos != pos);
                name.erase(pos);
            }
            if(name == "*") {
                if(table->current->type != BlockType::Module) {
                    PYPA_ADD_SYMBOL_WARN("Import * only allowed at module level", n);
                }
                table->current->unoptimized |= OptimizeFlag_ImportStar;
                table->current->opt_last_line = n.line;
            }
            else {
                add_def(name, SymbolFlag_Import, a);
            }
            return false;
        }

        bool operator() (AstExec & e) {
            table->current->opt_last_line = e.line;
            if(e.globals) {
                table->current->unoptimized |= OptimizeFlag_Exec;
            }
            else {
                table->current->unoptimized |= OptimizeFlag_BareExec;
            }
            return true;
        }

        bool operator() (AstContinue & c) {
            if(!table->current->in_loop) {
                PYPA_ADD_SYMBOL_ERR("'continue' not properly in loop", c);
            }
            else if(table->current->in_finally) {
                PYPA_ADD_SYMBOL_ERR("'continue' not supported inside 'finally' clause", c);
            }
            return true;
        }

        bool operator() (AstBreak & b) {
            if(!table->current->in_loop) {
                PYPA_ADD_SYMBOL_ERR("'break' outside loop", b);
            }
            return true;
        }

        bool operator() (AstTryFinally & t) {
            walk_tree(t.body, *this);
            bool previous = table->current->in_finally;
            if(t.final_body) {
                table->current->in_finally = true;
                walk_tree(t.final_body, *this);
                table->current->in_finally = previous;
            }
            return false;
        }

        bool operator() (AstFor & f) {
            visit(*this, f.target);
            visit(*this, f.iter);
            bool previous = table->current->in_loop;
            bool previous_finally = table->current->in_finally;
            table->current->in_loop = true;
            table->current->in_finally = false;
            walk_tree(f.body, *this);
            table->current->in_loop = previous;
            table->current->in_finally = previous_finally;
            walk_tree(f.orelse, *this);
            return false;
        }

        bool operator() (AstWhile & f) {
            visit(*this, f.test);
            bool previous = table->current->in_loop;
            bool previous_finally = table->current->in_finally;
            table->current->in_loop = true;
            table->current->in_finally = false;
            walk_tree(f.body, *this);
            table->current->in_loop = previous;
            table->current->in_finally = previous_finally;
            walk_tree(f.orelse, *this);
            return false;
        }

        bool operator() (AstName & n) {
            add_def(n.id, n.context == AstContext::Load ? SymbolFlag_Used : SymbolFlag_Local, n);
            return false;
        }

        template< typename T >
        bool operator ()(T)
        {
            return true;
        }
    };


}

#endif // GUARD_PYPA_PARSER_SYMBOL_TABLE_VISITOR_HH_INCLUDED
