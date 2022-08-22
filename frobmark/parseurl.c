#include "frobio/frobmark/parseurl.h"
#include "frobio/nyformat.h"
#include "frobio/ncl/buf.h"
#include "frobio/ncl/malloc.h"
#include "frobio/ncl/std.h"

static const char* ScanNonSlashName(
           const char* p, const char** name_out) {
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
        s = ScanNonSlashName(s, &url_out->hostport);
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
            s = ScanNonSlashName(s+2, &url_out->hostport);
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
    BufFinish(&path);
    url_out->path = BufTake(&path);
    return OKAY;
}

error JoinUrls(const Url* a, const Url* b, Url* out) {
    memset(out, 0, sizeof *out);
    assert(a->valid && b->valid);
    // assert(a->scheme && b->scheme);
    // assert(a->hostport && b->hostport);
    assert(a->path && b->path);

    if (b->scheme) {
        *out = *b;
        out->scheme = strdup(b->scheme);
        if (b->hostport) out->hostport = strdup(b->hostport);
        if (b->path) out->path = strdup(b->path);
    } else if (b->hostport) {
        *out = *b;
        out->scheme = strdup(a->scheme);
        if (b->hostport) out->hostport = strdup(b->hostport);
        if (b->path) out->path = strdup(b->path);
    } else if (b->path[0] == '/') {
        *out = *b;
        out->scheme = strdup(a->scheme);
        out->hostport = strdup(a->hostport);
        out->path = strdup(b->path);
    } else {
        *out = *b;
        out->scheme = strdup(a->scheme);
        out->hostport = strdup(a->hostport);
        char* s = malloc(strlen(a->path) + strlen(b->path) + 4);
        strcpy(s, a->path);
        int n = strlen(s);
        // Go backwards and delete the tail of s
        // until you find a '/'.
        for (int i = n-1; i>0; i--) {
            if (s[i]!='/') {
                s[i] = '\0';
            } else {
                break;
            }
        }
        // strcat(s, "/");
        strcat(s, b->path);
        out->path = s;
    }
    return OKAY;
}

char* UrlToStr(Url* a) {
    if (!a->valid) return strdup("URL(INVALID)");
    return StrFormat(
        "URL(%q,%q,%q)",
        a->scheme ? a->scheme : "-null-",
        a->hostport ? a->hostport : "-null-",
        a->path ? a->path : "-");
}

void DeleteUrl(Url* a) {
    if (a->scheme) free((void*)a->scheme);
    if (a->hostport) free((void*)a->hostport);
    if (a->path) free((void*)a->path);
    memset(a, 0, sizeof *a);
}
