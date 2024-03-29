/* ncl: Modified and enhanced for cmoc for NitrOS9/OS9 by Strick Yak.  BSD licensed */

/* Was Picol: Tcl in ~ 500 lines of code by Salvatore antirez Sanfilippo.  BSD licensed */

#define BUF_SIZE 200            /* instead of 1024 */

extern int Error(char *argv0, int err);
extern int ResultD(int x);
extern int ResultS(const char *msg, const char *x);

// In order to get all code into a single assembly listing for the module,
// we include these C files instead of using the linker to link the usual stuff.

#include "frobio/nytypes.h"
#include "frobio/nylib.h"
#include "frobio/os9call.h"
#include "frobio/ncl/malloc.h"

#include "frobio/ncl/buf.h"
#include "frobio/ncl/puthex.h"
#include "frobio/ncl/re.h"
#include "frobio/ncl/std.h"
#include "frobio/ncl/util.h"

#undef HEAP_CHECKS             // Check heap integregity at key points.
#define TEAR_DOWN               // Free all memory structures at the end, to check for leak.  Use `;` to exit repl.

#define STRCMP strcasecmp // used to be strcasecmp
#define strcaseeq(A,B) (0==STRCMP((A),(B)))
#define bzero(A,B) memset((A), 0, (B))
#define puts Os9_puts

enum { PICOL_OK, PICOL_ERR, PICOL_RETURN, PICOL_BREAK, PICOL_CONTINUE };
enum { PT_ESC, PT_STR, PT_CMD, PT_VAR, PT_SEP, PT_EOL, PT_EOF };

struct picolParser {
  char *text;
  char *p;                      /* current text position */
  int len;                      /* remaining length */
  char *start;                  /* token start */
  char *end;                    /* token end */
  int type;                     /* token type, PT_... */
  int insidequote;              /* True if inside " " */
};

struct picolVar {
  char *name, *val;
  struct picolVar *next;
};

struct picolArray {
  char *name;
  struct picolVar *vars;
  struct picolArray *next;
};

typedef int (*picolCmdFunc)(int argc, char **argv, void *privdata);

struct picolCmd {
  char *name;
  byte minArgc, maxArgc;
  picolCmdFunc func;
  void *privdata;
  struct picolCmd *next;
};

struct picolCallFrame {
  struct picolVar *vars;
  struct picolCallFrame *parent;        /* parent is NULL at top level */
};

// Were in struct picolInterp; now are global:
struct picolCallFrame *Callframe;
struct picolCmd *Commands;
struct picolArray *Arrays;
char *Result;

#ifdef HEAP_CHECKS
void HC(void *p) {
  heap_check_block(((struct MallocHead *) p) - 1, 0);
}

void HeapCheckVars(struct picolVar *v) {
  while (v) {
    HC(v);
    HC(v->name);
    HC(v->val);
    v = v->next;
  }
}

void HeapCheck() {
  for (struct picolCmd * c = Commands; c; c = c->next) {
    HC(c);
    HC(c->name);
    if (c->privdata) {
      HC(((char **) (c->privdata))[0]);
      HC(((char **) (c->privdata))[1]);
      HC(c->privdata);
    }
  }
  for (struct picolCallFrame * f = Callframe; f; f = f->parent) {
    HC(f);
    HeapCheckVars(f->vars);
  }
  for (struct picolArray * a = Arrays; a; a = a->next) {
    HC(a);
    HC(a->name);
    HeapCheckVars(a->vars);
  }
  HC(Result);
}
#endif

#ifdef TEAR_DOWN
void TearDownVars(struct picolVar *v) {
  while (v) {
    free(v->name);
    free(v->val);
    void *tmp = v;
    v = v->next;
    free(tmp);
  }
}

void TearDown() {
  for (struct picolCmd * c = Commands; c;) {
    free(c->name);
    if (c->privdata) {
      free(((char **) (c->privdata))[0]);
      free(((char **) (c->privdata))[1]);
      free(c->privdata);
    }
    void *tmp = c;
    c = c->next;
    free(tmp);
  }
  if (Callframe->parent)
    panic("CF P");
  TearDownVars(Callframe->vars);
  free(Callframe);
  for (struct picolArray * a = Arrays; a;) {
    free(a->name);
    TearDownVars(a->vars);
    void *tmp = a;
    a = a->next;
    free(tmp);
  }
  free(Result);
}
#endif

void FreeDope(int c, const char **v) {
  for (int j = 0; j < c; j++)
    free((void *) v[j]);        // Free the strings.
  free(v);                      // Free the vector.
}

char static_buf_d[16];
char *StaticD(int x) {
  bzero(static_buf_d, 16);
  snprintf_d(static_buf_d, 15, "%d", x);
  return static_buf_d;
}

void picolInitParser(struct picolParser *p, const char *text) {
  p->text = p->p = (char *) text;
  p->len = strlen(text);
  p->start = 0;
  p->end = 0;
  p->insidequote = 0;
  p->type = PT_EOL;
}

int picolParseSep(struct picolParser *p) {
  p->start = p->p;
  while (*p->p == ' ' || *p->p == '\t' || *p->p == '\n' || *p->p == '\r') {
    p->p++;
    p->len--;
  }
  p->end = p->p - 1;
  p->type = PT_SEP;
  return PICOL_OK;
}

int picolParseEol(struct picolParser *p) {
  p->start = p->p;
  while (*p->p == ' ' || *p->p == '\t' || *p->p == '\n' || *p->p == '\r' || *p->p == ';') {
    p->p++;
    p->len--;
  }
  p->end = p->p - 1;
  p->type = PT_EOL;
  return PICOL_OK;
}

int picolParseCommand(struct picolParser *p) {
  int level = 1;
  int blevel = 0;
  p->start = ++p->p;
  p->len--;
  while (1) {
    if (p->len == 0) {
      break;
    } else if (*p->p == '[' && blevel == 0) {
      level++;
    } else if (*p->p == ']' && blevel == 0) {
      if (!--level)
        break;
    } else if (*p->p == '\\') {
      p->p++;
      p->len--;
    } else if (*p->p == '{') {
      blevel++;
    } else if (*p->p == '}') {
      if (blevel != 0)
        blevel--;
    }
    p->p++;
    p->len--;
  }
  p->end = p->p - 1;
  p->type = PT_CMD;
  if (*p->p == ']') {
    p->p++;
    p->len--;
  }
  return PICOL_OK;
}

int picolParseVar(struct picolParser *p) {
  p->start = ++p->p;
  p->len--;                     /* skip the $ */
  while (1) {
    if ((*p->p >= 'a' && *p->p <= 'z')
        || (*p->p >= 'A' && *p->p <= 'Z') || (*p->p >= '0' && *p->p <= '9')
        || *p->p == '_') {
      p->p++;
      p->len--;
      continue;
    }
    break;
  }
  if (p->start == p->p) {       /* It's just a single char string "$" */
    p->start = p->end = p->p - 1;
    p->type = PT_STR;
  } else {
    p->end = p->p - 1;
    p->type = PT_VAR;
  }
  return PICOL_OK;
}

int picolParseBrace(struct picolParser *p) {
  int level = 1;
  p->start = ++p->p;
  p->len--;
  while (1) {
    if (p->len >= 2 && *p->p == '\\') {
      p->p++;
      p->len--;
    } else if (p->len == 0 || *p->p == '}') {
      level--;
      if (level == 0 || p->len == 0) {
        p->end = p->p - 1;
        if (p->len) {
          p->p++;
          p->len--;             /* Skip final closed brace */
        }
        p->type = PT_STR;
        return PICOL_OK;
      }
    } else if (*p->p == '{')
      level++;
    p->p++;
    p->len--;
  }
  return PICOL_OK;              /* unreached */
}

