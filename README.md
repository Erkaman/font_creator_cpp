Description
=============

This program uses FreeType to generate a bitmap texture atlas from a
.ttf or .otf file. This atlas could be used to efficiently render text
in a game.

Dependencies
==============

Lodepng and Freetype. However, because Lodepng is a rather tiny
library, for reasons of convenience it has been included in this
repository. So the only real dependency is Freetype.

Building
==============

The program is built using cmake as follows:

```
mkdir build && cd build && cmake .. && make
```

This should produce an executable named font_creator_cpp.

Usage
==============

TODO

TODO
==============

* The program does not yet support italic fonts.
