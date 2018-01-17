Here are the glyphs of all fonts.
Files must be in format ABBBCCC.bmp where A is the font number.
Font1 is the Artist, Font2 is the Title font, Font3 is the studio and Font4 is the price.
BBB is the ASCII code of the character corresponding to this glyph.
CCC is a second character corresponding to this glyph.
Two characters can correspond to a glyph because i noticed that in some scans some characters are merged - like the 'rt' in 'Liberty' (Font3).
In the normal case CCC should be '000'.
In the perfect case Font1, Font2 and font3 should contain 96 entries each - 0-9, A-Z, a-z and 32 symbols. Google 'ASCII table'.
Font4 should contain 11 entries - 0-9 and the dot.
The image must be 24bit MS Windows Bitmap (use paintbrush) and it must be taken from internals\page.bmp.
It must contain only one block. If there are parts of other symbols erase them with white brush.
For glyphs made up of more than one block (ij) leave the biggest and erase the others.
