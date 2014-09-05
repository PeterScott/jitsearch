// Naive string search: a baseline for everything that's coming next.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int
naiveSearch(const char *restrict needle, int needleLen,
            const char *restrict haystack, int haystackLen) {
    if (needleLen > haystackLen) return -1;
    int startBound = haystackLen - needleLen;
    for (int start = 0; start < startBound; start++) {
        for (int i = 0; i < needleLen; i++) {
            if (needle[i] != haystack[start + i]) goto mismatch;
        }
        return start;
    mismatch: {}
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////

#define BANNER(subject) printf("----------------\nTest: " subject "\n")

static void testBasicSearch(void) {
    BANNER("Basic search");
    char *needle = "hello";
    char *haystack = "Yo, help me say hello, bro";
    int needleLen = strlen(needle), haystackLen = strlen(haystack);
    
    int pos = naiveSearch(needle, needleLen, haystack, haystackLen);
    printf("pos = %i -- %s\n", pos, pos == 16 ? "SUCCESS" : "FAIL");
}

static void testEmptyStrings(void) {
    BANNER("Empty strings");
    int pos = naiveSearch("", 0, "Hello", 5);
    printf("pos = %i -- %s\n", pos, pos == 0 ? "SUCCESS" : "FAIL");
    pos = naiveSearch("hello", 5, "", 0);
    printf("pos = %i -- %s\n", pos, pos == -1 ? "SUCCESS" : "FAIL");
}

int main(void) {
    testBasicSearch();
    testEmptyStrings();
    return 0;
}
    
