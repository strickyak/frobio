#include "frobio/nytypes.h"

static void Usage() {
    printf("Usage:  f.web -a -l FirstuRL\n");
    // printf("  -a for render all and quit.\n");
    // printf("  -l for print links and quit.\n");
    exit(255);
}

int main(int argc, char *argv[]) {
    if (argc != 2) Usage();
    char* url = argv[1];
    while (url) {
        char* lines = FetchSourceAsLines(url);
        url = RenderLines(lines);
    }
    return 0;
}
