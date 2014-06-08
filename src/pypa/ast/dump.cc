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
    }

    void operator() (AstArguments const & a) {
       visit(a.arguments);
       if(!a.args.empty()) { printf("*%s, ", a.args.c_str()); }
       if(!a.kwargs.empty()) { printf("**%s, ", a.kwargs.c_str()); }
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
        printf("FunctionDef: %s\n", p.name.c_str());
        printf("Decorators:  "); visit(p.decorators);
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
        for(std::size_t i = 0; i < p.elements.size(); ++i) {
            if(i) printf(", "); visit(p.elements[i]);
        }
        printf("]");
    }

    void operator() (AstExpressionStatement const & p) {
        visit(p.expr);
    }

    void operator() (AstExpressions const & p) {
        visit(p.items);
    }

    void operator() (AstKeyword const & p) {
        printf("KWName: %s KWValue: ", p.name.c_str());
        visit(p.value);
    }

    void operator() (AstKeywordExpr const & p) {
        visit(p.kw);
    }

    void operator() (AstCall const & p) {
        printf("Call:\n\tFunction:");
        visit(p.function);
        printf("\n\tPositional Arguments:");
        visit(p.arguments);
        printf("\n\t*args:");
        visit(p.args);
        printf("\n\tKeyword Arguments:");
        visit(p.keywords);
        printf("\n\t**kwargs:");
        visit(p.kwargs);
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

    void operator() (AstWithItem const & p) {
        printf("WithItem:\n\tContext:");
        visit(p.context);
        printf("\n\tOptional:");
        visit(p.optional);
        printf("\n");
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
