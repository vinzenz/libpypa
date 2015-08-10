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
#if !(defined(GUARD_PYPA_AST_AST_HH_INCLUDED) && defined(GUARD_PYPA_AST_TYPES_HH_INCLUDED))
#   error "This must be only included by pypa/ast/ast.hh"
#endif
#ifndef GUARD_PYPA_AST_MACROS_HH_INCLUDED
#define GUARD_PYPA_AST_MACROS_HH_INCLUDED


#define DEF_AST_TYPE_BY_ID(ID, TYPE)                \
    template<> struct AstIDByType<TYPE> {           \
        static constexpr AstType Id = AstType::ID;  \
        static char const * name() {                \
            return #ID;                             \
        }                                           \
    };                                              \
                                                    \
    template<> struct AstTypeByID<AstType::ID> {    \
        typedef TYPE Type;                          \
        static char const * name() {                \
            return #ID;                             \
        }                                           \
    }


#define DEF_AST_TYPE_BY_ID1(TYPE) DEF_AST_TYPE_BY_ID(TYPE, Ast##TYPE)


#define PYPA_AST_TYPE_DECL_ALIAS(NAME, ALIASPTR, ALIASLIST) \
    typedef std::shared_ptr<struct NAME> ALIASPTR;          \
    typedef std::vector<ALIASPTR> ALIASLIST;                \
    struct NAME


