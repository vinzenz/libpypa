AM_CPPFLAGS=-DIEEE_8087

lib_LTLIBRARIES=libpypa.la
libpypa_la_LDFLAGS=$(PYPA_LDFLAGS) -lgmp
libpypa_la_SOURCES=\
	pypa/ast/ast.cc \
	pypa/ast/dump.cc \
	pypa/filebuf.cc \
	pypa/lexer/lexer.cc \
	pypa/parser/parser.cc \
	pypa/parser/make_string.cc \
	pypa/parser/symbol_table.cc \
	double-conversion/src/bignum-dtoa.cc \
	double-conversion/src/bignum.cc \
	double-conversion/src/cached-powers.cc \
	double-conversion/src/diy-fp.cc \
	double-conversion/src/double-conversion.cc \
	double-conversion/src/fast-dtoa.cc \
	double-conversion/src/fixed-dtoa.cc \
	double-conversion/src/strtod.cc \
	$(NULL)

noinst_PROGRAMS=lexer-test parser-test
lexer_test_SOURCES=\
	pypa/lexer/test.cc \
	$(NULL)
lexer_test_LDADD=libpypa.la

parser_test_SOURCES=\
	pypa/parser/test.cc \
	$(NULL)
parser_test_LDADD=libpypa.la

check-local:lexer-test parser-test $(srcdir)/run-tests.sh
	CPYTHON_SRC=$(CPYTHON_SRC) $(srcdir)/run-tests.sh

pypadir=$(includedir)/pypa
pypa_HEADERS=\
	pypa/filebuf.hh \
	pypa/reader.hh \
	pypa/types.hh \
	$(NULL)


pypaparserdir=$(includedir)/pypa/parser
pypaparser_HEADERS=\
	pypa/parser/apply.hh \
	pypa/parser/error.hh \
	pypa/parser/future_features.hh \
	pypa/parser/parser.hh \
	pypa/parser/parser_fwd.hh \
	pypa/parser/state.hh \
	pypa/parser/symbol_table.hh \
	pypa/parser/symbol_table_visitor.hh \
	$(NULL)

pypaastdir=$(includedir)/pypa/ast
pypaast_HEADERS=\
	pypa/ast/ast.hh \
	pypa/ast/ast_type.inl \
	pypa/ast/base.hh \
	pypa/ast/context_assign.hh \
	pypa/ast/dump.hh \
	pypa/ast/macros.hh \
	pypa/ast/tree_walker.hh \
	pypa/ast/types.hh \
	pypa/ast/visitor.hh \
	$(NULL)

pypalexerdir=$(includedir)/pypa/lexer
pypalexer_HEADERS=\
	pypa/lexer/delim.hh \
	pypa/lexer/keyword.hh \
	pypa/lexer/lexer.hh \
	pypa/lexer/op.hh \
	pypa/lexer/tokendef.hh \
	pypa/lexer/tokens.hh \
	$(NULL)

EXTRA_DIST=double-conversion