int picolParseString(struct picolParser *p) {
  int newword = (p->type == PT_SEP || p->type == PT_EOL || p->type == PT_STR);
  if (newword && *p->p == '{')
    return picolParseBrace(p);
  else if (newword && *p->p == '"') {
    p->insidequote = 1;
    p->p++;
    p->len--;
  }
  p->start = p->p;
  while (1) {
    if (p->len == 0) {
      p->end = p->p - 1;
      p->type = PT_ESC;
      return PICOL_OK;
    }
    switch (*p->p) {
    case '\\':
      if (p->len >= 2) {
        p->p++;
        p->len--;
      }
      break;
    case '$':
    case '[':
      p->end = p->p - 1;
      p->type = PT_ESC;
      return PICOL_OK;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case ';':
      if (!p->insidequote) {
        p->end = p->p - 1;
        p->type = PT_ESC;
        return PICOL_OK;
      }
      break;
    case '"':
      if (p->insidequote) {
        p->end = p->p - 1;
        p->type = PT_ESC;
        p->p++;
        p->len--;
        p->insidequote = 0;
        return PICOL_OK;
      }
      break;
    }
    p->p++;
    p->len--;
  }
  return PICOL_OK;              /* unreached */
}

int picolParseComment(struct picolParser *p) {
  while (p->len && *p->p != '\n' && *p->p != '\r') {
    p->p++;
    p->len--;
  }
  return PICOL_OK;
}

int picolGetToken(struct picolParser *p) {
TOP:
  while (1) {
    if (!p->len) {
      if (p->type != PT_EOL && p->type != PT_EOF)
        p->type = PT_EOL;
      else
        p->type = PT_EOF;
      return PICOL_OK;
    }
    switch (*p->p) {
    case ' ':
    case '\t':
      if (p->insidequote)
        return picolParseString(p);
      return picolParseSep(p);
    case '\n':
    case '\r':
    case ';':
      if (p->insidequote)
        return picolParseString(p);
      return picolParseEol(p);
    case '[':
      return picolParseCommand(p);
    case '$':
      return picolParseVar(p);
    case '#':
      if (p->type == PT_EOL) {
        picolParseComment(p);
        goto TOP;               /* continue; */
      }
      return picolParseString(p);
    default:
      return picolParseString(p);
    }
  }
  return PICOL_OK;              /* unreached */
}

const char *Explode(char *s, int n) {
  if (n < 0)
    n = strlen(s);

  struct Buf result;
  BufInit(&result);
  for (int i = 0; i < n; i++) {
    char tmp[8];
    snprintf_d(tmp, 10, "%d", s[i]);
    BufAppElemS(&result, tmp);
  }
  BufFinish(&result);

  return BufTake(&result);
}

void picolInitInterp() {
  Callframe = (struct picolCallFrame *) malloc(sizeof(struct picolCallFrame));
  Callframe->vars = NULL;
  Callframe->parent = NULL;
  Commands = NULL;
  Arrays = NULL;
  Result = strdup("");
}

void picolAppendResult(const char *s) {
  int sn = strlen(s);
  int rn = strlen(Result);
  Result = realloc(Result, sn + rn + 1);
  strcat(Result, s);
}

void picolSetResult(const char *s) {
  free(Result);
  Result = strdup(s);
}

void picolMoveToResult(const char *s) {
  free(Result);
  Result = (char *) s;
}

int EmptyOrError(int e, char **argv) {
  if (e)
    return Error(argv[0], e);
  picolSetResult("");
  return PICOL_OK;
}

int IntOrError(int e, char **argv, int x) {
  if (e)
    return Error(argv[0], e);
  return ResultD(x);
}

struct picolVar *picolGetVarFromRoot(struct picolVar *v, const char *name) {
  for (; v; v = v->next) {
    if (STRCMP(v->name, name) == 0)
      return v;
  }
  return NULL;
}

struct picolVar *picolGetVar(const char *name) {
  return picolGetVarFromRoot(Callframe->vars, name);
}

int picolSetVarFromRoot(struct picolVar **root, const char *name, const char *val) {
  struct picolVar *v = picolGetVarFromRoot(*root, name);
  if (v) {
    free(v->val);
    v->val = strdup(val);
  } else {
    v = (struct picolVar *) malloc(sizeof(*v));
    v->name = strdup(name);
    v->val = strdup(val);
    v->next = *root;
    *root = v;
  }
  return PICOL_OK;
}

int picolSetVar(const char *name, const char *val) {
  return picolSetVarFromRoot(&Callframe->vars, name, val);
}

struct picolCmd *picolGetCommand(const char *name) {
  for (struct picolCmd * c = Commands; c; c = c->next) {
    if (STRCMP(c->name, name) == 0) {
      return c;
    }
  }
  return NULL;
}

struct picolArray *picolGetArray(const char *name) {
  for (struct picolArray * p = Arrays; p; p = p->next) {
    if (STRCMP(p->name, name) == 0) {
      return p;
    }
  }
  return NULL;
};

int picolRegisterCommand(const char *name, byte minArgc, byte maxArgc, picolCmdFunc f,
                         void *privdata) {
  struct picolCmd *c = picolGetCommand(name);
  if (c) {
    if (c->privdata && c->func == picolCommandCallProc) {
      // procdata always has two malloced slots.
      free(((char **) c->privdata)[0]);
      free(((char **) c->privdata)[1]);
      free((char *) c->privdata);
    }                           // or else it is a memory leak because we don't understand the privdata.
  } else {
    // define a new command, so malloc a new struct *c.
    c = (struct picolCmd *) malloc(sizeof(*c));
    c->next = Commands;
    c->name = strdup(name);
    Commands = c;
  }
  c->func = f;
  c->minArgc = minArgc;
  c->maxArgc = maxArgc;
  c->privdata = privdata;
  return PICOL_OK;
}

int picolArityErr(char *name) {
  char buf[BUF_SIZE];
  snprintf_s(buf, BUF_SIZE, "Wrong number of args for %s", name);
  picolSetResult(buf);
  return PICOL_ERR;
}

