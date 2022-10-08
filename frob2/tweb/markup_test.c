#include "frob2/tweb/markup.h"
#include "frob2/frobos9.h"

mstring fizzbuzz(word x) {
    if (false && x%100 == 0) {
        return strdup( "wifjwjfiejwofwijfiowejfoiewjfiewjfoiewjfewfjoewjffjeabcdefghijklmnopq");
    } else if (false && x%15 == 0) {
        return strdup( "FIZZBUZZ");
    } else if (false && x%5 == 0) {
        return strdup( "buzz");
    } else if (false && x%3 == 0) {
        return strdup( "fizz");
    } else {
        return StrFormat("%d", x);
    }
}

struct fizz_fetcher {
    Fetcher super;
    word count;
};

mstring FizzFetcherReadLine(struct fetcher* handle) {
    struct fizz_fetcher* ff = (struct fizz_fetcher*)handle;
    ff->count++;
    if (ff->count > 1000) return NULL;
    return fizzbuzz(ff->count);
}
void FizzFetcherClose(struct fetcher* handle) {
    struct fizz_fetcher* ff = (struct fizz_fetcher*)handle;
    DeleteUrl(&ff->super.url);
    if(ff->super.debug_str) Free((void*)ff->super.debug_str);
    memset(handle, 0, sizeof *handle);
}

Fetcher* FizzFetcher_Open(const Url* url) {
    struct fizz_fetcher* ff = (struct fizz_fetcher*) Malloc(sizeof *ff);
    memset((void*)ff, 0, sizeof *ff);
    ff->super.readline = FizzFetcherReadLine;
    ff->super.close = FizzFetcherClose;
    return (Fetcher*)ff;
}

errnum TestPrintLine(byte* buf) {
    printf("%3ld {%s}\n", (long)strlen((const char*)buf), buf);
    return OKAY;
}

Rendering rend_test; 

int main() {
    InstallOpener("fizz:", FizzFetcher_Open);
    Url url;
    memset((void*)&url, 0, sizeof url);
    errnum e = ParseUrl("fizz://buzz/fizz", &url);
    Assert(!e);

    for (word page = 1; page <= 10; page++) {
        Printf("# -----  PAGE %d  --------------------------------------\n", page);
        rend_test.fetcher = FetcherFactory(&url);

        rend_test.width = 60;
        rend_test.height = 10;
        rend_test.print_line = TestPrintLine;
        rend_test.prompt_and_input= NULL;

        rend_test.page = page;
        FmRender(&rend_test);

        rend_test.fetcher->close(rend_test.fetcher);
        Free((void*)rend_test.fetcher);
        rend_test.fetcher = 0;
    }
    Printf("# ---- EXIT ----------------\n");
    DeleteUrl(&url);
    
    Printf("\nDONE (markup_test)\n");
    GomarHyperExit(0);
    return 0;
}
