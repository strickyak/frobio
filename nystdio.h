#ifndef _FROBIO_FSTDIO_H_
#define _FROBIO_FSTDIO_H_

#include "frobio/nytypes.h"

#ifndef unix

#ifndef EOF
#define EOF (-1)
#endif

// There may be multiple implementatios of stdio laying around.
// Cmoc includes some.  We prefix with ny_ to distinguish ours.
typedef struct ny_file { int fd; } NY_FILE;

// Fopen mode can be "r" or "w".
FILE* ny_fopen(const char* pathname, const char* mode);
char *ny_fgets(char *buf, int size, NY_FILE *f);
int ny_fputs(const char *s, NY_FILE *f);
int ny_fclose(NY_FILE *f);
void ny_perror(const char* s);

extern int ny_errno;
extern NY_FILE ny_stdin;
extern NY_FILE ny_stdout;
extern NY_FILE ny_stderr;

// Usually we want to use the standard names.
#ifndef DONT_RENAME_FOPEN
#define FILE NY_FILE
#define fopen ny_fopen
#define fgets ny_fgets
#define fclose ny_fclose
#define perror ny_perror
#define stdin ny_stdin
#define stdout ny_stdout
#define stderr ny_stderr
#endif

#endif // !unix

#endif //  _FROBIO_FSTDIO_H_
