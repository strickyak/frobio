#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"

void fizzbuzz(byte* buf, word x) {
    char* s = (char*)buf;
    if (x%100 == 0) {
        strcpy(s, "wifjwjfiejwofwijfiowejfoiewjfiewjfoiewjfewfjoewjffjeabcdefghijklmnopq");
    } else if (x%15 == 0) {
        strcpy(s, "FIZZBUZZ");
    } else if (x%5 == 0) {
        strcpy(s, "buzz");
    } else if (x%3 == 0) {
        strcpy(s, "fizz");
    } else {
        sprintf(s, "%d", x);
    }
}

word line_num;
error TestGetLine(byte* buf, word max) {
    if (line_num > 1000)
        return E_EOF;
    fizzbuzz(buf, line_num);
    line_num++;
    return OKAY;
}

error TestPrintLine(byte* buf) {
    printf("\n<<< %3ld {%s} >>>\n", (long)strlen((const char*)buf), buf);
    return OKAY;
}

Rendering render_me; 

int main() {
    render_me.width = 60;
    render_me.height = 10;
    render_me.get_src_line = TestGetLine;
    render_me.print_line = TestPrintLine;
    render_me.prompt_and_input= NULL;
    printf("\n####---------------------------------------------\n");

    for (word page = 0; page < 12; page++) {
        render_me.page = page;
        FmRender(&render_me);
        printf("\n####---------------------------------------------\n");
    }
    printf("\nFIN");
    
    return 0;
}