/* EVAL! */
int picolEval(const char *t, const char *where) {
#ifdef HEAP_CHECKS
  HeapCheck();
#endif
  struct picolParser p;
  int argc = 0, j;
  char **argv = NULL;
  char errbuf[BUF_SIZE];
  int retcode = PICOL_OK;
  picolSetResult("");
  picolInitParser(&p, t);
  while (1) {
    char *t;
    int tlen;
    int prevtype = p.type;
    picolGetToken(&p);
    if (p.type == PT_EOF)
      break;
    tlen = p.end - p.start + 1;
    if (tlen < 0)
      tlen = 0;
    t = (char *) malloc(tlen + 1);
    memcpy(t, p.start, tlen);
    t[tlen] = '\0';
    if (p.type == PT_VAR) {
      struct picolVar *v = picolGetVar(t);
      if (!v) {
        snprintf_s(errbuf, BUF_SIZE, "No such variable '%s'", t);
        free(t);
        picolSetResult(errbuf);
        retcode = PICOL_ERR;
        goto err;
      }
      free(t);
      t = strdup(v->val);
    } else if (p.type == PT_CMD) {
      retcode = picolEval(t, "[...]");
      free(t);
      if (retcode != PICOL_OK)
        goto err;
      t = strdup(Result);
    } else if (p.type == PT_ESC) {
      /* XXX: TODO: escape handling missing! */
    } else if (p.type == PT_SEP) {
      prevtype = p.type;
      free(t);
      continue;
    }
    /* We have a complete command + args. Call it! */
    if (p.type == PT_EOL) {
      struct picolCmd *c;
      free(t);
      prevtype = p.type;
      if (argc) {
        char **new_argv = NULL;
        if ((c = picolGetCommand(argv[0])) == NULL) {
          if (STRCMP(argv[0], "unknown")) {
            c = picolGetCommand("unknown");
            if (c) {
              new_argv = (char **) malloc((argc + 1) * sizeof(char *));
              argc++;
              // shift everything down one slot.
              for (int k = argc - 2; k >= 0; k--) {
                new_argv[k + 1] = argv[k];
              }
              // and shove "unknown" in front of them.
              new_argv[0] = (char *) "unknown";
              goto call;
            }
          }
          snprintf_s(errbuf, BUF_SIZE, "No such command '%s'", argv[0]);
          picolSetResult(errbuf);
          retcode = PICOL_ERR;
          goto err;
        }
      call:
        char **chosen_argv = new_argv ? new_argv : argv;
        if (c->minArgc && argc < (int) c->minArgc)
          retcode = picolArityErr(chosen_argv[0]);
        else if (c->minArgc && argc < (int) c->minArgc)
          retcode = picolArityErr(chosen_argv[0]);
        else
          retcode = c->func(argc, chosen_argv, c->privdata);
        free(new_argv);
        if (retcode != PICOL_OK)
          goto err;
      }
      /* Prepare for the next command */
      for (j = 0; j < argc; j++)
        free(argv[j]);
      free(argv);
      argv = NULL;
      argc = 0;
      continue;
    }
    /* We have a new token, append to the previous or as new arg? */
    if (prevtype == PT_SEP || prevtype == PT_EOL) {
      argv = (char **) realloc(argv, sizeof(char *) * (argc + 1));
      argv[argc] = t;
      argc++;
    } else {                    /* Interpolation */
      int oldlen = strlen(argv[argc - 1]), tlen = strlen(t);
      argv[argc - 1] = (char *) realloc(argv[argc - 1], oldlen + tlen + 1);
      memcpy(argv[argc - 1] + oldlen, t, tlen);
      argv[argc - 1][oldlen + tlen] = '\0';
      free(t);
    }
    prevtype = p.type;
  }
err:
  for (j = 0; j < argc; j++)
    free(argv[j]);
  free(argv);
  if (retcode == PICOL_ERR) {
    picolAppendResult("; in ");
    picolAppendResult(where);
  }
#ifdef HEAP_CHECKS
  HeapCheck();
#endif
  return retcode;
}

/* ACTUAL COMMANDS! */
//- eq a b -> z (string compare: a==b; returns 0 or 1)
//- ne a b -> z (string compare: a!=b; returns 0 or 1)
//- lt a b -> z (string compare: a<b; returns 0 or 1)
//- le a b -> z (string compare: a<=b; returns 0 or 1)
//- gt a b -> z (string compare: a>b; returns 0 or 1)
//- ge a b -> z (string compare: a>=b; returns 0 or 1)
int picolCommandStringRelOp(int argc, char **argv, void *pd) {
  char m1 = Up(argv[0][0]);
  char m2 = Up(argv[0][1]);
  int cmp = STRCMP(argv[1], argv[2]);
  int b;

  if (m1 == 'E' && m2 == 'Q')
    b = (cmp == 0);
  else if (m1 == 'N' && m2 == 'E')
    b = (cmp != 0);
  else if (m1 == 'L' && m2 == 'T')
    b = (cmp < 0);
  else if (m1 == 'L' && m2 == 'E')
    b = (cmp <= 0);
  else if (m1 == 'G' && m2 == 'T')
    b = (cmp > 0);
  else if (m1 == 'G' && m2 == 'E')
    b = (cmp >= 0);
  else {
    puthex('1', m1);
    puthex('2', m2);
    panic("WUT");
  }
  picolSetResult(b ? "1" : "0");
  return PICOL_OK;
}

//- + args... -> z (add integers; 0 if none)
//- * args... -> z (multiply integers; 1 if none)
//- - a b -> z (subtract: a-b)
//- / a b -> z (integer division: a/b)
//- % a b -> z (integer modulo: a%b)
//- == a b -> z (integer compare: returns 0 or 1)
//- != a b -> z (integer compare: returns 0 or 1)
//- < a b -> z (integer compare: returns 0 or 1)
//- <= a b -> z (integer compare: returns 0 or 1)
//- > a b -> z (integer compare: returns 0 or 1)
//- >= a b -> z (integer compare: returns 0 or 1)
//- bitand a b -> z (bitwise and: a&b)
//- bitor a b -> z (bitwise or: a|b)
//- bitxor a b -> z (bitwise xor: x^b)
//- << a b -> z (shift left: a<<b)
//- >> a b -> z (shift right, signed: a>>b)
//- >>> a b -> z (shift right, unsigned: a>>b)
int picolCommandMath(int argc, char **argv, void *pd) {
  char m1 = argv[0][0];
  char m2 = argv[0][1];
  char m3 = argv[0][2];
  char buf[8];
  int a, b, c;
  if (m1 == '+' || m1 == '*') {
    // + and * allow any number of args.
    c = (m1 == '+') ? 0 : 1;
    for (int j = 1; j < argc; j++) {
      b = atoi(argv[j]);
      c = (m1 == '+') ? c + b : c * b;
    }
  } else {
    // The rest apply to only 2 numbers.
    if (argc != 3)
      return picolArityErr(argv[0]);
    a = atoi(argv[1]);
    b = atoi(argv[2]);
    if (m1 == '-')
      c = a - b;
    else if (m1 == '/')
      c = a / b;
    else if (m1 == '%')
      c = a % b;
    else if (m1 == '>' && m2 == '\0')
      c = a > b;
    else if (m1 == '>' && m2 == '=')
      c = a >= b;
    else if (m1 == '>' && m2 == '>' & m3 == '\0')
      c = a >> b;
    else if (m1 == '>' && m2 == '>' & m3 == '>')
      c = (int) ((word) a >> b);
    else if (m1 == '<' && m2 == '\0')
      c = a < b;
    else if (m1 == '<' && m2 == '=')
      c = a <= b;
    else if (m1 == '<' && m2 == '<')
      c = a << b;
    else if (m1 == '=' && m2 == '=')
      c = a == b;
    else if (m1 == '!' && m2 == '=')
      c = a != b;
    else if (STRCMP(argv[0], "bitand") == 0)
      c = a & b;
    else if (STRCMP(argv[0], "bitor") == 0)
      c = a | b;
    else if (STRCMP(argv[0], "bitxor") == 0)
      c = a ^ b;
    else
      c = 0;                    /* I hate warnings */
  }
  snprintf_d(buf, 8, "%d", c);
  picolSetResult(buf);
  return PICOL_OK;
}

int NotFound() {
  picolSetResult("not found");
  return PICOL_ERR;
}

// array ?array_name? ?array_key? ?array_value? (no args: list array names. 1 arg: list array keys. 2 args: get value. 3 args: set value)
int picolCommandArray(int argc, char **argv, void *pd) {
  struct Buf buf;
  BufInit(&buf);

  switch (argc) {
  case 1:{
      // List arrays.
      for (struct picolArray * p = Arrays; p; p = p->next) {
        BufAppElemS(&buf, p->name);
      }
      BufFinish(&buf);
      picolMoveToResult(BufTake(&buf));
    }
    break;
  case 2:{
      // List keys of named array.
      struct picolArray *array = picolGetArray(argv[1]);
      if (!array) {
        return NotFound();
      }

      char *list = strdup("");
      for (struct picolVar * q = array->vars; q; q = q->next) {
        BufAppElemS(&buf, q->name);
      }
      BufFinish(&buf);
      picolMoveToResult(BufTake(&buf));
    }
    break;
  case 3:{
      struct picolArray *array = picolGetArray(argv[1]);
      if (!array) {
        return NotFound();
      }
      struct picolVar *var = picolGetVarFromRoot(array->vars, argv[2]);
      if (!var) {
        return NotFound();
      }
      picolSetResult(var->val);
    }
    break;
  case 4:{
      // Set variable.
      struct picolArray *array = picolGetArray(argv[1]);
      if (!array) {
        array = (struct picolArray *) malloc(sizeof *array);
        array->name = strdup(argv[1]);
        array->vars = NULL;
        array->next = Arrays;
        Arrays = array;
      }
      picolSetVarFromRoot(&array->vars, argv[2], argv[3]);
    }
    break;
  }
  BufDel(&buf);
  return PICOL_OK;
}

