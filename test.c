#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_time.h>

#include "naivesearch.h"

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

static void benchmark(void) {
    BANNER("Benchmark");
    char *needleFound = "theoretical", *needleNotFound = "theoretical fish";
    int needleFoundLen = strlen(needleFound), needleNotFoundLen = strlen(needleNotFound);
    int haystackLen = 1438084;
    char *haystack = calloc(haystackLen + 1, sizeof(char));
    FILE *f = fopen("sicp.texi", "r");
    size_t sz = fread(haystack, sizeof(char), haystackLen, f);
    assert(sz == haystackLen);

    mach_timebase_info_data_t timebaseInfo;
    mach_timebase_info(&timebaseInfo);

    uint64_t start = mach_absolute_time();
    for (long i = 0; i < 1000; i++) naiveSearch(needleFound, needleFoundLen, haystack, haystackLen);
    uint64_t elapsed = mach_absolute_time() - start;
    uint64_t ns = elapsed * timebaseInfo.numer / timebaseInfo.denom;
    printf("Found: %0.3f ms\n", (double)ns / 1000000.0);
    
    start = mach_absolute_time();
    for (long i = 0; i < 1000; i++) naiveSearch(needleNotFound, needleNotFoundLen, haystack, haystackLen);
    elapsed = mach_absolute_time() - start;
    ns = elapsed * timebaseInfo.numer / timebaseInfo.denom;
    printf("Not found: %0.3f ms\n", (double)ns / 1000000.0);
}

int main(void) {
    testBasicSearch();
    testEmptyStrings();
    while (1) benchmark();
    return 0;
}
    
