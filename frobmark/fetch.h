#ifndef _FROBIO_FROBMARK_FETCH_H_
#define _FROBIO_FROBMARK_FETCH_H_

#include "frobio/nytypes.h"
#include "frobio/frobmark/parseurl.h"

typedef struct fetcher {
    const char* debug_str;
    Url url;

    byte* (*readline)(struct fetcher* handle);
    void  (*close)(struct fetcher* handle);
} Fetcher;

Fetcher* FetcherFactory(const Url* url);

#endif // _FROBIO_FROBMARK_FETCH_H_