int SplitList(const char *s, int *argcP, const char ***argvP) {
  struct Buf dope;
  BufInit(&dope);

  while (*s) {
    while (*s && *s <= 32) {    // skip white
      s++;
    }
    if (!s)
      break;

    const char *end;
    int len = ElemLen(s, &end);
    const char *elem = ElemDecode(s);
    s = end;

    BufAppDope(&dope, elem);
  }
  *argvP = BufTakeDope(&dope, argcP);
  return PICOL_OK;
}

// error message (throws the error)
int picolCommandError(int argc, char **argv, void *pd) {
  picolSetResult(argv[1]);
  return PICOL_ERR;
}

// join str ?delim?  (if no delim, join on empty string)
int picolCommandJoin(int argc, char **argv, void *pd) {
  int c = 0;
  const char **v = NULL;
  int err = SplitList(argv[1], &c, &v);

  char delim;
  switch (argc) {
  case 2:
    delim = 0;                  // Join with empty string.
    break;

  case 3:
    delim = argv[2][0];         // Join with first char of 2nd arg.
    break;
  }

  struct Buf result;
  BufInit(&result);
  for (int j = 0; j < c; j++) {
    if (j && delim) {
      BufAppC(&result, delim);
    }
    BufAppS(&result, v[j], -1);
  }
  BufFinish(&result);

  FreeDope(c, v);
  picolMoveToResult(BufTake(&result));
  return PICOL_OK;
}

// split str ?delim?  (if no delim, split on whitespace into nonempty words)
int picolCommandSplit(int argc, char **argv, void *pd) {
  char delim;
  char *s = argv[1];

  switch (argc) {
  case 2:
    delim = 0;                  // Split on white space.
    break;

  case 3:
    delim = argv[2][0];         // Split on first char of 2nd arg.
    break;
  }

  byte final_delim = false;
  struct Buf list;
  BufInit(&list);
  while (*s) {
    struct Buf part;
    BufInit(&part);
    while (*s) {
      if (delim) {
        // Use specified delimiter.
        if (*s == delim) {
          final_delim = true;
          break;
        }
      } else {
        // Use any whitespace.
        if (*s <= 32)
          break;
      }

      // Not at a delimiter.
      BufAppC(&part, *s);
      s++;
      final_delim = false;
    }
    if (*s)
      s++;                      // past delim.

    // Finished a part.
    if (delim || part.n) {      // no empties if split on white.
      BufAppElemS(&list, BufFinish(&part));
    }
    BufDel(&part);
  }
  if (final_delim) {
    BufAppElemS(&list, "");
  }
  BufFinish(&list);
  picolMoveToResult(BufTake(&list));
  return PICOL_OK;
}

//- smatch pattern str -> 1 or 0 (`*` for any string, `?` for any char, `[...]` for char range)
//- sregexp pattern str -> position or -1 (Most basic patterns are supported. see re.h for documentation)
int picolCommandStringMatchOrRegexp(int argc, char **argv, void *pd) {
  // Always do case-independant matching.
  char *pattern = strdup_upper(argv[1]);
  char *s = strdup_upper(argv[2]);
  int z = (Up(argv[0][1]) == 'R') ? re_match(pattern, s) : Tcl_StringMatch(s, pattern);
  free(pattern);
  free(s);
  return ResultD(z);
}

//- set varname ?value? (if value not provided, returns value of variable)
int picolCommandSet(int argc, char **argv, void *pd) {
  if (argc == 2) {
    // with one argument, get var.
    struct picolVar *s = picolGetVar(argv[1]);
    if (!s) {
      picolSetResult("no such var");
      ResultS("no such var: %s", argv[1]);
      return PICOL_ERR;
    }
    picolSetResult(s->val);
    return PICOL_OK;
  }
  // with two arguments, set var.
  picolSetVar(argv[1], argv[2]);
  picolSetResult(argv[2]);
  return PICOL_OK;
}

//- read fd num_bytes -> list_of_numbers (numbers are byte values)
int picolCommand9Read(int argc, char **argv, void *pd) {
  int fd = atoi(argv[1]);
  int n = atoi(argv[2]);
  char *buf = malloc(n + 1);
  int bytes_read = 0;
  int e = Os9Read(fd, buf, n, &bytes_read);
  if (e)
    return Error(argv[0], e);
  picolMoveToResult(Explode(buf, bytes_read));
  free(buf);
  return PICOL_OK;
}

//- gets fd varname -> num_bytes_read (puts value read in varname)
int picolCommandGets(int argc, char **argv, void *pd) {
  int fd = atoi(argv[1]);
  char *varname = argv[2];
  char buf[BUF_SIZE + 1];
  int bytes_read = 0;
  bzero(buf, BUF_SIZE + 1);
  int e = Os9ReadLn(fd, buf, BUF_SIZE, &bytes_read);
  if (e == 211 /*EOF*/) {
    picolSetVar(varname, "");
    return ResultD(0);
  }
  if (e)
    return Error(argv[0], e);
  picolSetVar(varname, buf);
  return ResultD(bytes_read);
}

//- puts ?-nonewline? ?fd? str (write str to fd, default is stdout)
int picolCommandPuts(int argc, char **argv, void *pd) {
  char **orig_argv = argv;      // argv may increment.
  byte nonewline = false;
  // any dash argument must be -nonewline.
  if (argc > 2 && argv[1][0] == '-') {
    nonewline = true;
    argc--, argv++;
  }
  if (argc != 2 && argc != 3)
    return picolArityErr(orig_argv[0]);
  // defaults to path 1.
  int fd = (argc == 3) ? atoi(argv[1]) : 1;
  int unused;
  int e = Os9WritLn(fd, argv[argc - 1], strlen(argv[argc - 1]), &unused);
  if (e)
    return Error(orig_argv[0], e);
  if (!nonewline) {
    e = Os9WritLn(fd, "\r", 1, &unused);
    if (e)
      return Error(orig_argv[0], e);
  }
  return EmptyOrError(0, orig_argv);
}

//- if {cond} {body_if_true} ?else {body_if_else}?
int picolCommandIf(int argc, char **argv, void *pd) {
  int retcode;
  if (argc != 3 && argc != 5)
    return picolArityErr(argv[0]);
  if ((retcode = picolEval(argv[1], "cond of if")) != PICOL_OK)
    return retcode;
  if (atoi(Result))
    return picolEval(argv[2], "then of if");
  else if (argc == 5)
    return picolEval(argv[4], "else of if");
  return PICOL_OK;
}

#if 0
Not Working Yet.
//#- if {cond} {body_if_true} ?else {body_if_else}?
//#- if {cond} {body_if_true} elseif {cond2} {body_if_true2} ... ?else {body_if_else}?
int picolCommandIf(int argc, char **argv, void *pd) {
  char **orig_argv = argv;

  int retcode;
  puts("if...");
  puts(StaticD(argc));
  puts("...");
  for (int i = 0; i < argc; i++) {
    puts(StaticD(i));
    puts("=");
    puts(argv[i]);
  }
  puts("\r");

  while (true) {
    if (argc < 3)
      return picolArityErr(orig_argv[0]);

    if ((retcode = picolEval(argv[1], "cond of if")) != PICOL_OK)
      return retcode;

    if (atoi(Result)) {
      return picolEval(argv[2], "then of if");
    } else if (argc > 5 && strcaseeq(argv[3], "elseif")) {
      argc -= 3, argv += 3;
      continue;
    } else if (argc == 5 && strcaseeq(argv[3], "else")) {
      return picolEval(argv[4], "else of if");
    } else {
      return picolArityErr(orig_argv[0]);
    }
  }
  return PICOL_OK;
}
#endif

