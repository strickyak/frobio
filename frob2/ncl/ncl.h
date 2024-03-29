// ncl:
//    NitrOS Command Language (similar to Tcl)
//    based on Picol, modified and enhanced for cmoc for NitrOS9/OS9
//        by Henry Strickland (github.com/strickyak).
//    BSD licensed.
//
// Was Picol:
//    Tcl in ~ 500 lines of code by Salvatore antirez Sanfilippo.
//    BSD licensed.

// In order to get all code into a single assembly listing for the module,
// we include these C files instead of using the linker to link the usual stuff.

#ifndef _FROB2_NCL_NCL_H_
#define _FROB2_NCL_NCL_H_

#include "frob2/froblib.h"
#include "frob2/frobos9.h"
#include "frob2/froblib/malloc.h"

#include "frob2/regexp/re.h"
#include "frob2/match/util.h"

#include <stdarg.h>

#define free Free
#define malloc Malloc
#define realloc ReAlloc
#define snprintf_s SnPrintf
#define printf_s Printf
#define printf_d Printf

#undef HEAP_CHECKS             // Check heap integregity at key points.
#define TEAR_DOWN               // Free all memory structures at the end, to check for leak.  Use `;` to exit repl.

#define STRCMP strcmp // used to be strcasecmp
#define BZERO(A,B) memset((A), 0, (B))
#define PRINT Os9_print

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
extern struct picolCallFrame *Callframe;
extern struct picolCmd *Commands;
extern struct picolArray *Arrays;
extern char *Result;

#define BUF_SIZE 200            /* instead of 1024 */

extern int ErrorNum(char *argv0, int err);
extern int ResultD(int x);
extern int ResultFormat(const char *msg, ...);
extern int NotFound();

int picolCommandCallProc(int argc, char **argv, void *pd);
const char *JoinWithSpaces(int argc, char **argv);
const char *AddCR(char *s);
int picolArityErr(char *name);
int IntOrErrorNum(int e, char **argv, int x);
void picolSetResult(const char *s);
int picolSetVar(const char *name, const char *val);
int EmptyOrErrorNum(int e, char **argv);
int picolEval(const char *t, const char *where);
const char* StaticFormatSignedInt(int x);

#endif // _FROB2_NCL_NCL_H_
