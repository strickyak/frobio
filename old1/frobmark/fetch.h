#ifndef _FROBIO_FROBMARK_FETCH_H_
#define _FROBIO_FROBMARK_FETCH_H_

#include "frobio/nytypes.h"
#include "frobio/frobmark/parseurl.h"

typedef struct fetcher {
    const char* debug_str;
    Url url;

    mstring (*readline)(struct fetcher* handle);
    void  (*close)(struct fetcher* handle);
} Fetcher;

Fetcher* FetcherFactory(const Url* url);

typedef Fetcher* (*FetcherOpener)(const Url* url);
Fetcher* FileFetcher_Open(const Url* url);

void InstallOpener(const char* scheme,
                   FetcherOpener opener);

#endif // _FROBIO_FROBMARK_FETCH_H_