//- and {cond1} {cond2}... -> first_false_or_one (stop evaluating if one is false)
int picolCommandAnd(int argc, char **argv, void *pd) {
  int n = 1;
  for (int j = 1; j < argc; j++) {
    int e = picolEval(argv[j], "clause of and");
    if (e)
      return e;
    n = atoi(Result);
    if (!n)
      return ResultD(0);
  }
  return ResultD(n);
}

//- or {cond1} {cond2}... -> first_true_or_zero (stop evaluating if one is true)
int picolCommandOr(int argc, char **argv, void *pd) {
  for (int j = 1; j < argc; j++) {
    int e = picolEval(argv[j], "clause of or");
    if (e)
      return e;
    int n = atoi(Result);
    if (n)
      return ResultD(n);
  }
  return ResultD(0);
}

//- while {cond} {body} (cond evaluates to 0 for false, other int for true)
int picolCommandWhile(int argc, char **argv, void *pd) {
  while (1) {
    int retcode = picolEval(argv[1], "cond of while");
    if (retcode != PICOL_OK)
      return retcode;
    if (atoi(Result)) {
      if ((retcode = picolEval(argv[2], "body of while")) == PICOL_CONTINUE)
        continue;
      else if (retcode == PICOL_OK)
        continue;
      else if (retcode == PICOL_BREAK)
        return PICOL_OK;
      else
        return retcode;
    } else {
      return PICOL_OK;
    }
  }
}

int picolCommandRetCodes(int argc, char **argv, void *pd) {
  if (STRCMP(argv[0], "break") == 0)
    return PICOL_BREAK;
  else if (STRCMP(argv[0], "continue") == 0)
    return PICOL_CONTINUE;
  return PICOL_OK;
}

void picolDropCallFrame() {
  struct picolCallFrame *cf = Callframe;
  struct picolVar *v = cf->vars, *t;
  while (v) {
    t = v->next;
    free(v->name);
    free(v->val);
    free(v);
    v = t;
  }
  Callframe = cf->parent;
  free(cf);
}

int picolCommandCallProc(int argc, char **argv, void *pd) {
  char **pair = (char **) pd, *alist = pair[0], *body = pair[1];

  struct picolCallFrame *cf = (struct picolCallFrame *) malloc(sizeof(*cf));
  cf->vars = NULL;
  cf->parent = Callframe;
  Callframe = cf;

  // TODO: preprocess the alist.
  int c = 0;
  const char **v = NULL;
  int err = SplitList(alist, &c, &v);

  byte varargs = false;
  if (c && STRCMP(v[c - 1], "args") == 0) {
    varargs = true;
  }
  if ((!varargs && c != argc - 1) || (varargs && argc - 1 < c - 1)) {
    char errbuf[BUF_SIZE];
    snprintf_s(errbuf, BUF_SIZE, "Proc '%s' called with wrong num args", argv[0]);
    picolSetResult(errbuf);
    picolDropCallFrame();       /* remove the called proc callframe */
    FreeDope(c, v);
    return PICOL_ERR;
  }

  for (int i = 0; i < c - varargs; i++) {
    picolSetVar(v[i], argv[i + 1]);
  }

  if (varargs) {
    struct Buf rest;
    BufInit(&rest);
    for (int j = c; j < argc; j++) {
      BufAppElemS(&rest, argv[j]);
    }
    BufFinish(&rest);
    picolSetVar("args", rest.s);
    BufDel(&rest);
  }

  int errcode = picolEval(body, argv[0]);
  if (errcode == PICOL_RETURN)
    errcode = PICOL_OK;
  picolDropCallFrame();         /* remove the called proc callframe */
  FreeDope(c, v);
  return errcode;
}

//- proc name varlist body (defines a new proc)
int picolCommandProc(int argc, char **argv, void *pd) {
  char **procdata = (char **) malloc(sizeof(char *) * 2);
  procdata[0] = strdup(argv[2]);        /* arguments list */
  procdata[1] = strdup(argv[3]);        /* procedure body */
  return picolRegisterCommand(argv[1], 0, 0, picolCommandCallProc, procdata);
}

//- return ?value?   (returns from proc with given value or empty)
int picolCommandReturn(int argc, char **argv, void *pd) {
  picolSetResult((argc == 2) ? (const char *) argv[1] : "");
  return PICOL_RETURN;
}

//- info   (prints lots of info about interpreter state)
int picolCommandInfo(int argc, char **argv, void *pd) {
  puts(" procs: ");
  struct picolCmd *c;
  for (c = Commands; c; c = c->next) {
    if (c->func != picolCommandCallProc)
      continue;
    puts(c->name);
    puts(" ");
    // puts("   proc ");
    // puts(c->name);
    // puts(" {");
    // puts(((const char **) c->privdata)[0]);
    // puts("} {");
    // puts(((const char **) c->privdata)[1]);
    // puts("}\r");
  }
  puts("\r");

  puts(" commands: ");
  for (c = Commands; c; c = c->next) {
    if (c->func == picolCommandCallProc)
      continue;
    puts(c->name);
    puts(" ");
  }
  puts("\r");

  for (struct picolCallFrame * f = Callframe; f; f = f->parent) {
    puts(f->parent ? " frame: " : " globals: ");
    for (struct picolVar * v = f->vars; v; v = v->next) {
      puts(v->name);
      // puts("=");
      // puts(v->val);
      puts(" ");
    }
    puts("\r");
  }

  puts(" arrays:\r");
  for (struct picolArray * array = Arrays; array; array = array->next) {
    printf_s("   %s: ", array->name);
    for (struct picolVar * v = array->vars; v; v = v->next) {
      puts(v->name);
      // puts("=");
      // puts(v->val);
      puts(" ");
    }
    puts("\r");
  }

#if 1
  printf_d(" heap: used=%d", heap_here - heap_min);
  printf_d(" avail=%d", heap_max - heap_here);

  int cap = SMALLEST_BUCKET;
  for (byte b = 0; b < NBUCKETS; b++) {
    int num_in_bucket = 0;
    for (struct MallocHead * h = buck_freelist[b]; h; h = h->next) {
      num_in_bucket++;
    }
    printf_d(" [%d]", cap);
    if (buck_num_alloc[b]) {
      printf_d("a:%d-", buck_num_alloc[b]);
      printf_d("f:%d=", buck_num_free[b]);
      printf_d("u:%d;", buck_num_alloc[b] - buck_num_free[b]);
      printf_d("b:%d-", buck_num_brk[b]);
      printf_d("x:%d.", num_in_bucket);
    }
    cap += cap;
  }
  puts("\r");
#endif
  picolSetResult("");
  return PICOL_OK;
}

//- eval args... -> result (args are joined with spaces)
int picolCommandEval(int argc, char **argv, void *pd) {
  struct Buf buf;
  BufInit(&buf);
  // Join the args simply with spaces.
  for (int j = 1; j < argc; j++) {
    if (j)
      BufAppC(&buf, ' ');
    BufAppS(&buf, argv[j], -1);
  }
  BufFinish(&buf);
  int e = picolEval(BufPeek(&buf), "eval");
  BufDel(&buf);
  return e;
}

//- catch body ?varname? -> code (result value put in varname; code 0 is no error)
int picolCommandCatch(int argc, char **argv, void *pd) {
  char *body = argv[1];
  char *resultVar = (argc == 3) ? argv[2] : (char *) NULL;
  int e = picolEval(body, "catch");
  if (resultVar) {
    picolSetVar(resultVar, Result);
  }
  return ResultD(e);
}

