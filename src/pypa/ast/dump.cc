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
#include <pypa/ast/visitor.hh>
#include <cassert>

namespace pypa {

struct dump_visitor {
    template<typename T> // Fallback visitor
    void operator()(T const & t) {
        printf("No defined visitor for AstType #%d\n", int(t.TYPE));
    }

    void operator() (AstModule const & p) {
        printf("Module:\n\n");
        visit(p.body);
    }

    void operator() (AstSuite const & a) {
        printf("Suite:\n");
        visit(a.items);
        printf("\n");
    }

    void operator() (AstNone const &) {
        printf("None");
    }

    void operator() (AstListComp const & p) {
        printf("ListComp: Element: "); visit(p.element);
        printf("Generator: "); visit(p.generators);
    }

    void operator() (AstGenerator const & p) {
        printf("Generator: "); visit(p.expression);
        printf(" "); visit(p.for_expr);
    }

    void operator() (AstDictComp const & p) {
        printf("DictComp: Key: "); visit(p.key);
        printf(" Value: "); visit(p.value);
        printf(" Generators: "); visit(p.generators);
    }

    void operator() (AstComprehension const & p) {
        printf("Comprehension: "); visit(p.target);
        printf("Iter: "); visit(p.iter);
    }

    void operator() (AstForExpr const & p) {
        printf("ForExpr: Target: "); visit(p.items);
        printf(" Generator: "); visit(p.generators);
        printf(" Iter: "); visit(p.iter);
    }

    void operator() (AstBinOp const & p) {
        printf("BinOp: "); visit(p.left);
        printf(" "); visit(p.op); printf(" "); visit(p.right);
    }

    void operator() (AstDecorator const & p) {
        printf("Decorator: "); visit(p.name);
        printf(" Arguments: "); visit(p.arguments);
    }

    void operator() (AstDecorated const & p) {
        printf("Decorated: "); visit(p.decorators); visit(p.cls_or_fun_def);
    }

    void operator() (AstDecorators const & p) {
        printf("Decorators: "); visit(p.decorators);
    }

    void operator() (AstDelete const & p) {
        printf("Delete: "); visit(p.targets);
    }

    void operator() (AstFor const & p) {
        printf("For: "); visit(p.target);
        printf(" in: "); visit(p.iter);
        printf(" Body: "); visit(p.body);
        printf(" Else: "); visit(p.orelse);
    }

    void operator() (AstIf const & p) {
        printf("If: "); visit(p.test);
        printf(" Body: "); visit(p.body);
        printf(" ElIf: "); visit(p.elif);
        printf(" Else: "); visit(p.orelse);
    }

    void operator() (AstImportFrom const & p) {
        printf("ImportFrom: From: "); visit(p.module);
        printf(" Import: "); visit(p.names);
        printf(" Level: %d\n", p.level);
    }

    void operator() (AstImport const & p) {
        printf("ImportFrom: From: "); visit(p.names);
    }

    void visit(AstBinOpType op) {
        switch(op) {
        case AstBinOpType::Add:         printf("+"); break;
        case AstBinOpType::BitAnd:      printf("&"); break;
        case AstBinOpType::BitOr:       printf("|"); break;
        case AstBinOpType::BitXor:      printf("^"); break;
        case AstBinOpType::Div:         printf("/"); break;
        case AstBinOpType::FloorDiv:    printf("//"); break;
        case AstBinOpType::LeftShift:   printf("<<"); break;
        case AstBinOpType::Mod:         printf("%%"); break;
        case AstBinOpType::Mult:        printf("*"); break;
        case AstBinOpType::Power:       printf("**"); break;
        case AstBinOpType::RightShift:  printf(">>"); break;
        case AstBinOpType::Sub:         printf("-"); break;
        default:
            assert("No such binop" && false);
        }
    }

    void operator() (AstAugAssign const & p) {
        printf("AugAssign Target: "); visit(p.target);
        printf(" "); visit(p.op); printf("= "); visit(p.value);
    }

    void visit(AstCompareOpType op) {
        switch(op) {
        case AstCompareOpType::Equals:      printf("=="); break;
        case AstCompareOpType::In:          printf("in"); break;
        case AstCompareOpType::Is:          printf("is"); break;
        case AstCompareOpType::IsNot:       printf("is not"); break;
        case AstCompareOpType::Less:        printf("<"); break;
        case AstCompareOpType::LessEqual:   printf("<="); break;
        case AstCompareOpType::More:        printf(">"); break;
        case AstCompareOpType::MoreEqual:   printf(">="); break;
        case AstCompareOpType::NotEqual:    printf("!="); break;
        case AstCompareOpType::NotIn:       printf("not in"); break;
        }
    }

