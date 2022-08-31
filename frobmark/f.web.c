#include "frobio/nytypes.h"
#include "frobio/nyformat.h"
#include "frobio/frobmark/fetch.h"
#include "frobio/frobmark/parseurl.h"
#include "frobio/frobmark/markup.h"

error PrintLine(byte* b) {
    ny_eprintf("%2d: {%s}\n", strlen(b), b);
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

bool just_print_links;
Rendering rend;
int main(int argc, char *argv[]) {
    argc--, argv++;  // Skip unused argv 0.
    while (argc && argv[0][0]=='-') {
        switch (argv[0][1]) {
        case 'l':
            just_print_links = true;
            break;
        default:
            Usage();
        }
        argc--, argv++;
    }


    if (argc != 1) Usage();
    char* url_str = argv[0];

    InstallOpener("file:", FileFetcher_Open);

    for (byte pg=0; pg<=10;pg++) {
        ny_eprintf("###### PAGE %d #######\n", pg);
        Url url;
        error e = ParseUrl(url_str, &url);
        if (e) {
            ny_eprintf("cannot ParseUrl %q: error %d\n", url_str, e);
            return 255;
        }
        // ny_eprintf("Parsed URL: %s\n", UrlToStr(&url));

        Fetcher* f = FetcherFactory(&url);
        if (!f) {
            ny_eprintf("cannot ParseUrl %q\n", url_str);
            return 255;
        }

        memset(&rend, 0, sizeof rend);
        rend.page = pg;
        rend.just_print_links = just_print_links;
        rend.width = 40;
        rend.height = 10;
        rend.fetcher = f;
        rend.print_line = PrintLine;
        rend.prompt_and_input = PromptAndInput;
        // ny_eprintf("--- Calling FmRender. ---\n");
        FmRender(&rend);
        // ny_eprintf("--- FmRender Done. ---\n");

        f->close(f);
        free(f);
        DeleteUrl(&url);
    }

    return 0;
}
