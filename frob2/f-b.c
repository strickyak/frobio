#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

#include "frob2/tweb/fetch.h"
#include "frob2/tweb/markup.h"
#include "frob2/tweb/parseurl.h"

errnum PrintLine(byte* b) {
    EPrintf("%2d: {%s}\n", StrLen(b), b);
    return OKAY;
}

errnum PromptAndInput(byte* buf, word max) {
    EPrintf("  >>?>> ");
    //# fgets(buf, max, stdin);
    return 255;
}

static void FatalUsage() {
    LogFatal("Usage:  f.b  FirstuRL\n");
    // printf("  -a for render all and quit.\n");
    // printf("  -l for print links and quit.\n");
}

bool just_print_links = false;
Rendering rend;
int main(int argc, char *argv[]) {
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "lv:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
        case 'l':
            just_print_links = true;
            break;
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         wiz_hwport = (byte*)prefixed_atoi(FlagArg);
         break;
      default:
        FatalUsage();
    }
  }

  if (argc != 1) {
    FatalUsage();
  }
    char* url_str = argv[0];

    InstallOpener("file:", FileFetcher_Open);

    for (byte pg=0; pg<=10;pg++) {
        EPrintf("(( Page %d ))\n", pg);
        Url url;

        errnum e = ParseUrl(url_str, &url);
        if (e) LogFatal("cannot ParseUrl %q: errnum %d\n", url_str, e);

        Fetcher* f = FetcherFactory(&url);
        if (!f) LogFatal("cannot ParseUrl %q\n", url_str);

        memset(&rend, 0, sizeof rend);
        rend.page = pg;
        rend.just_print_links = just_print_links;
        rend.width = 40;
        rend.height = 10;
        rend.fetcher = f;
        rend.print_line = PrintLine;
        rend.prompt_and_input = PromptAndInput;
        FmRender(&rend);

        f->close(f);
        Free(f);
        DeleteUrl(&url);
    }

    return 0;
}
