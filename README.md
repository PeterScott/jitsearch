# JIT string search for fun and profit

**Note: this is a zeroth draft.**

Sometimes you want to find the location of one string in another; think `strstr()` in C, or `string.find()` in Python. If you know what string you're searching for, and you plan to search for it in a *lot* of other strings, it might be a good idea to JIT compile it to machine code at runtime, making a string search specialized for that particular string. V8 does a more advanced version of this for regular expressions with [irregexp](http://blog.chromium.org/2009/02/irregexp-google-chromes-new-regexp.html) And whether or not it's a good idea, it's definitely a *fun* idea, so let's do this.

## Defining the problem

Let's start by *not* jitting anything, just writing a really basic string search. This looks for the string `needle` in the string `haystack`:

```c
int naiveSearch(const char *needle, int needleLen, const char *haystack, int haystackLen) {
    if (needleLen > haystackLen) return -1;
    int startBound = haystackLen - needleLen;
    for (int start = 0; start < startBound; start++) {
        for (int i = 0; i < needleLen; i++) {
            if (needle[i] != haystack[start + i]) goto mismatch;
        }
        return start;
    mismatch: {}
    }
    return -1;  // No match found
}
```

This searches for `needle` at every possible starting position in `haystack`. It's simple, it's easy, and it's fast on small inputs because of the tight inner loop and negligible setup costs. For finding small strings in small strings, this is actually a really good algorithm, despite its O(n*m) runtime.

## Well, that was fun. Let's JIT.

The easiest JIT-writing tool I know of is [DynASM](http://luajit.org/dynasm.html), used by [LuaJIT](http://luajit.org/). There's [a good introduction, with examples](http://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html), which I referred to heavily when writing this. DynASM is a preprocessor for C code, written in Lua, which produces code that will very quickly take values known at runtime and plug them into a template. All the special lines start with a '|' character. Let's go.

First, we need to decide what registers to put the variables in. An ordinary compiler would have a register allocator for this, but we're in the Wild West here, where bandits shoot stagecoaches and then chew tobacco in front of polecats. Luckily, you can define aliases for things:

```
|.define STARTPTR, rdi
|.define ENDPTR, rsi
```

Let's say that we're defining a function with this signature:

```c
int myJittedFunction(char *start, char *end);
```

It takes a pointer to the start of the haystack (the string to search in), and a pointer to the last position in the haystack which is far enough ahead of the beginning that it could be the start of a match. We'll also need a local variable to hold the position we're looking at right now:

```
|.define POS, rax
```

Now, you may be wondering why I picked these particular registers. The answer is that, in [the x86_64 calling conventions used on my platform](http://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI), the first two parameters to a function are passed in the RDI and RSI registers, and RAX is a really boring, conventional register that I picked arbitrarily.

Now, for every possible starting position, we need to look for the string. For any given needle, we can generate code to match it:

```c
void emitSearchCodeForNeedle(char *needle) {
    |  mov POS, STARTPTR
    |1:
    |  cmp POS, ENDPTR; jge >3
    for (int i = 0; needle[i]; i++) {
        |  cmp byte [POS+(i)], (needle[i]); jne >2
    }
    |  mov rax, POS; sub rax, STARTPTR
    |  ret
    |2:
    |  inc POS
    |  jmp <1
    |3:
    |  mov eax, -1
    |  ret
}
```

Those lines of assembly code mean "write the machine code for this to the current buffer". The numeric labels are local; when you jump to one, it jumps to the nearest numeric label with that number in the indicated direction. It's handy if you can remember what number refers to what.

Well! There's some scaffolding involved, but let's handwave that away for now. How does it perform? The answer is, according to my completely unreproducible benchmarks, about **1.8x-1.9x faster than the plain C code**, if you ignore the (fairly fast) time required to do the jitting.

## Faster! It can be faster!

I thought of a trick: compare more than one byte at a time! How about, say, *four* bytes at a time? If the needle is more than four bytes long, then there's definitely potential for this. Now, you may be saying to yourself, "unaligned reads are slow." On Intel's processors, that stopped being true around the Nehalem microarchitecture. So, let's emit code to search four bytes at a time until we're out of four-byte pieces of the needle, and then use byte-at-a-time search for the rest:

```void emitSearchCodeForNeedleWordAtATime(char *needle, int needleLen) {
    |  mov POS, STARTPTR
    |1:
    |  cmp POS, ENDPTR; jge >3;

    uint32_t *dwords = (uint32_t *)needle;
    int ndwords = needleLen / 4;
    for (int i = 0; i < ndwords; i++) {
        |  cmp dword [POS+(i)*4], (dwords[i]); jne >2
    }
    for (int i = ndwords*4; i < needleLen; i++) {
        |  cmp byte [POS+(i)], (needle[i]); jne >2
    }

    |  mov rax, POS; sub rax, STARTPTR
    |  ret
    |2:
    |  inc POS
    |  jmp <1
    |3:
    |  mov eax, -1
    |  ret
}
```

The structure of this is essentially the same as the code above, except that instead of matching a byte at a time, it tries to match in 32-bit words as much as possible.

So, how does it perform? According to my benchmark, about 3.3-3.7x faster than the plain C version. Yay!
