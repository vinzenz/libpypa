def run(self, cmd, globals=None, locals=None):
    if globals is None:
        import __main__
        globals = __main__.__dict__
    if locals is None:
        locals = globals
    self.reset()
    sys.settrace(self.trace_dispatch)
    if not isinstance(cmd, types.CodeType):
        cmd = cmd+'\n'
    try:
        exec cmd in globals, locals
    except BdbQuit:
        pass
    finally:
        self.quitting = 1
        sys.settrace(None)
