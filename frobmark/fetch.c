#include "frobio/frobmark/fetch.h"
#include "frobio/frobmark/parseurl.h"
#include "frobio/nystdio.h"
#include "frobio/nyformat.h"
#include "frobio/ncl/buf.h"
#include "frobio/ncl/malloc.h"
#include "frobio/ncl/std.h"

struct t_fetcher {
    Fetcher fetcher;
    //
    // TODO
};

struct file_fetcher {
    Fetcher fetcher;
    //
    Url url;
    FILE* fd;
};

struct opener_registration {
    struct opener_registration* next;
    const char* scheme;
    FetcherOpener opener;
} *first_opener;

void InstallOpener(const char* scheme,
                    FetcherOpener opener) {
  struct opener_registration* p =
      (struct opener_registration*)malloc(sizeof *p);
  p->scheme = scheme;
  p->opener = opener;
  p->next = first_opener;
  first_opener = p;
}

static char readline_buf[300];

mstring FileFetcher_Readline(struct fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    char* ee = fgets(readline_buf, sizeof(readline_buf)-1, ff->fd);
    if (ee) return strdup(readline_buf);
    return NULL;
}
void FileFetcher_Close(Fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    error e = fclose(ff->fd);
    assert(!e);
}

Fetcher* FileFetcher_Open(const Url* url) {
    struct file_fetcher* ff =
                (struct file_fetcher*)malloc(sizeof *ff);
    memset(ff, 0, sizeof *ff);
    ff->fd = fopen(url->path, "r");    
    ny_eprintf("---- %s %lx ----\n", url->path, (long)(word)ff->fd);
    if (!ff->fd) {
        return NULL;
    }
    ff->fetcher.readline = FileFetcher_Readline;
    ff->fetcher.close = FileFetcher_Close;
    return (Fetcher*)ff;
}

Fetcher* FetcherFactory(const Url* url) {
    Fetcher* fetcher = NULL;
    const char* sch = url->scheme;
    if (!sch) sch="file:";
    if (!strlen(sch)) sch="file:";

    struct opener_registration *p;
    for (p=first_opener; p; p=p->next) {
        if (!strcasecmp(sch, p->scheme)) {
            fetcher = p->opener(url);
            if (!fetcher) {
                ny_eprintf("Cannot open %q\n", url->path);
            }
            break;
        }
    }

    if (fetcher) CopyUrl(&fetcher->url, url);
    return fetcher;
}