//- explode str -> list_of_numbers (numbers are ascii values)
int picolCommandExplode(int argc, char **argv, void *pd) {
  picolMoveToResult(Explode(argv[1], -1));
  return PICOL_OK;
}

//- implode list_of_numbers -> str (numbers are ascii values)
int picolCommandImplode(int argc, char **argv, void *pd) {
  int c = 0;
  const char **v = NULL;
  int err = SplitList(argv[1], &c, &v);

  char *z = malloc(c + 1);
  int j;
  for (j = 0; j < c; j++) {
    z[j] = (char) atoi(v[j]);
  }
  z[j] = '\0';

  FreeDope(c, v);
  picolMoveToResult(z);
  return PICOL_OK;
}

//- peek addr
int picolCommandPeek(int argc, char **argv, void *pd) {
  byte* addr = (byte*) atoi(argv[1]);
  picolSetResult(StaticD(*addr));
  return PICOL_OK;
}

//- poke addr value
int picolCommandPoke(int argc, char **argv, void *pd) {
  byte* addr = (byte*) atoi(argv[1]);
  int val = atoi(argv[2]);
  *addr = (byte)val;
  picolSetResult("");
  return PICOL_OK;
}

//- incr varname ?value?
int picolCommandIncr(int argc, char **argv, void *pd) {
  struct picolVar *var = picolGetVar(argv[1]);
  if (!var) {
    picolSetVar(argv[1], "0");
    var = picolGetVar(argv[1]);
  }
  int z = (argc == 3) ? atoi(argv[2]) : 1;
  z += atoi(var->val);
  free(var->val);
  var->val = strdup(StaticD(z));
  picolSetResult(var->val);
  return PICOL_OK;
}

//- append varname ?items...?
int picolCommandAppend(int argc, char **argv, void *pd) {
  struct picolVar *var = picolGetVar(argv[1]);
  if (!var) {
    picolSetVar(argv[1], "");
    var = picolGetVar(argv[1]);
  }
  struct Buf buf;
  BufInitTake(&buf, var->val);
  for (int j = 2; j < argc; j++) {
    BufAppS(&buf, argv[j], -1);
  }
  BufFinish(&buf);
  var->val = (char *) BufTake(&buf);
  return PICOL_OK;
}

//- lappend varname ?items...?
int picolCommandListAppend(int argc, char **argv, void *pd) {
  struct picolVar *var = picolGetVar(argv[1]);
  if (!var) {
    picolSetVar(argv[1], "");
    var = picolGetVar(argv[1]);
  }
  struct Buf buf;
  BufInitTake(&buf, var->val);
  for (int j = 2; j < argc; j++) {
    BufAppElemS(&buf, argv[j]);
  }
  BufFinish(&buf);
  var->val = (char *) BufTake(&buf);
  return PICOL_OK;
}

//- llength list -> length
int picolCommandListLength(int argc, char **argv, void *pd) {
  int c = 0;
  const char **v = NULL;
  int err = SplitList(argv[1], &c, &v);
  FreeDope(c, v);
  return ResultD(c);
}

//- lindex list index -> item (return item at that index)
//- lrange list first last -> sublist (return sublit range from first to last inclusive)
int picolCommandListRangeOrIndex(int argc, char **argv, void *pd) {
  char *list = argv[1];
  int a = atoi(argv[2]);
  // lindex is like lrange with index twice.
  // TODO -- BUG? -- doesn't lrange return a list, while lindex does not?
  int b = (argc == 3) ? a : atoi(argv[3]);

  int c = 0;
  const char **v = NULL;
  int err = SplitList(list, &c, &v);

  struct Buf result;
  BufInit(&result);
  for (int j = 0; j < c; j++) {
    if (a <= j && j <= b)
      BufAppElemS(&result, v[j]);
  }
  FreeDope(c, v);

  BufFinish(&result);
  picolMoveToResult(BufTake(&result));
  return PICOL_OK;
}

//- slength str -> length
int picolCommandStringLength(int argc, char **argv, void *pd) {
  char *s = argv[1];
  int n = strlen(s);
  return ResultD(n);
}

//- sindex str index -> substr (return 1-char substring at that index)
//- srange str first last -> substr (return substring range from first to last inclusive)
int picolCommandStringRangeOrIndex(int argc, char **argv, void *pd) {
  char *s = argv[1];
  int n = strlen(s);
  int a = atoi(argv[2]);
  // sindex is like srange with index twice.
  int b = (argc == 3) ? a : atoi(argv[3]);
  if (a < 0)
    a = 0;
  if (b >= n)
    b = n - 1;
  struct Buf result;
  BufInit(&result);
  for (int j = a; j <= b; j++) {
    BufAppC(&result, s[j]);
  }
  BufFinish(&result);
  picolMoveToResult(BufTake(&result));
  return PICOL_OK;
}

//- supper str -> newstr (convert str to ASCII uppercase)
//- slower str -> newstr (convert str to ASCII lowercase)
int picolCommandStringUpperOrLower(int argc, char **argv, void *pd) {
  byte up = (Up(argv[0][1]) == 'U');
  char *s = argv[1];
  int n = strlen(s);
  char *z = malloc(n + 1);
  for (int j = 0; j <= n; j++) {
    z[j] = up ? Up(s[j]) : Down(s[j]);
  }
  picolMoveToResult(z);
  return PICOL_OK;
}

//- foreach var list body (assign each list item to var and execute body)
int picolCommandForEach(int argc, char **argv, void *pd) {
  char *var = argv[1];
  char *list = argv[2];
  char *body = argv[3];

  int c = 0;
  const char **v = NULL;
  int err = SplitList(list, &c, &v);
  for (int j = 0; j < c; j++) {
    picolSetVar(var, v[j]);
    int e = picolEval(body, "body of foreach");
    if (e == PICOL_CONTINUE)
      continue;
    if (e == PICOL_BREAK)
      break;
    if (e != PICOL_OK)
      return e;
  }

  FreeDope(c, v);
  picolSetResult("");
  return PICOL_OK;
}

const char *FormList(int argc, char **argv) {
  struct Buf buf;
  BufInit(&buf);
  for (int i = 0; i < argc; i++) {
    BufAppElemS(&buf, argv[i]);
  }
  BufFinish(&buf);
  return BufTake(&buf);
}

//- list ?items...?
int picolCommandList(int argc, char **argv, void *pd) {
  const char *s = FormList(argc - 1, argv + 1);
  picolSetResult(s);
  return PICOL_OK;
}

int Error(char *argv0, int err) {
  char buf[BUF_SIZE];
  snprintf_s(buf, BUF_SIZE, "%s: ERROR %d", argv0);
  char buf2[32];
  snprintf_d(buf2, 32, buf, err);
  picolSetResult(buf2);
  return PICOL_ERR;
}

int ResultD(int x) {
  char buf[32];
  snprintf_d(buf, 32, "%d", x);
  picolSetResult(buf);
  return PICOL_OK;
}

int ResultS(const char *msg, const char *x) {
  char buf[BUF_SIZE];
  snprintf_s(buf, 32, msg, x);
  picolSetResult(buf);
  return PICOL_OK;
}

//- exit ?status? (does not return.  0 is good status, 1..255 are bad)
int picolCommand9Exit(int argc, char **argv, void *pd) {
  exit((argc == 2) ? atoi(argv[1]) : 0);
  return PICOL_OK;
}

const char *AddCR(char *s) {
  int n = strlen(s);
  s = realloc(s, n + 2);
  s[n] = '\r';
  return (const char *) s;
}

