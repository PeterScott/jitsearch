// Naive string search: a baseline for everything that's coming next.

int naiveSearch(const char *restrict needle, int needleLen,
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
