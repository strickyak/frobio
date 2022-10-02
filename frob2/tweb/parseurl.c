#include "frob2/froblib.h"

#include "frob2/tweb/parseurl.h"

static const char* ScanNonSlashName(
           const char* p, const char** name_out) {
    Buf buf;
    BufInit(&buf);
    while (*p > ' ' && *p <= '~' && *p != '/') {
        BufAppC(&buf, *p);
        p++;
    }
    *name_out = BufFinish(&buf);
    return p;   
}

errnum ParseUrl(const char* ascii, Url* url_out) {
    Assert(url_out);
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
        // int n = strlen(front); // TODO what was this for?
        if (s[-1]==':' && s[0]=='/' && s[1]=='/') {
            // Syntactically, it's a scheme.
            url_out->scheme = front;
            s = ScanNonSlashName(s+2, &url_out->hostport);
            BufAppC(&path, '/'); // path will be absolute.
        } else {
            Free((char*)front);
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

errnum JoinUrls(const Url* a, const Url* b, Url* out) {
    memset(out, 0, sizeof *out);
    Assert(a->valid && b->valid);
    // Assert(a->scheme && b->scheme);
    // Assert(a->hostport && b->hostport);
    Assert(a->path && b->path);

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
        char* s = Malloc(strlen(a->path) + strlen(b->path) + 4);
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
        // StrCat(s, "/");
        StrCat((byte*)s, (byte*)b->path);
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
    if (a->scheme) Free((void*)a->scheme);
    if (a->hostport) Free((void*)a->hostport);
    if (a->path) Free((void*)a->path);
    memset(a, 0, sizeof *a);
}

void CopyUrl(Url* dest, const Url* src) {
    memcpy((void*)dest, (const void*)src, sizeof *src);
    if (src->scheme) dest->scheme = strdup(src->scheme);
    if (src->hostport) dest->hostport = strdup(src->hostport);
    if (src->path) dest->path = strdup(src->path);
}
