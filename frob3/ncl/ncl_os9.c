// ncl:
//    NitrOS Command Language (similar to Tcl)
//    based on Picol, modified and enhanced for cmoc for NitrOS9/OS9
//        by Henry Strickland (Strick Yak).
//    BSD licensed.
//
// Was Picol:
//    Tcl in ~ 500 lines of code by Salvatore antirez Sanfilippo.
//    BSD licensed.

#include "frob3/ncl/ncl.h"

// TODO: unify 9chain & 9fork.
//- 9chain command args.... (does not return unless error)
int picolCommand9Chain(int argc, char **argv, void *pd) {
  char *program = argv[1];
  const char *params = /*FormList */ JoinWithSpaces(argc - 2, argv + 2);
  params = AddCR((char *) params);
  int e = Os9Chain(program, params, strlen(params), 0 /*lang_type */ ,
                   0 /*mem_size */ );
  // If returns, it is an error.
  return ErrorNum(argv[1], e);
}

// TODO: unify 9chain & 9fork.
//- 9fork ?-mX? command args.... -> child_id (option: -m8 for 8 ram blocks; -m8K for 8K ram)
int picolCommand9Fork(int argc, char **argv, void *pd) {
  int mem_size = 0;
  while (argc > 2 && Tcl_StringMatch(argv[1], "-m*")) {
    mem_size = atoi(argv[1] + 2);
    if (CharUp(argv[1][strlen(argv[1]) - 1]) == 'K')
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
  return IntOrErrorNum(e, argv + 1, child_id);
}

//- 9filesize fd -> size (error if 64K or bigger)
int picolCommand9FileSize(int argc, char **argv, void *pd) {
  int d, x, u;
  int e = Os9GetStt(atoi(argv[1]), 2 /*SS.Size */ , &d, &x, &u);
  if (e)
    return ErrorNum(argv[0], e);
  if (x)
    return picolSetResult("toobig"), PICOL_ERR;
  return ResultD(u);
}

//- 9wait child_id_var exit_status_var (name two variables to receive results)
int picolCommand9Wait(int argc, char **argv, void *pd) {
  int child_id_and_exit_status = 0;
  int e = Os9Wait(&child_id_and_exit_status);
  if (e)
    return ErrorNum(argv[0], e);
  if (argc >= 2)
    picolSetVar(argv[1], StaticFormatSignedInt(255 & (child_id_and_exit_status >> 8)));
  if (argc >= 3)
    picolSetVar(argv[2], StaticFormatSignedInt(255 & (child_id_and_exit_status)));
  return PICOL_OK;
}

//- 9dup fd -> new_fd
int picolCommand9Dup(int argc, char **argv, void *pd) {
  int new_path = 0;
  int path = atoi(argv[1]);
  int e = Os9Dup(path, &new_path);
  return IntOrErrorNum(e, argv, new_path);
}

//- 9close fd
int picolCommand9Close(int argc, char **argv, void *pd) {
  int path = atoi(argv[1]);
  int e = Os9Close(path);
  return EmptyOrErrorNum(e, argv);
}

//- kill processid ?signal_code?
int picolCommand9Kill(int argc, char **argv, void *pd) {
  int victim = atoi(argv[1]);
  int signal = (argc < 3) ? 228 : atoi(argv[2]);
  int e = Os9Send(victim, signal);
  return EmptyOrErrorNum(e, argv);
}

//- sleep num_ticks
int picolCommand9Sleep(int argc, char **argv, void *pd) {
  int ticks = atoi(argv[1]);
  int e = Os9Sleep(ticks);
  return EmptyOrErrorNum(e, argv);
}

//- 9create filepath access_mode attrs -> fd (access_mode: 2=write 3=update)
int picolCommand9Create(int argc, char **argv, void *pd) {
  char *path = argv[1];
  int mode = atoi(argv[2]);
  int attrs = atoi(argv[3]);
  int fd = 0;

  int e = Os9Create(mode, attrs, path, &fd);
  return IntOrErrorNum(e, argv, fd);
}

//- 9open filepath access_mode -> fd (access_mode: 1=read 2=write 3=update)
int picolCommand9Open(int argc, char **argv, void *pd) {
  char *path = argv[1];
  int mode = atoi(argv[2]);
  int fd = 0;

  int e = Os9Open(mode, path, &fd);
  return IntOrErrorNum(e, argv, fd);
}

//- 9makdir filepath mode
//- 9chgdir filepath which_dir (which_dir: 1=working, 4=execute)
int picolCommand9MakOrChgDir(int argc, char **argv, void *pd) {
  char *path = argv[1];
  int mode = atoi(argv[2]);

  auto errnum (*f)(const char *, int);  // defines `f`.
  f = (CharUp(argv[0][1]) == 'M') ? Os9MakDir : Os9ChgDir;
  int e = f(path, mode);
  return EmptyOrErrorNum(e, argv);
}

//- 9delete filepath
int picolCommand9Delete(int argc, char **argv, void *pd) {
  char *path = argv[1];

  int e = Os9Delete(path);
  return EmptyOrErrorNum(e, argv);
}

//- source filepath (read and execute the script; 16K max}
int picolCommandSource(int argc, char **argv, void *pd) {
  // Open.

  char *path = argv[argc - 1];
  int fd = 0;
  int e = Os9Open(path, 1 /*read mode */ , &fd);
  if (e)
    return ErrorNum(path, e);

  // Stat: FileSize

  int d, x, u;
  e = Os9GetStt(fd, 2 /*SS.Size */ , &d, &x, &u);
  if (e)
    return ErrorNum(path, e);
  if (x || u < 0 || u > 16 * 1024)      // cannot handle more than say 16K.
    return picolSetResult("toobig"), PICOL_ERR;

  // Read.

  char *buf = (char*)malloc(u + 1);
  BZERO(buf, u + 1);
  int bytes_read = 0;
  e = Os9Read(fd, buf, u, &bytes_read);
  if (e)
    return ErrorNum(path, e);
  if (bytes_read != u)
    return ErrorNum(path, 0);

  // Close.
  e = Os9Close(fd);
  if (e)
    return ErrorNum(argv[0], e);

  e = picolEval(buf, path);
  free(buf);
  return e;
}

