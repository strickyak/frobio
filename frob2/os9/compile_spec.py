import sys

class Compiler:
    def __init__(self):
        self.Reset()

    def Reset(self):
        self.name = None
        self.a = None
        self.b = None
        self.d = None
        self.x = None
        self.y = None
        self.u = None
        self.oa = None
        self.ob = None
        self.od = None
        self.ox = None
        self.oy = None
        self.ou = None
        self.os9 = None
        self.params = []

    def AddParams(self, p, isOut):
        self.params.append((p, isOut))

    def DoDef(self, args):
        self.name = args[0]

    def Flush(self):
        if not self.name:
            return
        s = '#define OS9_%s(' % self.name
        for (p, isOut) in self.params:
            s += p[-1] + ','
        if s.endswith(','):
            s = s[:-1]
        s += ') (\\\n'

        if self.a: s += '(nc.dab.ab.a = (%s)),\\\n' % self.a[-1]
        if self.b: s += '(nc.dab.ab.b = (%s)),\\\n' % self.b[-1]
        if self.d: s += '(nc.dab.d.d = (%s)),\\\n' % self.d[-1]

        if self.x: s += '(nc.x = (%s)),\\\n' % self.x[-1]
        if self.y: s += '(nc.y = (%s)),\\\n' % self.y[-1]
        if self.u: s += '(nc.u = (%s)),\\\n' % self.u[-1]

        s += '    nc.os9num = %s,\\\n' % self.os9[-1]
        s += '    NewCall(&nc),\\\n'

        if self.oa: s += '(*(%s) = nc.dab.ab.a),\\\n' % self.oa[-1]
        if self.ob: s += '(*(%s) = nc.dab.ab.b),\\\n' % self.ob[-1]
        if self.od: s += '(*(%s) = nc.dab.d.d),\\\n' % self.od[-1]

        if self.ox: s += '(*(%s) = nc.x),\\\n' % self.ox[-1]
        if self.oy: s += '(*(%s) = nc.y),\\\n' % self.oy[-1]
        if self.ou: s += '(*(%s) = nc.u),\\\n' % self.ou[-1]

        s += '    nc.err)\n\n'
        print(s)
        self.Reset()

    def DoWords(self, words):
        n = len(words)
        f = words[0]
        args = words[1:]

        if n==2 and f=='def':
            self.Flush()
            self.DoDef(args)

        elif n==3 and f=='def':
            self.Flush()
            self.DoDef(args)

        elif f=='a:':
            self.a = args
            self.AddParams(args, False)
        elif f=='b:':
            self.b = args
            self.AddParams(args, False)
        elif f=='d:':
            self.d = args
            self.AddParams(args, False)
        elif f=='x:':
            self.x = args
            self.AddParams(args, False)
        elif f=='y:':
            self.y = args
            self.AddParams(args, False)
        elif f=='u:':
            self.u = args
            self.AddParams(args, False)

        elif f=='*a:':
            self.oa = args
            self.AddParams(args, True)
        elif f=='*b:':
            self.ob = args
            self.AddParams(args, True)
        elif f=='*d:':
            self.od = args
            self.AddParams(args, True)
        elif f=='*x:':
            self.ox = args
            self.AddParams(args, True)
        elif f=='*y:':
            self.oy = args
            self.AddParams(args, True)
        elif f=='*u:':
            self.ou = args
            self.AddParams(args, True)

        elif f=='os9':
            self.os9 = args

        else:
            raise Exception('WUT? %d,%s' % (n, f))

compiler = Compiler()
for line in sys.stdin:
    if line.startswith('#'):
        continue

    words = line.split()
    if words:
        compiler.DoWords(words)
