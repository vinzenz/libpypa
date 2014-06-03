PYPA_CXXFLAGS=-std=c++11 -I$(top_srcdir)/src $(CXXFLAGS)

lib_LTLIBRARIES=libpypa.la
libpypa_la_CXXFLAGS=$(PYPA_CXXFLAGS)
libpypa_la_SOURCES=\
	pypa/filebuf.cc \
	pypa/ast/ast.cc \
	pypa/lexer/lexer.cc \
	pypa/parser/parser.cc \
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
