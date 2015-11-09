/*

Copyright (c) 2015, Eric Arnebäck(arnebackeric@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:



The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/





/*
  include FT.
*/
#include <ft2build.h>
#include FT_FREETYPE_H


/*
  According to fterrors.h, we must do this before including FT_ERRORS_H
*/
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };

const struct
{
    int          err_code;
    const char*  err_msg;
} ft_errors[] =
#include FT_ERRORS_H
/*
  ft_errors can now be used to find an error message for all FT errors.
*/


/*
  lodepng is used for writing the font atlas png images.
 */
#include "lodepng.h"



#include <stdio.h>
#include <string>

using std::string;


/*
  Constants:
*/



/*
  Function definitions:
*/

void check_ft_error(const FT_Error error, const char* filename, const int line);

/*
  Copy the font bitmap into the atlas buffer, starting at the pixel coordinates (start_x,start_y)
 */
void copy_font_bitmap(unsigned char atlas_buffer[], FT_Bitmap bitmap,
		      unsigned int start_x, unsigned int start_y);

// Strip the file extension from a file name.
// If for instance str = "file.txt", then "file" will be returned.
string strip_file_extension(const string& str);

/*
Given the maximum width and height of all characters bitmaps, find an atlas size
that will fit all characters, yet is, approximately, as small as possible.
 */
unsigned int find_atlas_size(unsigned int max_width, unsigned int max_height);



/*
  FreeType-Check

  Macro used for handling when FT functions return error codes.
*/
#define FT_C(stmt) do {					\
	FT_Error error = stmt;				\
	check_ft_error(error, __FILE__, __LINE__);	\
    } while (0)


// horizontal and vertical resolution in DPI
#define RESOLUTION 72

#define START_CHAR 32
#define END_CHAR 95 // 90

// the spacing between rows of characters in the atlas.
#define ROW_SPACING 10

/*
  Global variables.
 */
const FT_F26Dot6 font_size = 64;
unsigned int atlas_width;
unsigned int atlas_height;

int main() {

    FT_Library  library;
    FT_Face     face;

    FT_C(FT_Init_FreeType( &library ));

    const string input_file = "../Ubuntu-B.ttf";

    FT_C(FT_New_Face( library,
		      input_file.c_str(),
		      0, // face index. We'll be assuming there is only one face in the file.
		      &face ));

    // all files outputted by this program will start with this string.
    const string output_file_prefix = strip_file_extension(input_file);

    if(face->num_faces != 1) {
	printf("This file has %ld font face(s), but this program only supports one face/n", face->num_faces);
	exit(1);
    }













    // set the font size.
    FT_C(FT_Set_Char_Size(
	     face,    // handle to face object
	     font_size * 64,  /* char_width  */
	     0,   //char_height. It is 0, so it is set to char_width
	     RESOLUTION,     /* horizontal device resolution    */
	     RESOLUTION ));   /* vertical device resolution      */




    /*
      Next, we determine the size of the atlas.
     */

    unsigned int max_width = 0;
    unsigned int max_height = 0;


    for(unsigned int ch = START_CHAR; ch <= END_CHAR; ++ch) {

	FT_C(FT_Load_Char(face, (char)ch, FT_LOAD_RENDER));

	FT_GlyphSlot glyph = face->glyph;
	FT_Bitmap bitmap = glyph->bitmap;

	if(bitmap.rows > max_height) {
	    max_height = bitmap.rows;
	}

	if(bitmap.width > max_width) {
	    max_width = bitmap.width;
	}
    }

    printf("atlas_width: %d\n", atlas_width);
    printf("atlas_height: %d\n", atlas_height);

    printf("max_width: %d\n", max_width);
    printf("max_height: %d\n", max_height);

    atlas_width = find_atlas_size(max_width, max_height);
    atlas_height = atlas_width;


/*    atlas_width = 1024;
    atlas_height = 1024;
*/


//    find_atlas_size(max_width, max_height);

    /*
      Allocate the atlas pixel buffer.
     */

    printf("atlas width: %d\n", atlas_width);
    printf("atlas heigit: %d\n", atlas_height);

    unsigned int atlas_num_pixels = atlas_width * atlas_height;

    //contains REBA values, with a byte for each channel.
    unsigned char* atlas_buffer = new unsigned char[atlas_num_pixels * 4];

    // initially, set all atlas pixels to fully transparent white: (1,1,1,0).
    for(int i = 0; i < atlas_num_pixels; ++i) {
	atlas_buffer[4*i + 0] = 255;
	atlas_buffer[4*i + 1] = 255;
	atlas_buffer[4*i + 2] = 255;
	atlas_buffer[4*i + 3] = 0;
    }




    FILE* fp = fopen((output_file_prefix+string(".amf")).c_str(), "w");

    //unsigned int max_height = max_bitmap_top + max_rows;

    unsigned int atlas_x = 0;
    unsigned int atlas_y = 0;

    for(unsigned int ch = START_CHAR; ch <= END_CHAR; ++ch) {

	FT_C(FT_Load_Char(face, (char)ch, FT_LOAD_RENDER));

	FT_GlyphSlot glyph = face->glyph;
	FT_Bitmap bitmap = glyph->bitmap;

	const unsigned int bitmap_width = bitmap.width;
	const unsigned int bitmap_height = bitmap.rows;

	// start a new row, if the current one is already filled.
	if(bitmap_width + atlas_x > atlas_width) {
	    atlas_x = 0;

	    atlas_y += max_height/*+ROW_SPACING*/;
	}

	// ensure that the characters are not crammed together
	atlas_x += glyph->bitmap_left;

	copy_font_bitmap(atlas_buffer, bitmap, atlas_x, atlas_y/* + max_bitmap_top - glyph->bitmap_top*/);

	string line =
	    string(1,(char)ch) + "," +
	    std::to_string(atlas_x) + "," +
	    std::to_string(atlas_y) + "," +
	    std::to_string(bitmap_width) + "," +
	    std::to_string(bitmap_height) +

	    "\n";

	fputs(line.c_str(), fp);



	// move to the next letter.
	atlas_x += bitmap_width;
    }




    unsigned int error = lodepng_encode32_file((output_file_prefix+string(".png")).c_str(), atlas_buffer, atlas_width, atlas_height);


    /*if there's an error, display it*/
    if(error) {
	printf("error %u: %s\n", error, lodepng_error_text(error));
	exit(1);
    }

    /*
      Clean up
     */

    fclose(fp);
    delete[] atlas_buffer;
    FT_C(FT_Done_FreeType( library ));

    system("open Ubuntu-B.png");
}

