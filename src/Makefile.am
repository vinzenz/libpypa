PYPA_CXXFLAGS=-std=c++11 -I$(top_srcdir)/src -fstack-protector-strong
AM_CFLAGS=-DIEEE_8087

lib_LTLIBRARIES=libpypa.la
libpypa_la_CXXFLAGS=$(PYPA_CXXFLAGS)
libpypa_la_SOURCES=\
	pypa/filebuf.cc \
	pypa/ast/ast.cc \
	pypa/ast/dump.cc \
	pypa/lexer/lexer.cc \
	pypa/parser/parser.cc \
	double-conversion/src/bignum.cc \
	double-conversion/src/bignum-dtoa.cc \
	double-conversion/src/cached-powers.cc \
	double-conversion/src/diy-fp.cc \
	double-conversion/src/double-conversion.cc \
	double-conversion/src/fast-dtoa.cc \
	double-conversion/src/fixed-dtoa.cc \
	double-conversion/src/strtod.cc \
	$(NULL)

noinst_PROGRAMS=lexer-test parser-test
lexer_test_CXXFLAGS=$(PYPA_CXXFLAGS)
lexer_test_SOURCES=\
	pypa/lexer/test.cc \
	$(NULL)
lexer_test_LDADD=libpypa.la

parser_test_CXXFLAGS=$(PYPA_CXXFLAGS)
parser_test_SOURCES=\
	pypa/parser/test.cc \
	$(NULL)
parser_test_LDADD=libpypa.la

pypadir=$(includedir)/pypa
pypa_HEADERS=\
	pypa/filebuf.hh \
	$(NULL)

pypaastdir=$(includedir)/pypa/ast
pypaast_HEADERS=\
	pypa/ast/ast.hh \
	$(NULL)

pypalexerdir=$(includedir)/pypa/lexer
pypalexer_HEADERS=\
	pypa/lexer/op.hh \
	pypa/lexer/keyword.hh \
	pypa/lexer/tokendef.hh \
	pypa/lexer/delim.hh \
	pypa/lexer/lexer.hh \
	pypa/lexer/tokens.hh \
	$(NULL)