    void operator() (AstUnaryOp const & p) {
        printf("UnaryOp: "); visit(p.op); visit(p.operand);
    }

    void operator() (AstExcept const & p) {
        printf("Except: "); visit(p.type); printf(" as "); visit(p.name);
        printf(" Body: "); visit(p.body);
    }

    void operator() (AstLambda const & p) {
        printf("Lambda: "); visit(p.arguments);
        printf(" Body: "); visit(p.body);
    }

    void operator() (AstBreak const &) {
        printf("Break");
    }

    void visit(AstUnaryOpType op) {
        switch(op) {
        case AstUnaryOpType::Add:    printf("+"); break;
        case AstUnaryOpType::Invert: printf("~"); break;
        case AstUnaryOpType::Not:    printf("not"); break;
        case AstUnaryOpType::Sub:    printf("-"); break;
        }
    }

    void visit(AstBoolOpType op) {
        switch(op) {
        case AstBoolOpType::And:    printf("and"); break;
        case AstBoolOpType::Or:     printf("or"); break;
        }
    }

    void operator () (AstCompare const & p) {
        printf("Compare: "); visit(p.left);
        printf(" "); visit(p.op); printf(" ");
        visit(p.right);
    }

    void operator() (AstArguments const & a) {
       if(!a.defaults.empty()) {
            assert(a.defaults.size() == a.arguments.size());
            if(a.defaults.size() == a.arguments.size()) {
                for(size_t i = 0; i < a.defaults.size(); ++i) {
                    visit(a.arguments[i]); printf("="); visit(a.defaults[i]); printf(", ");
                }
            }

        }
        else {
           visit(a.arguments);
        }
        if(!a.args.empty()) {
            printf(", *"); visit(a.args);
        }
        if(!a.keywords.empty()) {
            assert(a.keywords.empty() || a.defaults.empty());
            printf(", "); visit(a.keywords);
        }
        if(a.kwargs) {
            printf(", **"); visit(a.kwargs);
        }
    }

    void operator() (AstYieldExpr const & a) {
        printf("Yield: ");
        visit(a.args);
    }

    void operator() (AstYield const & a) {
        printf("\t- ");
        visit(a.yield); printf("\n");
    }

    void operator() (AstFunctionDef const & p) {
        printf("FunctionDef:"); visit(p.name);
        printf("\nArguments: "); visit(p.args);
        printf("\nBody:");
        visit(p.body);
    }

    void operator() (AstDict const & p) {
        printf("Dict {");
        assert(p.keys.size() == p.values.size());
        for(std::size_t i = 0; i < p.keys.size(); ++i) {
            if(i) printf(", "); visit(p.keys[i]); printf(": "); visit(p.values[i]);
        }
        printf("}");
    }

    void operator() (AstSet const & p) {
        printf("Set {");
        for(std::size_t i = 0; i < p.elements.size(); ++i) {
            if(i) printf(", "); visit(p.elements[i]);
        }
        printf("}");
    }

    void operator() (AstList const & p) {
        printf("List [");
        for(auto e : p.elements) {
            visit(e); printf(", ");
        }
        printf("]\n");
    }

    void operator() (AstExpressionStatement const & p) {
        visit(p.expr);
    }

    void operator() (AstExpressions const & p) {
        visit(p.items);
    }

    void operator() (AstKeyword const & p) {
        printf("KWName: "); visit(p.name);
        printf("KWValue: ");visit(p.value);
    }

    void operator() (AstCall const & p) {
        printf("Call:\n\tFunction:");
        visit(p.function);
        printf("\n\t\tArgs:");
        visit(p.arglist);
        printf("\n");
    }

    void operator() (AstTuple const & p) {
        printf("Tuple (");
        for(std::size_t i = 0; i < p.elements.size(); ++i) {
            visit(p.elements[i]); printf(", ");
        }
        printf(")");
    }

    void visit(AstContext p) {
        printf("[Context: ");
        switch(p) {
        case AstContext::AugLoad:   printf("AugLoad"); break;
        case AstContext::AugStore:  printf("AugStore"); break;
        case AstContext::Load:      printf("Load"); break;
        case AstContext::Store:     printf("Store"); break;
        case AstContext::Param:     printf("Param"); break;
        case AstContext::Del:       printf("Del"); break;
        }
        printf("] ");
    }

