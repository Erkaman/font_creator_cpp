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


/*
  Constants:
*/


// horizontal and vertical resolution in DPI
#define RESOLUTION 72


/*
  Function definitions:
*/

    void CheckFTError(const FT_Error error, const char* filename, const int line);


/*
  FreeType-Check

  Macro used for handling when FT functions return error codes.
*/
#define FT_C(stmt) do {					\
	FT_Error error = stmt;				\
	CheckFTError(error, __FILE__, __LINE__);	\
    } while (0)

int main() {

    const FT_F26Dot6 font_size = 64;
    const unsigned int atlas_width = 1024;
    const unsigned int atlas_height = 1024;



    FT_Library  library;
    FT_Face     face;

    FT_C(FT_Init_FreeType( &library ));

    FT_C(FT_New_Face( library,
		      "./Ubuntu-B.ttf",
		      0, // face index. We'll be assuming there is only one face in the file.
		      &face ));

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

    FT_C(FT_Load_Char(face, 'A', FT_LOAD_RENDER));

    FT_GlyphSlot g = face->glyph;
    FT_Bitmap bm = g->bitmap;

    unsigned int width = bm.width;
    unsigned int height = bm.rows;
    unsigned int num_pixels = width * height;

    //contains REBA values, with a byte for each channel.
    unsigned char* png_buffer = new unsigned char[num_pixels * 4];

    for(unsigned int i = 0; i < num_pixels; ++i) {
	// for this value, a value of 255, means fully opaque.
	// 0 means fully transparent.
	// so it is the alpha value.
	unsigned char a = bm.buffer[i];

	png_buffer[4*i + 0] = 255;
	png_buffer[4*i + 1] = 255;
	png_buffer[4*i + 2] = 255;
	png_buffer[4*i + 3] = a;
    }

    unsigned int error = lodepng_encode32_file("out.png", png_buffer, width, height);


    /*if there's an error, display it*/
    if(error) {
	printf("error %u: %s\n", error, lodepng_error_text(error));
	exit(1);
    }

    /*
        //Encode the image
    unsigned error = lodepng::encode(filename, pixels, width, height);


  //if there's an error, display it
   if(error)
      LOG_E("PNG encoder error: %d: %s", error, lodepng_error_text(error) );

    */
}

/*
  If no FT error, do nothing.
  Else, print FT error message, and shut down.
*/
void CheckFTError(const FT_Error error, const char* filename, const int line) {
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
