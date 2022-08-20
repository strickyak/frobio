#include "frobio/frobmark/parseurl.h"
#include "frobio/ncl/buf.h"
#include "frobio/ncl/malloc.h"

struct test {
    char* input;
    Url want;
} Tests[] = {
  {
    "t://frodo.bilbo.xyz:6789/one/two/three",
    { true, "t:", "frodo.bilbo.xyz:6789", "/one/two/three" }
  },
  {
    "//frodo.bilbo.xyz:6789/one/two/three",
    { true, NULL, "frodo.bilbo.xyz:6789", "/one/two/three" }
  },
  {
    "/one/two/three",
    { true, NULL, NULL, "/one/two/three" }
  },
  {
    "one/two/three",
    { true, NULL, NULL, "one/two/three" }
  },
  {
    NULL,
    { false, NULL, NULL, NULL }
  },
};

int main(int argc, char *argv[]) {
    struct test *t;
    for (t = Tests; t->input; t++) {
        printf("Test #%d: `%s`\n", (int)(t-Tests), t->input);
        Url got;
        error e = ParseUrl(t->input, &got);
        assert(!e);
        assert(t->want.valid == got.valid);
        assert(t->want.scheme == got.scheme ||
               !strcmp(t->want.scheme , got.scheme));
        assert(t->want.host == got.host ||
               !strcmp(t->want.host , got.host));
printf("PATH: want `%s` got `%s`\n", t->want.path, got.path);
        assert(t->want.path == got.path ||
               !strcmp(t->want.path , got.path));
    }
    printf("DONE (parseurl_test): passed %d tests\n", (int)(t-Tests));
    return 0;
}