    void operator() (AstStr const & p) {
        printf("[STR] (%s)", p.value.c_str());
    }

    void operator() (AstStatement const & s) {
        printf("[Statement? %d]\n", int(s.type));
    }

    void operator() (AstExpression const & s) {
        printf("[Expression? %d]\n", int(s.type));
    }

    void operator() (AstAssign const & p) {
        printf("Assign: Target: "); visit(p.targets);
        printf(" = Value: "); visit(p.value);
    }

    void operator() (AstAttribute const & p) {
        printf("Attribute: Value: "); visit(p.value);
        printf("AttrName: "); visit(p.attribute);
    }

    void operator() (AstWithItem const & p) {
        printf("WithItem:\n\tContext:");
        visit(p.context);
        printf("\n\tOptional:");
        visit(p.optional);
        printf("\n");
    }

    void operator() (AstComplex const & p) {
        printf("Complex: %s", p.complex.c_str());
    }

    void operator() (AstContinue const &) {
        printf("Continue");
    }

    void operator() (AstElseIf const & p) {
        printf("ElseIf: "); visit(p.test);
        printf(" Body: "); visit(p.body);
    }

    void operator() (AstRaise const & p) {
        printf("Raise: "); visit(p.arg0); printf(", "); visit(p.arg1); printf(", "); visit(p.arg2);
    }

    void operator() (AstReturn const & p) {
        printf("Return: "); visit(p.value);
    }

    void operator() (AstTryExcept const & p) {
        printf("TryExcept: "); visit(p.body);
        printf(" ExceptHandlers: "); visit(p.handlers);
        printf(" OrElse: "); visit(p.orelse);
    }

    void operator() (AstTryFinally const & p) {
        printf("AstTryFinally: "); visit(p.body);
        printf(" Finally: "); visit(p.final_body);
    }

    void operator() (AstBoolOp const & p) {
        printf("BoolOp: Op: "); visit(p.op); printf(" Values: "); visit(p.values);
    }

    void operator() (AstWhile const & p) {
        printf("While: "); visit(p.test);
        printf(" Body"); visit(p.body);
        printf(" Else"); visit(p.orelse);
    }

    void operator() (AstIfExpr const & p) {
        printf("IfExpr: "); visit(p.test);
        printf(" Body: "); visit(p.body);
        printf(" Else: "); visit(p.orelse);
    }

    void operator() (AstClassDef const & p) {
        printf("ClassDef: "); visit(p.name);
        printf("Bases: "); visit(p.bases);
        printf("Body: "); visit(p.body);
    }

    void operator() (AstExtSlice const & p) {
        printf("ExtSlice Dims: "); visit(p.dims);
    }

    void operator() (AstSlice const & p) {
        printf(" Lower: "); visit(p.lower);
        printf(" Upper: "); visit(p.upper);
        printf(" Step: "); visit(p.step);
    }

    void operator() (AstSubscript const & p) {
        printf("\n\tSubscript: Value: ");
        visit(p.value);
        printf(" - Slice: ");
        visit(p.slice);
        printf("\n");
    }

    void operator() (AstIndex const & p) {
        printf("Index:"); visit(p.value);
    }

    void operator() (AstWith const & p) {
        printf("With:\n\tItems: ");
        visit(p.items);
        printf("\tBody:\n");
        visit(p.body);
        printf("\n");
    }

    void operator() (AstPrint const & p) {
        printf("\t- Print ");
        if(p.destination) { printf(" >> "); visit(p.destination); printf(","); }
        visit(p.values);
        printf("\n");
    }

    void operator() (AstNumber const & n) {
        if(n.num_type == AstNumber::Float) printf("Number: %g", n.floating);
        else printf("Number: %lli", n.integer);
    }

    void operator() (AstPass const &) {
        printf("\t- Pass\n");
    }

    void operator() (AstName const & p) {
        printf("Name: %s ", p.id.c_str());
        visit(p.context);
        printf(", ");
    }

    void operator() (AstBool const & p) {
        printf("%s", p.value ? "True" : "False");
    }

    template< typename T >
    void visit(std::vector<T> const & v) {
        if(v.empty()) printf("[]");
        else for(auto e : v) {
            visit(e);
        }
    }

    void visit(Ast const & p) {
        pypa::visit(*this, p);
    }

    void visit(AstPtr const p) {
        if(p) {
            pypa::visit(*this, *p);
        } else {
            printf("<NULL>");
        }
    }
};

void dump(AstPtr p) {
    visit(dump_visitor(), *p);
}

}
