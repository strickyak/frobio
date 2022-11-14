import re, sys

def Chopper(filename):
    Input = open(filename)
    SrcName = re.sub(r'[^A-Za-z0-9]', '_', filename)

    MATCH_CHOP = re.compile(r'^\s*[/][/]\s*chop\s*$').match
    MATCH_EMPTY = re.compile(r'^\s*$').match
    MATCH_COMMENT = re.compile(r'^\s*[/][/].*$').match

    LineNo = 0
    Head = []
    Buf = []
    K = -1
    Something = False

    def Flush(k):
        with open('__%05d_%s_.c' % (k, SrcName), 'w') as w:
            i = 0
            for s in Head:
                i += 1
                print(s, file=w)
            n = LineNo - len(Buf)
            while i < n:
                i += 1
                print('/*%d*/' % i, file=w)
            for s in Buf:
                i += 1
                print(s, file=w)

    Headlines = True
    for line in Input:
        line = line.rstrip()
        print('# Line %s', repr(line))
        Buf.append(line)
        LineNo += 1
        if line == "}" or MATCH_CHOP(line):
            print('## chopped')
            K += 1
            if Headlines:
                Head = Buf
                Buf = []
            else:
                Flush(K)
            Something = False
            Buf = []
            Headlines = False
        elif MATCH_EMPTY(line) or MATCH_COMMENT(line):
            print('## empty')
            pass
        else:
            print('## other')
            Something = True
            pass

    K += 1
    Flush(K)


if len(sys.argv) < 2:
    raise Exception("Usage:  python3 chopper.py file1.c file2.c ...")

for arg in sys.argv[1:]:
    Chopper(arg)
