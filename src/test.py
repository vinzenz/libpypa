def test(a, b, c, *args, **kwargs):
    with 1.22:
        print 1.2
    print {'s', 'e', 't'}
    print ['l', 'i', 's', 't']
    print ('t', 'u', 'p', 'l', 'e')
    print {'d': '1', 'i': '2', 'c': '3', 't': '4'}
    print [x for x in []]
    print [x for x in [] if x is not None]
    print 123456789
    print 0x75BCD15
    print 0b111010110111100110100010101
    print 0726746425
    print 0o726746425
    print "123456789"
    pass
    pass
    yield 1
    a(a, b, *args, c=d, x=y, **kwargs)
    subscript[idx]
    slicesimpl[slow:supper]
    sliceext[elow:eupper:estep]


class Foobar1:
    pass

class Foobar2(SomeBase):
    pass

class Foobar3(Base1, Base2):
    pass

class Foobar4(Base1, Base2):
    def __init__(self, *args, **kwargs):
        self.arg = args