const char *JoinWithSpaces(int argc, char **argv) {
  struct Buf buf;
  BufInit(&buf);
  for (int i = 0; i < argc; i++) {
    if (i)
      BufAppC(&buf, ' ');
    BufAppS(&buf, argv[i], -1);
  }
  BufFinish(&buf);
  return BufTake(&buf);
}

// TODO: unify 9chain & 9fork.
//- 9chain command args.... (does not return unless error)
int picolCommand9Chain(int argc, char **argv, void *pd) {
  char *program = argv[1];
  const char *params = /*FormList */ JoinWithSpaces(argc - 2, argv + 2);
  params = AddCR((char *) params);
  int e = Os9Chain(program, params, strlen(params), 0 /*lang_type */ ,
                   0 /*mem_size */ );
  // If returns, it is an error.
  return Error(argv[1], e);
}

// TODO: unify 9chain & 9fork.
//- 9fork ?-mX? command args.... -> child_id (option: -m8 for 8 ram blocks; -m8K for 8K ram)
int picolCommand9Fork(int argc, char **argv, void *pd) {
  int mem_size = 0;
  while (argc > 2 && Tcl_StringMatch(argv[1], "-m*")) {
    mem_size = atoi(argv[1] + 2);
    if (Up(argv[1][strlen(argv[1]) - 1]) == 'K')
      mem_size <<= 2;
    argc--;
    argv++;
  }
  if (argc < 2) {
    return picolArityErr(argv[0]);
  }
  char *program = argv[1];
  const char *params = /*FormList */ JoinWithSpaces(argc - 2, argv + 2);
  params = AddCR((char *) params);
  int child_id = 0;
  int e = Os9Fork(program, params, strlen(params), 0 /*lang_type */ ,
                  mem_size, &child_id);
  free((char *) params);
  return IntOrError(e, argv + 1, child_id);
}

//- 9filesize fd -> size (error if 64K or bigger)
int picolCommand9FileSize(int argc, char **argv, void *pd) {
  int d, x, u;
  int e = Os9GetStt(atoi(argv[1]), 2 /*SS.Size */ , &d, &x, &u);
  if (e)
    return Error(argv[0], e);
  if (x)
    return picolSetResult("toobig"), PICOL_ERR;
  return picolSetResult(StaticD(u)), PICOL_OK;
}

//- 9wait child_id_var exit_status_var (name two variables to receive results)
int picolCommand9Wait(int argc, char **argv, void *pd) {
  int child_id_and_exit_status = 0;
  int e = Os9Wait(&child_id_and_exit_status);
  if (e)
    return Error(argv[0], e);
  if (argc >= 2)
    picolSetVar(argv[1], StaticD(255 & (child_id_and_exit_status >> 8)));
  if (argc >= 3)
    picolSetVar(argv[2], StaticD(255 & (child_id_and_exit_status)));
  return PICOL_OK;
}

//- 9dup fd -> new_fd
int picolCommand9Dup(int argc, char **argv, void *pd) {
  int new_path = 0;
  int path = atoi(argv[1]);
  int e = Os9Dup(path, &new_path);
  return IntOrError(e, argv, new_path);
}

//- 9close fd
int picolCommand9Close(int argc, char **argv, void *pd) {
  int path = atoi(argv[1]);
  int e = Os9Close(path);
  return EmptyOrError(e, argv);
}

//- kill processid ?signal_code?
int picolCommand9Kill(int argc, char **argv, void *pd) {
  int victim = atoi(argv[1]);
  int signal = (argc < 3) ? 228 : atoi(argv[2]);
  int e = Os9Send(victim, signal);
  return EmptyOrError(e, argv);
}

//- sleep num_ticks
int picolCommand9Sleep(int argc, char **argv, void *pd) {
  int ticks = atoi(argv[1]);
  int e = Os9Sleep(ticks);
  return EmptyOrError(e, argv);
}

char SetHiBitOfLastChar(char *s) {
  int n = strlen(s);
  char z = s[n - 1];
  s[n - 1] |= 0x80;
  return z;
}

void RestoreLastChar(char *s, char c) {
  int n = strlen(s);
  s[n - 1] = c;
}

//- 9create filepath access_mode attrs -> fd (access_mode: 2=write 3=update)
int picolCommand9Create(int argc, char **argv, void *pd) {
  char *path = argv[1];
  char final = SetHiBitOfLastChar(path);
  int mode = atoi(argv[2]);
  int attrs = atoi(argv[3]);
  int fd = 0;

  int e = Os9Create(path, mode, attrs, &fd);
  RestoreLastChar(path, final);
  return IntOrError(e, argv, fd);
}

//- 9open filepath access_mode -> fd (access_mode: 1=read 2=write 3=update)
int picolCommand9Open(int argc, char **argv, void *pd) {
  char *path = argv[1];
  char final = SetHiBitOfLastChar(path);
  int mode = atoi(argv[2]);
  int fd = 0;

  int e = Os9Open(path, mode, &fd);
  RestoreLastChar(path, final);
  return IntOrError(e, argv, fd);
}

//- 9makdir filepath mode
//- 9chgdir filepath which_dir (which_dir: 1=working, 4=execute)
int picolCommand9MakOrChgDir(int argc, char **argv, void *pd) {
  char *path = argv[1];
  char final = SetHiBitOfLastChar(path);
  int mode = atoi(argv[2]);

  error (*f)(const char *, int);  // defines `f`.
  f = (Up(argv[0][1]) == 'M') ? Os9MakDir : Os9ChgDir;
  int e = f(path, mode);
  RestoreLastChar(path, final);
  return EmptyOrError(e, argv);
}

//- 9delete filepath
int picolCommand9Delete(int argc, char **argv, void *pd) {
  char *path = argv[1];
  char final = SetHiBitOfLastChar(path);

  int e = Os9Delete(path);
  RestoreLastChar(path, final);
  return EmptyOrError(e, argv);
}

//- source filepath (read and execute the script; 16K max}
int picolCommandSource(int argc, char **argv, void *pd) {
  // Open.

  char *path = argv[argc - 1];
  char final = SetHiBitOfLastChar(path);
  int fd = 0;
  int e = Os9Open(path, 1 /*read mode */ , &fd);
  RestoreLastChar(path, final);
  if (e)
    return Error(path, e);

  // Stat: FileSize

  int d, x, u;
  e = Os9GetStt(fd, 2 /*SS.Size */ , &d, &x, &u);
  if (e)
    return Error(path, e);
  if (x || u < 0 || u > 16 * 1024)      // cannot handle more than say 16K.
    return picolSetResult("toobig"), PICOL_ERR;

  // Read.

  char *buf = malloc(u + 1);
  bzero(buf, u + 1);
  int bytes_read = 0;
  e = Os9Read(fd, buf, u, &bytes_read);
  if (e)
    return Error(path, e);
  if (bytes_read != u)
    return Error(path, 0);

  // Close.
  e = Os9Close(fd);
  if (e)
    return Error(argv[0], e);

  e = picolEval(buf, path);
  free(buf);
  return e;
}

