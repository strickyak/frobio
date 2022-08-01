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
NY_FILE* ny_fopen(const char* pathname, const char* mode);
char *ny_fgets(char *buf, int size, NY_FILE *f);
int ny_fputs(const char *str, NY_FILE *f);
int ny_fclose(NY_FILE *f);
void ny_perror(const char* str);

extern int ny_errno;
extern NY_FILE ny_stdin_file;
extern NY_FILE ny_stdout_file;
extern NY_FILE ny_stderr_file;
#define ny_stdin (&ny_stdin_file)
#define ny_stdout (&ny_stdout_file)
#define ny_stderr (&ny_stderr_file)

// Usually we want to use the standard names.
#ifndef DONT_RENAME_NY_STDIO
#define FILE NY_FILE
#define fopen ny_fopen
#define fgets ny_fgets
#define fputs ny_fputs
#define fclose ny_fclose
#define perror ny_perror
#define errno ny_errno
#define stdin ny_stdin
#define stdout ny_stdout
#define stderr ny_stderr
#endif

#endif // !unix

#endif //  _FROBIO_FSTDIO_H_