/*
  If no FT error, do nothing.
  Else, print FT error message, and shut down.
*/
void check_ft_error(const FT_Error error, const char* filename, const int line) {
    if(error == 0) {
	// no error.
	return;
    }

    // else, we write out an error message and shut down.
    printf("At line %d: Error %d: %s\n",
	   line,
	   ft_errors[FT_Err_Unknown_File_Format].err_code,
	   ft_errors[FT_Err_Unknown_File_Format].err_msg);
    exit(1);
}

// 954

void copy_font_bitmap(unsigned char atlas_buffer[], FT_Bitmap bitmap,
		      unsigned int start_x, unsigned int start_y) {

    // atlas row width in bytes.
    unsigned int atlas_row_size = atlas_width * 4;

    //printf("atlas_row_size: %d\n", atlas_row_size);
//    printf("start_y: %d\n", start_y);

    unsigned int atlas_i = atlas_row_size * start_y + start_x * 4;
//    printf("atlas_i: %d\n", atlas_i);




    const unsigned int bitmap_width = bitmap.width;
    const unsigned int bitmap_height = bitmap.rows;
    const unsigned int bitmap_num_pixels = bitmap_width * bitmap_height;

    unsigned int bitmap_x = 0;

    /* printf("bitmap_width: %d\n", bitmap_width);
    printf("bitmap_height: %d\n", bitmap_height);
    */
    for(int bitmap_i = 0; bitmap_i < bitmap_num_pixels; ++bitmap_i) {

	// for this value, a value of 255, means fully opaque.
	// 0 means fully transparent.
	// so it is the alpha value.
	unsigned char a = bitmap.buffer[bitmap_i];

//	("atlias_i %d\n", atlas_i);

//	printf("atlas_i: %d\n", atlas_i);

	atlas_buffer[atlas_i + 0] = 255;
	atlas_buffer[atlas_i + 1] = 0;
	atlas_buffer[atlas_i + 2] = 255;
	atlas_buffer[atlas_i + 3] = a;

	if( ( (bitmap_i+1) % bitmap_width) ==0 && bitmap_i != 0 ) {
	    // new row:
	    atlas_i += atlas_row_size - 4 * bitmap_width + 4;
//	    printf("FLIP\n");
//	    printf("atlas_i: %d\n", atlas_i);

	} else {
	    atlas_i += 4;
//	    printf("atlas_i: %d\n", atlas_i);

	}

    }
}


string strip_file_extension(const string& str) {
    size_t last_dot = str.find_last_of(".");

    return str.substr(0,last_dot);
}

unsigned int find_atlas_size(unsigned int max_width, unsigned int max_height) {

    // an atlas smaller than 128x128 will probably not exist :)
    unsigned int atlas_size = 128;

    while(true) {

	unsigned int total_width = max_width * (END_CHAR - START_CHAR);

	unsigned int rows = total_width / atlas_size;

	printf("atlas_size: %d\n",  atlas_size);

	if(rows < 1) {
	    printf("WAT\n");
	    exit(1);
	}

	if(rows * max_height < atlas_size) {
	    // enough rows. Found an atlas size big enough.
	    break;
	}

	atlas_size *= 2;

    }


    return atlas_size;

}
