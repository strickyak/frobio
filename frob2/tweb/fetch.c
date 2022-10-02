#include "frob2/tweb/fetch.h"
#include "frob2/tweb/parseurl.h"

struct t_fetcher {
    Fetcher fetcher;
    //
    // TODO
};

struct file_fetcher {
    Fetcher fetcher;
    //
    Url url;
    File* fd;
};

struct opener_registration {
    struct opener_registration* next;
    const char* scheme;
    FetcherOpener opener;
} *first_opener;

void InstallOpener(const char* scheme,
                    FetcherOpener opener) {
  struct opener_registration* p =
      (struct opener_registration*)Malloc(sizeof *p);
  p->scheme = scheme;
  p->opener = opener;
  p->next = first_opener;
  first_opener = p;
}

static char readline_buf[300];

mstring FileFetcher_Readline(struct fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    FGets(readline_buf, sizeof(readline_buf)-1, ff->fd);
    if (ErrNo) return NULL;
    return strdup(readline_buf);
}
void FileFetcher_Close(Fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    FClose(ff->fd);
    Assert(!ErrNo);
    DeleteUrl(&ff->fetcher.url);
}

Fetcher* FileFetcher_Open(const Url* url) {
    struct file_fetcher* ff =
                (struct file_fetcher*)Malloc(sizeof *ff);
    memset(ff, 0, sizeof *ff);
    ff->fd = FOpen(url->path, "r");    
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
                LogFatal("Cannot open %q\n", url->path);
            }
            break;
        }
    }

    if (fetcher) CopyUrl(&fetcher->url, url);
    return fetcher;
}
