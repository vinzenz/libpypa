#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

import ast
PADDING = '    '


def member_filter(item):
    if item in ('col_offset', 'lineno'):
        return False
    return not item.startswith("_")


def print_ast(a, pad=''):
    if isinstance(a, (list, tuple)):
        print '['
        for i in a:
            print_ast(i, pad + PADDING)
        print '%s]' % pad
    elif isinstance(a, (basestring, int, float, long, complex)) or a is None:
        print a
    elif isinstance(a, (ast.Load, ast.Store, ast.AugLoad, ast.AugStore,
                        ast.Param, ast.Del)):
        print a.__class__.__name__
    else:
        print '\n%s[%s]' % (pad, a.__class__.__name__)
        items = [i for i in dir(a) if member_filter(i)]
        for i in items:
            print '%s%s- %s:' % (pad, PADDING, i),
            print_ast(getattr(a, i), pad + PADDING)
#    else:
#        print a.__class__.__name__, [i for i in dir(a) if member_filter(i)]


if __name__ == '__main__':
    import sys
    print_ast(ast.parse(open(sys.argv[1], 'r').read()))