#define PYPA_AST_TYPE_DECL(NAME)                            \
    PYPA_AST_TYPE_DECL_ALIAS(NAME, NAME##Ptr, NAME##List)


#define PYPA_AST_TYPE_DECL_DERIVED_TPL(NAME, TPL)       \
    PYPA_AST_TYPE_DECL(Ast##NAME) : TPL<AstType::NAME>


#define PYPA_AST_TYPE_DECL_DERIVED(NAME)        \
    PYPA_AST_TYPE_DECL_DERIVED_TPL(NAME, AstT)


#define PYPA_AST_TYPE_DECL_DERIVED_ALIAS(NAME, ALIASPTR, ALIASLIST)                 \
    PYPA_AST_TYPE_DECL_ALIAS(Ast##NAME, ALIASPTR, ALIASLIST) : AstT<AstType::NAME>


#define PYPA_AST_TYPE_DECL_SLICE_KIND(NAME)         \
    PYPA_AST_TYPE_DECL_DERIVED_TPL(NAME, AstSliceT)


#define PYPA_AST_STMT(AST_TYPE)                                         \
    typedef std::shared_ptr<struct Ast##AST_TYPE> Ast##AST_TYPE##Ptr;   \
    DEF_AST_TYPE_BY_ID(AST_TYPE, struct Ast##AST_TYPE);                 \
    struct Ast##AST_TYPE : AstStmtT<AstType::AST_TYPE>


#define PYPA_AST_EXPR(AST_TYPE)                                         \
    typedef std::shared_ptr<struct Ast##AST_TYPE> Ast##AST_TYPE##Ptr;   \
    DEF_AST_TYPE_BY_ID(AST_TYPE, struct Ast##AST_TYPE);                 \
    struct Ast##AST_TYPE : AstExprT<AstType::AST_TYPE>

#define PYPA_AST_SLICE(AST_TYPE)                                        \
    typedef std::shared_ptr<struct Ast##AST_TYPE> Ast##AST_TYPE##Ptr;   \
    DEF_AST_TYPE_BY_ID(AST_TYPE, struct Ast##AST_TYPE);                 \
    struct Ast##AST_TYPE : AstSliceTypeT<AstType::AST_TYPE>

#define PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPEID)                                     \
    template<>                                                                      \
    struct ast_member_dump<Ast##TYPEID> {                                           \
        typedef Ast##TYPEID Type;                                                   \
        static void dump(int depth, std::shared_ptr<Ast##TYPEID> const & p) {       \
            if(p) dump(depth, *p);                                                  \
        }                                                                           \
        static void dump(int depth, Ast##TYPEID const & t) {                        \
            printf("\n");                                                           \
            pypa::detail::print_padding(depth);                                     \
            printf("[%s]\n", AstIDByType<Ast##TYPEID>::name());


#define PYPA_AST_MEMBER_DUMP_IMPL_END(TYPEID)                                       \
        }                                                                           \
        template< typename MemberType >                                             \
        static void dump_member(int depth, Type const & t, char const * name,       \
                                MemberType const Type::*m) {                        \
                pypa::detail::print_padding(depth);                                 \
                printf("  - %s: ", name);                                           \
                pypa::detail::dump_member_value(depth + 4, t.*m);                   \
        }                                                                           \
    };


#define PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARGNAME)              \
            dump_member(depth, t, #ARGNAME, &Type::ARGNAME);


#define PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPEID)                                \
    template<>                                                                  \
    struct ast_member_visit<AstType::TYPEID> {                                  \
        template<typename T, typename V, typename F>                            \
        static void do_apply(T t, V T::*v, F f) {                               \
            detail::apply_member(t, v, f);                                      \
        }                                                                       \
        template<typename T, typename V, typename F>                            \
        static void do_apply(std::shared_ptr<T> t, V T::*v, F f) {              \
            detail::apply_member(t, v, f);                                      \
        }                                                                       \
        template<typename T, typename F>                                        \
        static void apply(T t, F f) {                                           \
            typedef typename AstTypeByID<AstType::TYPEID>::Type Type;           \
            if(!f(t)) return;


#define PYPA_AST_MEMBER_VISIT_IMPL_END }}


#define PYPA_AST_MEMBERS0(TYPE)                 \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)       \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)         \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)      \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#define PYPA_AST_MEMBERS1(TYPE, ARG0)           \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)       \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG0)      \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)         \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)      \
    do_apply(t, &Type::ARG0, f);                \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#define PYPA_AST_MEMBERS2(TYPE, ARG0, ARG1)     \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)       \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG0)      \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG1)      \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)         \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)      \
    do_apply(t, &Type::ARG0, f);                \
    do_apply(t, &Type::ARG1, f);                \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#define PYPA_AST_MEMBERS3(TYPE, ARG0, ARG1, ARG2)       \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)               \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG0)              \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG1)              \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG2)              \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)                 \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)              \
    do_apply(t, &Type::ARG0, f);                        \
    do_apply(t, &Type::ARG1, f);                        \
    do_apply(t, &Type::ARG2, f);                        \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#define PYPA_AST_MEMBERS4(TYPE, ARG0, ARG1, ARG2, ARG3)     \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)                   \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG0)                  \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG1)                  \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG2)                  \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG3)                  \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)                     \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)                  \
    do_apply(t, &Type::ARG0, f);                            \
    do_apply(t, &Type::ARG1, f);                            \
    do_apply(t, &Type::ARG2, f);                            \
    do_apply(t, &Type::ARG3, f);                            \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#define PYPA_AST_MEMBERS5(TYPE, ARG0, ARG1, ARG2, ARG3, ARG4)   \
    PYPA_AST_MEMBER_DUMP_IMPL_BEGIN(TYPE)                       \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG0)                      \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG1)                      \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG2)                      \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG3)                      \
    PYPA_AST_MEMBER_DUMP_MEMBER_ITEM(ARG4)                      \
    PYPA_AST_MEMBER_DUMP_IMPL_END(TYPE)                         \
    PYPA_AST_MEMBER_VISIT_IMPL_BEGIN(TYPE)                      \
    do_apply(t, &Type::ARG0, f);                                \
    do_apply(t, &Type::ARG1, f);                                \
    do_apply(t, &Type::ARG2, f);                                \
    do_apply(t, &Type::ARG3, f);                                \
    do_apply(t, &Type::ARG4, f);                                \
    PYPA_AST_MEMBER_VISIT_IMPL_END


#endif //GUARD_PYPA_AST_MACROS_HH_INCLUDED
