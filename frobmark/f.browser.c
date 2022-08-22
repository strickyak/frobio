#include "frobio/nytypes.h"
#include "frobio/nyformat.h"
#include "frobio/frobmark/fetch.h"
#include "frobio/frobmark/parseurl.h"
#include "frobio/frobmark/markup.h"

error PrintLine(byte* b) {
    ny_eprintf("PRINT |||%s|||\n", b);
    return OKAY;
}

error PromptAndInput(byte* buf, word max) {
    ny_eprintf("  >>?>> ");
    //# fgets(buf, max, stdin);
    return 255;
}

static void Usage() {
    printf("Usage:  f.bro FirstuRL\n");
    // printf("  -a for render all and quit.\n");
    // printf("  -l for print links and quit.\n");
    exit(255);
}

int main(int argc, char *argv[]) {
    if (argc != 2) Usage();
    char* url_str = argv[1];

    for (byte pg=0; pg<10;pg++) {
        ny_eprintf("###### PAGE %d #######\n", pg);
        Url url;
        error e = ParseUrl(url_str, &url);
        if (e) {
            ny_eprintf("cannot ParseUrl %q: error %d\n", url_str, e);
            return 255;
        }
        ny_eprintf("Parsed URL: %s\n", UrlToStr(&url));

        Fetcher* f = FetcherFactory(&url);
        if (!f) {
            ny_eprintf("cannot ParseUrl %q: error %d\n", url_str, e);
            return 255;
        }

        Rendering r;
        memset(&r, 0, sizeof r);
        r.page = pg;
        r.width = 40;
        r.height = 10;
        r.fetcher = f;
        r.print_line = PrintLine;
        r.prompt_and_input = PromptAndInput;
        ny_eprintf("--- Calling FmRender. ---\n");
        FmRender(&r);
        ny_eprintf("--- FmRender Done. ---\n");

        #if 0
        while (true) {
            byte* line = f->readline(f);
            if (!line) break;
            printf("GOT LINE: <%s>\n", line);
        }
        #endif

        f->close(f);

    }

    return 0;
}
