rop-fk
---
Taking return oriented programming a bit too literally.

What is this cursed creation? It's a brainfuck interpreter, except each instruction is "interpreted" by adding a return address to a smashed stack buffer. It manages to run and gracefully exit, with only some extremely bizarre memory corruption issues sometimes.

It correctly runs hello world, which means it's good enough for me. It's also a very very precariously balanced pile of memory corruption, so lots of seemingly insignificant changes can easily break the code. For example, removing some of the print statements from before the code executes causes the output to be mangled. Why? I have no clue

Usage
---
`./rop.sh [script]`

If you look at `rop.sh` you see that it actually does this:
```
./rop-fk $1 > a.tmp; cat a.tmp; rm a.tmp
```

And you may be wondering, why? Why? Why does that script exist? What does it do?

The answer is that otherwise something gets corrupted and the code outputs garbage

Why? I have literally no idea.

The code will execute correctly under `gdb` and the `rop.sh` script, but not directly (`./rop-fk [script]`). Why? I have no idea
