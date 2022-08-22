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
};

struct file_fetcher {
    Fetcher fetcher;
    //
    Url url;
    FILE* fd;
};

static char buf[300];

byte* FileFetcher_Readline(struct fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    char* ee = fgets(buf, sizeof(buf)-1, ff->fd);
    if (ee) return (byte*) strdup(buf);
    return NULL;
}
void FileFetcher_Close(Fetcher* handle) {
    struct file_fetcher* ff = (struct file_fetcher*)handle;
    error e = fclose(ff->fd);
    assert(!e);
}

error FileFetcher_Open(const Url* url, struct file_fetcher* ff) {
    ff->fd = fopen(url->path, "r");    
    ny_eprintf("---- %s %lx ----\n", url->path, (long)ff->fd);
    if (!ff->fd) {
        return 252; // ERR(CANNOT_OPEN_FILE);
    }
    ff->fetcher.readline = FileFetcher_Readline;
    ff->fetcher.close = FileFetcher_Close;
    return OKAY;
}

Fetcher* FetcherFactory(const Url* url) {
    Fetcher* z = NULL;
    const char* sch = url->scheme;
    if (!sch) sch="file:";
    if (!strlen(sch)) sch="file:";

    #if 0
    if (!strcmp(sch "t:")) {
        struct t_fetcher* tf =
            (struct t_fetcher*)malloc(sizeof *tf);
        assert(false); // TODO
        z = (Fetcher*) tf;
    } else
    #endif

    if (!strcmp(sch, "file:") || !strcmp(sch, "")) {
        struct file_fetcher* ff =
            (struct file_fetcher*)malloc(sizeof *ff);
        error e = FileFetcher_Open(url, ff);
        if (e) {
            ny_eprintf("Cannot open %q: error %d\n",
                url->path, e);
            free(ff);
            return NULL;
        }
        z = (Fetcher*) ff;

    } else {
        return NULL;
    }
    // Take the URL.
    assert(z);
    z->url = *url;
    memset((void*)url, 0, sizeof(*url));
    return z;
}
