#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"

void fizzbuzz(byte* buf, word x) {
    if (x%100 == 0) {
        strcpy(buf, "wifjwjfiejwofwijfiowejfoiewjfiewjfoiewjfewfjoewjffjeabcdefghijklmnopq");
    } else if (x%15 == 0) {
        strcpy(buf, "FIZZBUZZ");
    } else if (x%5 == 0) {
        strcpy(buf, "buzz");
    } else if (x%3 == 0) {
        strcpy(buf, "fizz");
    } else {
        sprintf(buf, "%d", x);
    }
}

word cur;
error TestGetLine(byte* buf, word max) {
    switch(cur) {
    case 0: strcpy(buf, "zero"); break;
    case 1: strcpy(buf, "one"); break;
    case 2: strcpy(buf, "two"); break;
    //case 3: strcpy(buf, ""); return E_EOF;
    //default: assert(0);
    default:
        if (cur > 1000) {
            return E_EOF;
        }
        strcpy(buf, "foobar");
        fizzbuzz(buf, cur);
    }
    cur++;
    return 0;
}

error TestPrintLine(byte* buf) {
    printf("\n#### {%s} %ld ####\n", buf, (long)strlen(buf));
    return 0;
}

int main() {
    printf("\n####---------------------------------------------\n");

    for (word page = 0; page < 12; page++) {
        cur = 0;
        FmRender(60, 10, page, TestGetLine, TestPrintLine, NULL);
        printf("\n####---------------------------------------------\n");
    }
    printf("\nFIN");
    
    return 0;
}
