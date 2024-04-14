#ifndef _FROBIO_FROBMARK_PARSEURL_H_
#define _FROBIO_FROBMARK_PARSEURL_H_

// Types of URLs we grok:
//   * Scheme, hostport, and path.
//   * Host and path.
//   * Absolute path.
//   * Relative path. (relative to the directory
//                     of the .fm document
//                     in which it appears)
// "hostport" may have ":port" appended.

// Special URLs:
//   * Directory path:  If path ends with "/",
//          we append "index.fm".
//   * Root path:  If path is just "/",
//          this is just a special case of the
//          directory path, and we use "/index.fm".
//   * Raw Directory Listing:  If path ends with "/*",
//          we show all items in that directory,
//          even those not exposed in "index.fm".
//   * Directory Listing Fallback:  If "index.fm"
//          is not available, we show the raw
//          directory listing.

#include "frob2/froblib.h"

typedef struct url {
    bool valid;
    const char* scheme;
    const char* hostport;
    const char* path;
} Url;

errnum ParseUrl(const char* s, Url* url_out);
errnum JoinUrls(const Url* a, const Url* b, Url* out);
char* UrlToStr(Url* a);
void DeleteUrl(Url* a);
void CopyUrl(Url* dest, const Url* src);

#endif // _FROBIO_FROBMARK_PARSEURL_H_
