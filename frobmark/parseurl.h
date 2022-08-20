#ifndef _FROBIO_FROBMARK_PARSEURL_H_
#define _FROBIO_FROBMARK_PARSEURL_H_

#include "frobio/nytypes.h"

typedef struct url {
    bool valid;
    const char* scheme;
    const char* host;
    const char* path;
} Url;

typedef struct fetcher {
    void* (*open)(Url*);  // returns handle
    byte* (*readline)(void* handle);
    void  (*close)();
} Fetcher;

error ParseUrl(const char* s, Url* url_out);




#endif // _FROBIO_FROBMARK_PARSEURL_H_