void picolRegisterCoreCommands() {
  const char *mathOps[] = {
    "+", "-", "*", "/", "%", ">", ">=", "<", "<=", "==", "!=",
    "bitand", "bitor", "bitxor", "<<", ">>", ">>>", NULL
  };
  for (const char **p = mathOps; *p; p++)
    picolRegisterCommand(*p, 0, 0, picolCommandMath, NULL);

  const char *strOps[] = { "eq", "ne", "lt", "le", "gt", "ge", NULL };
  for (const char **p = strOps; *p; p++)
    picolRegisterCommand(*p, 3, 3, picolCommandStringRelOp, NULL);

  picolRegisterCommand("set", 2, 3, picolCommandSet, NULL);
  picolRegisterCommand("puts", 0, 0, picolCommandPuts, NULL);
  picolRegisterCommand("gets", 3, 3, picolCommandGets, NULL);
  picolRegisterCommand("if", 0, 0, picolCommandIf, NULL);
  picolRegisterCommand("and", 0, 0, picolCommandAnd, NULL);
  picolRegisterCommand("or", 0, 0, picolCommandOr, NULL);
  picolRegisterCommand("while", 3, 3, picolCommandWhile, NULL);
  picolRegisterCommand("break", 1, 1, picolCommandRetCodes, NULL);
  picolRegisterCommand("continue", 1, 1, picolCommandRetCodes, NULL);
  picolRegisterCommand("proc", 4, 4, picolCommandProc, NULL);
  picolRegisterCommand("return", 1, 2, picolCommandReturn, NULL);
  picolRegisterCommand("info", 1, 1, picolCommandInfo, NULL);
  picolRegisterCommand("foreach", 4, 4, picolCommandForEach, NULL);
  picolRegisterCommand("eval", 0, 0, picolCommandEval, NULL);
  picolRegisterCommand("catch", 2, 3, picolCommandCatch, NULL);
  picolRegisterCommand("list", 0, 0, picolCommandList, NULL);
  picolRegisterCommand("explode", 2, 2, picolCommandExplode, NULL);
  picolRegisterCommand("implode", 2, 2, picolCommandImplode, NULL);
  picolRegisterCommand("peek", 2, 2, picolCommandPeek, NULL);
  picolRegisterCommand("poke", 3, 3, picolCommandPoke, NULL);
  picolRegisterCommand("incr", 2, 3, picolCommandIncr, NULL);
  picolRegisterCommand("append", 2, 0, picolCommandAppend, NULL);
  picolRegisterCommand("lappend", 2, 0, picolCommandListAppend, NULL);
  picolRegisterCommand("llength", 2, 2, picolCommandListLength, NULL);
  picolRegisterCommand("lindex", 3, 3, picolCommandListRangeOrIndex, NULL);
  picolRegisterCommand("lrange", 4, 4, picolCommandListRangeOrIndex, NULL);
  picolRegisterCommand("slength", 2, 2, picolCommandStringLength, NULL);
  picolRegisterCommand("sindex", 3, 3, picolCommandStringRangeOrIndex, NULL);
  picolRegisterCommand("srange", 4, 4, picolCommandStringRangeOrIndex, NULL);
  picolRegisterCommand("supper", 2, 2, picolCommandStringUpperOrLower, NULL);
  picolRegisterCommand("slower", 2, 2, picolCommandStringUpperOrLower, NULL);
  picolRegisterCommand("smatch", 3, 3, picolCommandStringMatchOrRegexp, NULL);
  picolRegisterCommand("sregexp", 3, 3, picolCommandStringMatchOrRegexp, NULL);
  picolRegisterCommand("array", 1, 4, picolCommandArray, NULL);
  picolRegisterCommand("split", 2, 3, picolCommandSplit, NULL);
  picolRegisterCommand("join", 2, 3, picolCommandJoin, NULL);
  picolRegisterCommand("exit", 1, 2, picolCommand9Exit, NULL);
  picolRegisterCommand("error", 2, 2, picolCommandError, NULL);
  picolRegisterCommand("source", 2, 2, picolCommandSource, NULL);
  picolRegisterCommand("kill", 2, 3, picolCommand9Kill, NULL);

  // kernel-level os9 commands:
  picolRegisterCommand("9chain", 2, 0, picolCommand9Chain, NULL);
  picolRegisterCommand("9fork", 0, 0, picolCommand9Fork, NULL);
  picolRegisterCommand("9wait", 1, 3, picolCommand9Wait, NULL);
  picolRegisterCommand("9filesize", 2, 2, picolCommand9FileSize, NULL);
  picolRegisterCommand("9dup", 2, 2, picolCommand9Dup, NULL);
  picolRegisterCommand("9close", 2, 2, picolCommand9Close, NULL);
  picolRegisterCommand("9sleep", 2, 2, picolCommand9Sleep, NULL);
  picolRegisterCommand("9chgdir", 3, 3, picolCommand9MakOrChgDir, NULL);
  picolRegisterCommand("9makdir", 3, 3, picolCommand9MakOrChgDir, NULL);
  picolRegisterCommand("9open", 3, 3, picolCommand9Open, NULL);
  picolRegisterCommand("9create", 4, 4, picolCommand9Create, NULL);
  picolRegisterCommand("9delete", 2, 2, picolCommand9Delete, NULL);
  picolRegisterCommand("9read", 3, 3, picolCommand9Read, NULL);
  //picolRegisterCommand("9write", 0, 0, picolCommand9Write, NULL);
  //picolRegisterCommand("9readln", 0, 0, picolCommand9ReadLn, NULL);
  //picolRegisterCommand("9writln", 0, 0, picolCommand9WritLn, NULL);

}

int main(int argc, char* argv[]) {
  picolInitInterp();
#ifdef HEAP_CHECKS
  HeapCheck();
#endif
  picolRegisterCoreCommands();
#ifdef HEAP_CHECKS
  HeapCheck();
#endif

  // A script can `source $rcfile` if it wants.
  picolSetVar("rcfile", "/dd/sys/nclrc.tcl");
#ifdef HEAP_CHECKS
  HeapCheck();
#endif

  // If command-line params, invoke a script.
  int e;
#if 0
  int param_size = param_max - param_min;
  char *params = malloc(param_size + 1);
  memcpy(params, (char *) param_min, param_size);
  params[param_size] = '\0';
  params[param_size - 1] = '\0';        // TODO: why is this needed?

  int argc;
  const char **argv;
  e = SplitList(params, &argc, &argv);
#ifdef HEAP_CHECKS
  HeapCheck();
#endif
  if (e) {
    puts("Cannot SplitList params");
    return e;
  }
  free(params);
#ifdef HEAP_CHECKS
  HeapCheck();
#endif
#else
  argc--; argv++;  // match how I did it before.
#endif

  if (argc) {

    picolSetVar("argv0", argv[0]);

    struct Buf list;
    BufInit(&list);
    for (int j = 1; j < argc; j++) {
      BufAppElemS(&list, argv[j]);
      BufFinish(&list);
    }
    BufFinish(&list);
    picolSetVar("argv", list.s);
    BufDel(&list);

    BufInit(&list);
    BufAppElemS(&list, "source");
    BufAppElemS(&list, argv[0]);
    BufFinish(&list);
    e = picolEval(list.s, "__main__");
    BufDel(&list);
    if (e) {
      printf_d("ERROR code %d: ", e);
      printf_s("%s\r", Result);
    }
    return e;
  } else {

    e = picolEval("if {catch {source $rcfile} rcval} {puts 2 \"Error sourcing $rcfile: $rcval\"}",
                  "__rc__");
    if (e)
      panic("RC");

    while (1) {
      puts(" >NCL> ");
      char line[111];
      bzero(line, sizeof line);
      int bytes_read;
      e = Os9ReadLn(0 /*path */ , line, 111, &bytes_read);
      if (e) {
        puts(" *EOF*\r");
        break;
      }
#ifdef TEAR_DOWN
      // If debugging teardown, line starting with `;` exits repl.
      if (line[0] == ';') {
        break;
      }
#endif
#if 0
      ReduceBigraphs(line);
#endif
      e = picolEval(line, "__repl__");
      if (e) {
        puts(" ERROR: ");
        if (e > 1) {
          printf_d("CODE=%d: ", e);
        }
        puts(Result);
        puts("\r");
      } else {
        if (Result[0] != '\0') {
          puts(Result);
          puts("\r");
        }
      }
    }
  }
#ifdef TEAR_DOWN
 #if 0
  FreeDope(argc, argv);
 #endif
  TearDown();
#endif
  return 0;
}
