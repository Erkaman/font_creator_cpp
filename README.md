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

The program can be used to create a font atlas as follows:

```
./font_creator_cpp -fs 80 Ubuntu-B.ttf
```

which creates a font atlas of the font Ubuntu-B.ttf, and the font size is 80. Running the command creates two files 
`Ubuntu-B-80.png` and `Ubuntu-B-80.amf`. `Ubuntu-B-80.png` is simply the font atlas image:

![text](img/Ubuntu-B-80.png)

`Ubuntu-B-80.amf` specifies the exact position of every character in the atlas. The file is a long sequence of
lines, and every character has a line. For instance the character `#` has the line:

```
#,219,0,52,78,4
```

This specifies that this character can be found at the pixel positon `(219,0)` in the atlas image, it is
52 pixels wide, and 78 pixels high. The number 4 specifies that the cursor should be moved forward 4 pixels
before drawing the character. This ensures that the correct inter-letter spacing of the original font is properly used.

TODO
==============


* The program does not yet support italic fonts.
