#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"
#include "frobio/nyformat.h"
#include "frobio/ncl/malloc.h"
#include "frobio/ncl/std.h"

mstring fizzbuzz(word x) {
    if (x%100 == 0) {
        return strdup( "wifjwjfiejwofwijfiowejfoiewjfiewjfoiewjfewfjoewjffjeabcdefghijklmnopq");
    } else if (x%15 == 0) {
        return strdup( "FIZZBUZZ");
    } else if (x%5 == 0) {
        return strdup( "buzz");
    } else if (x%3 == 0) {
        return strdup( "fizz");
    } else {
        return StrFormat("%d", x);
    }
}

/* typedef struct fetcher {
    const char* debug_str;
    Url url;

    byte* (*readline)(struct fetcher* handle);
    void  (*close)(struct fetcher* handle);
} Fetcher; */

struct fizz_fetcher {
    Fetcher fetcher;
    word count;
};

mstring FizzFetcherReadLine(struct fetcher* handle) {
    struct fizz_fetcher* ff = (struct fizz_fetcher*)handle;
    if (ff->count == 200) return NULL;
    ff->count++;
    return fizzbuzz(ff->count);
}
void FizzFetcherClose(struct fetcher* handle) {
}

Fetcher* FizzFetcher_Open(const Url* url) {
    struct fizz_fetcher* ff = (struct fizz_fetcher*) malloc(sizeof *ff);
    memset((void*)ff, 0, sizeof *ff);
    ff->fetcher.readline = FizzFetcherReadLine;
    ff->fetcher.close = FizzFetcherClose;
    return (Fetcher*)ff;
}

error TestPrintLine(byte* buf) {
    printf("\n<<< %3ld {%s} >>>\n", (long)strlen((const char*)buf), buf);
    return OKAY;
}

Rendering render_me; 

int main() {
    InstallOpener("fizz:", FizzFetcher_Open);
    Url url;
    memset((void*)&url, 0, sizeof url);
    error e = ParseUrl("fizz://buzz/fizz", &url);
    assert(!e);

    render_me.width = 60;
    render_me.height = 10;
    render_me.fetcher = FetcherFactory(&url);
    render_me.print_line = TestPrintLine;
    render_me.prompt_and_input= NULL;
    printf("\n####---------------------------------------------\n");

    for (word page = 0; page < 12; page++) {
        render_me.page = page;
        FmRender(&render_me);
        printf("\n####---------------------------------------------\n");
    }
    DeleteUrl(&url);
    printf("\nDone (markup_test)\n");
    
    return 0;
}
