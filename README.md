
= Compiling:

It's cmake, I'm sorry, but it isn't actually all that terrible, I promise!

```
mkdir build
cd build
#debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
#release build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j5
cd ..
```

Resulting binary is at ./build/tubdown, run it from the main directory as it is going to look for a file called 'home'.


= Using (note this functionality is part of demo.c, not native to tubdown, though tubdown does provide some utilities to make this stuff trivial).

TAB: cycles through links

SPACE: follow active link

b: return to previous page

Q/Esc: Quit

