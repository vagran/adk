
class Conf(object):
    def __init__(self, SCons):
        self.SCons = SCons
    
    
def Apply(conf):
    s = conf.SCons.Script
    
    debug = s.ARGUMENTS.get('debug', 0)
    print('debug = ', debug)
    
    s.Program('hash.cpp')
