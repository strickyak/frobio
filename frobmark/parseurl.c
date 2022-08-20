#include "frobio/frobmark/parseurl.h"
#include "frobio/ncl/buf.h"
#include "frobio/ncl/malloc.h"

static const char* ScanNonSlashName(const char* p, const char** name_out) {
    Buf b;
    BufInit(&b);
    while (*p > ' ' && *p <= '~' && *p != '/') {
        BufAppC(&b, *p);
        p++;
    }
    *name_out = BufTake(&b);
    return p;   
}

error ParseUrl(const char* ascii, Url* url_out) {
    assert(url_out);
    memset(url_out, 0, sizeof *url_out);
    Buf path;
    BufInit(&path);

    const char*s = ascii;
    if (*s=='/' && s[1]=='/') {
        // Case: two or more slashes.
        while (*s=='/') s++;  // Skip slashes.
        s = ScanNonSlashName(s, &url_out->host);
        BufAppC(&path, '/'); // path will be absolute.
        while (*s=='/') s++;  // Skip slash(es).
    } else if (*s=='/') {
        // Case: Just one slash.
        BufAppC(&path, '/'); // path will be absolute.
    } else if (*s) {
        // Case: no slash.
        // Could be scheme, if ends in ':' and followed by "//".
        const char* front = NULL;
        s = ScanNonSlashName(s, &front);
        int n = strlen(front);
        if (s[-1]==':' && s[0]=='/' && s[1]=='/') {
            // Syntactically, it's a scheme.
            url_out->scheme = front;
            s = ScanNonSlashName(s+2, &url_out->host);
            BufAppC(&path, '/'); // path will be absolute.
        } else {
            free((char*)front);
            s = ascii;  // back up to start.
        }
    } else {
        return 255;
    }

    url_out->valid = true;
    while (*s == '/') s++;  // skip slashes before path.

    while (*s) {
        BufAppC(&path, *s);
        s++;
    }
    url_out->path = BufTake(&path);
    return OKAY;
}
