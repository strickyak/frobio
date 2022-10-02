#include "frobio/frobmark/parseurl.h"

struct parse_test {
    char* input;
    Url want;
} ParseTests[] = {
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

struct join_test {
    char* first;
    char* second;
    Url want;
} JoinTests[] = {
    {
        "abc://xyz.zed/one/two/index.fm",
        "/five/six/",
        { true, "abc:", "xyz.zed", "/five/six/" }
    },
    {
        "abc://xyz.zed/one/two/index.fm",
        "five/six/",
        { true, "abc:", "xyz.zed", "/one/two/five/six/" }
    },
    {
        "abc://xyz.zed/one/two/index.fm",
        "//hostname/five/six/",
        { true, "abc:", "hostname", "/five/six/" }
    },
    {
        "abc://xyz.zed/one/two/index.fm",
        "xyz://hostname/five/six/",
        { true, "xyz:", "hostname", "/five/six/" }
    },
    { NULL },
};

int main(int argc, char *argv[]) {
    int count = 0;
    
    for (struct parse_test *t = ParseTests; t->input; t++) {
        printf("ParseTest #%d: `%s`\n", (int)(t-ParseTests), t->input);
        Url got;
        error e = ParseUrl(t->input, &got);
        assert(!e);
        assert(t->want.valid == got.valid);
        assert(t->want.scheme == got.scheme ||
               !strcmp(t->want.scheme , got.scheme));
        assert(t->want.hostport == got.hostport ||
               !strcmp(t->want.hostport , got.hostport));
        printf(".......... path: want `%s` got `%s`\n", t->want.path, got.path);
        assert(t->want.path == got.path ||
               !strcmp(t->want.path , got.path));
        count++;
    }

    for (struct join_test *t = JoinTests; t->first; t++) {
        printf("JoinTest #%d: `%s` join `%s`\n", (int)(t-JoinTests), t->first, t->second);
        Url a, b, got;
        error e = ParseUrl(t->first, &a);
        assert(!e);
        e = ParseUrl(t->second, &b);
        assert(!e);
        e = JoinUrls(&a, &b, &got);
        assert(!e);
        assert(t->want.valid == got.valid);
        assert(t->want.scheme == got.scheme ||
               !strcmp(t->want.scheme , got.scheme));
        assert(t->want.hostport == got.hostport ||
               !strcmp(t->want.hostport , got.hostport));
        printf(".......... path: want `%s` got `%s`\n", t->want.path, got.path);
        assert(t->want.path == got.path ||
               !strcmp(t->want.path , got.path));
        count++;
    }

    printf("DONE (parseurl_test): passed %d tests\n", count);
    return 0;
}
