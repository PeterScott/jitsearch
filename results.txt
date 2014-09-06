Searching for "theoretical" and "theoretical fish" in sicp.texi:

    1691 us -> 860 us -> 505 us
    4593 us -> 2440 us -> 1246 us

Speedup: about 1.8-1.9x for bytewise, 3.3-3.7x for word-wise.

strstr takes 2550 us for not found, so we get about a 2x improvement (or a slight one bytewise)
