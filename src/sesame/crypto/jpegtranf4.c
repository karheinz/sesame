// This file is based on jpegtran.c part of libjpeg-turbo.
//
// Copyright (c) 2015, Karsten Heinze <karsten.heinze@sidenotes.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
 * jpegtran.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1995-2010, Thomas G. Lane, Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2010, 2014, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for JPEG transcoding.
 * It is very similar to cjpeg.c, and partly to djpeg.c, but provides
 * lossless transcoding between different JPEG file formats.  It also
 * provides some lossless and sort-of-lossless transformations of JPEG data.
 */


#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>

#include "jpeglib.h"
#include "jmemsys.h"

#include "jpegtranf4.h"
#include "jvirt_arrays.h"

#include "cdjpeg.h"             /* Common decls for cjpeg/djpeg applications */
#include "transupp.h"           /* Support routines for jpegtran */
#include "jversion.h"           /* for version message */
#include "jpegint.h"


/*
 * Custom error handling using setjmp()/longjmp().
 * Taken out of example.c from libjpeg-turbo.
 */
struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
};
typedef struct my_error_mgr* my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  //(*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

////////////////////////////////////////////////////////////////////////////////

int f4_extract_message(
  j_common_ptr info,
  jvirt_barray_ptr* coef_arrays,
  char* data,
  const size_t length
)
{
  unsigned int mbits = 0;
  unsigned char mbit;

  // Variables used while evaluating coefficients.
  JDIMENSION rows_to_read;
  JDIMENSION rows_read;
  JDIMENSION rows;
  JDIMENSION row;
  JDIMENSION blocks;
  JDIMENSION block;
  JDIMENSION coeffs = sizeof( JBLOCK ) / sizeof( JCOEF );
  JDIMENSION coeff;
  JBLOCKARRAY blockarray;
  JBLOCKROW blockrow;
  JCOEF value;

  // Evaluate coefficients.
  while ( *coef_arrays != 0 )
  {
     rows_to_read = ( *coef_arrays )->rows_in_array;
     rows_read = 0;
     
     while ( rows_to_read > 0 )
     {
        // Determine number of rows allowed to read at once.
        rows = MIN( rows_to_read, ( *coef_arrays )->maxaccess );

        // Read!
        blockarray = access_virt_barray(
              info,
              *coef_arrays,
              ( *coef_arrays )->cur_start_row + rows_read,  // start
              rows,                                         // rows
              0                                             // writable
           );

        rows_to_read -= rows;
        rows_read += rows;

        // Iterate.
        for ( row = 0; row < rows; ++row )
        {
            blockrow = blockarray[ row ];
            blocks = ( *coef_arrays )->blocksperrow;

            for ( block = 0; block < blocks; ++block ) 
            {
               // Ignore last coefficient.
               for ( coeff = 0; coeff < ( coeffs - 1 ); ++coeff )
               {
                  value = blockrow[ block ][ coeff ];

                  // Only eval non zeroes.
                  if ( value != 0 )
                  {
                     mbit = 0;

                     if ( value < 0 )
                     {
                        if ( value % 2 == 0 )
                        {
                           mbit = 1;
                        }
                     }
                     else
                     {
                        if ( value % 2 == 1 )
                        {
                           mbit = 1;
                        }
                     }

                     if ( ( mbits / 8 ) < length )
                     {
                        mbit = ( mbit << ( 7 - ( mbits % 8 ) ) );
                        data[ mbits / 8 ] |= mbit;
                     }

                     ++mbits;
                  }
               }
            }
        }
     }

     ++coef_arrays;
  }

  // Always return success.
  return 1;
}

int f4_embed_message(
  j_common_ptr info,
  jvirt_barray_ptr* coef_arrays,
  const char* data,
  const size_t length
)
{
  unsigned int mbits = ( length ) * 8;
  unsigned int mbit = 0;

  // Variables used while evaluating coefficients.
  JDIMENSION rows_to_read;
  JDIMENSION rows_read;
  JDIMENSION rows;
  JDIMENSION row;
  JDIMENSION blocks;
  JDIMENSION block;
  JDIMENSION coeffs = sizeof( JBLOCK ) / sizeof( JCOEF );
  JDIMENSION coeff;
  JBLOCKARRAY blockarray;
  JBLOCKROW blockrow;
  JCOEF value;
  JCOEF bit;

  // Evaluate coefficients.
  while ( *coef_arrays != 0 )
  {
     rows_to_read = ( *coef_arrays )->rows_in_array;
     rows_read = 0;

     while ( rows_to_read > 0 )
     {
        // Determine number of rows allowed to read at once.
        rows = MIN( rows_to_read, ( *coef_arrays )->maxaccess );

        // Read!
        blockarray = access_virt_barray(
              info,
              *coef_arrays,
              ( *coef_arrays )->cur_start_row + rows_read,  // start
              rows,                                         // rows
              1                                             // writable
           );

        rows_to_read -= rows;
        rows_read += rows;

        // Iterate.
        for ( row = 0; row < rows; ++row )
        {
            blockrow = blockarray[ row ];
            blocks = ( *coef_arrays )->blocksperrow;

            for ( block = 0; block < blocks; ++block ) 
            {
               // Ignore last coefficient.
               for ( coeff = 0; coeff < ( coeffs - 1 ); ++coeff )
               {
                  // Go on if there is nothing left to embed.
                  if ( mbit == mbits )
                  {
                     continue;
                  }

                  bit = data[ mbit / 8 ] >> ( 7 - ( mbit % 8 ) );
                  bit &= 1;

                  value = blockrow[ block ][ coeff ];

                  // Ignore zero values.
                  if ( value == 0 )
                  {
                     continue;
                  }
                  else if ( value > 0 )
                  {
                     if ( ( value & 1 ) != bit )
                     {
                        --value;
                     }
                  }
                  else
                  {
                     if ( ( value & 1 ) == bit )
                     {
                        ++value;
                     }
                  }

                  blockrow[ block ][ coeff ] = value;
                  if ( value != 0 )
                  {
                     ++mbit;
                  }
               }
            }
        }
     }
     ++coef_arrays;
  }

  // All data embedded?
  return ( mbit == mbits );
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Embeds data into image read from filename_in and
 * writes result to filename_out.
 */
int
f4_embed( const char *filename_in, const char* filename_out, const char* data, const size_t length )
{
  JCOPY_OPTION copyoption = JCOPYOPT_DEFAULT;
  struct jpeg_decompress_struct srcinfo;
  struct jpeg_compress_struct dstinfo;
  struct my_error_mgr jsrcerr, jdsterr;
  jvirt_barray_ptr * src_coef_arrays;
  jvirt_barray_ptr * dst_coef_arrays;

  /* We assume all-in-memory processing and can therefore use only a
   * single file pointer for sequential input and output operation.
   */
  FILE * fp;

  /* Initialize the JPEG decompression object with default error handling. */
  srcinfo.err = jpeg_std_error(&jsrcerr.pub);
  jsrcerr.pub.error_exit = my_error_exit;
  if (setjmp(jsrcerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&srcinfo);
    return 1;
  }
  jpeg_create_decompress(&srcinfo);

  /* Initialize the JPEG compression object with default error handling. */
  dstinfo.err = jpeg_std_error(&jdsterr.pub);
  jdsterr.pub.error_exit = my_error_exit;
  if (setjmp(jsrcerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&srcinfo);
    jpeg_destroy_compress(&dstinfo);
    return 2;
  }
  jpeg_create_compress(&dstinfo);

  /* Set memory limit to 10M. */
  dstinfo.mem->max_memory_to_use = 10000000;
  srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

  /* Open the input file. */
  if ((fp = fopen(filename_in, READ_BINARY)) == NULL) {
    jpeg_destroy_decompress(&srcinfo);
    jpeg_destroy_compress(&dstinfo);
    return 3;
  }

  /* Specify data source for decompression */
  jpeg_stdio_src(&srcinfo, fp);

  /* Enable saving of extra markers that we want to copy */
  jcopy_markers_setup(&srcinfo, copyoption);

  /* Read file header */
  (void) jpeg_read_header(&srcinfo, TRUE);

  /* Read source file as DCT coefficients */
  src_coef_arrays = jpeg_read_coefficients(&srcinfo);

  /* Initialize destination compression parameters from source values */
  jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

  /* Optimize Huffman table (smaller file, but slow compression) */
  dstinfo.optimize_coding = TRUE;

////////////////////////////////////////////////////////////////////////////////

  if ( ! f4_embed_message( (j_common_ptr)&srcinfo, src_coef_arrays, data, length ) )
  {
    jpeg_destroy_decompress(&srcinfo);
    jpeg_destroy_compress(&dstinfo);
    return 4;
  }

////////////////////////////////////////////////////////////////////////////////

  dst_coef_arrays = src_coef_arrays;

  /* Close input file, if we opened it.
   * Note: we assume that jpeg_read_coefficients consumed all input
   * until JPEG_REACHED_EOI, and that jpeg_finish_decompress will
   * only consume more while (! cinfo->inputctl->eoi_reached).
   * We cannot call jpeg_finish_decompress here since we still need the
   * virtual arrays allocated from the source object for processing.
   */
  fclose(fp);

  /* Open the output file. */
  if ((fp = fopen(filename_out, WRITE_BINARY)) == NULL)
  {
    jpeg_destroy_decompress(&srcinfo);
    jpeg_destroy_compress(&dstinfo);
    return 5;
  }

  /* Specify data destination for compression */
  jpeg_stdio_dest(&dstinfo, fp);

  /* Start compressor (note no image data is actually written here) */
  jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

  /* Copy to the output file any extra markers that we want to preserve */
  jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

  /* Finish compression and release memory */
  jpeg_finish_compress(&dstinfo);
  jpeg_destroy_compress(&dstinfo);
  (void) jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);

  /* Close output file, if we opened it */
  fclose(fp);

  /* All done. */
  return 0;
}

/*
 * Extracts data out of image read from filename and
 * writes result to data.
 */
int
f4_extract( const char *filename, char* data, const size_t length )
{
  JCOPY_OPTION copyoption = JCOPYOPT_DEFAULT;
  struct jpeg_decompress_struct srcinfo;
  struct my_error_mgr jsrcerr;
  jvirt_barray_ptr * src_coef_arrays;

  /* We assume all-in-memory processing and can therefore use only a
   * single file pointer for sequential input and output operation.
   */
  FILE * fp;

  /* Initialize the JPEG decompression object with default error handling. */
  srcinfo.err = jpeg_std_error(&jsrcerr.pub);
  jsrcerr.pub.error_exit = my_error_exit;
  if (setjmp(jsrcerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&srcinfo);
    return 1;
  }
  jpeg_create_decompress(&srcinfo);

  /* Set memory limit to 10M. */
  srcinfo.mem->max_memory_to_use = 10000000;

  /* Open the input file. */
  if ((fp = fopen(filename, READ_BINARY)) == NULL) {
    jpeg_destroy_decompress(&srcinfo);
    return 2;
  }

  /* Specify data source for decompression */
  jpeg_stdio_src(&srcinfo, fp);

  /* Read file header */
  (void) jpeg_read_header(&srcinfo, TRUE);

  /* Read source file as DCT coefficients */
  src_coef_arrays = jpeg_read_coefficients(&srcinfo);

////////////////////////////////////////////////////////////////////////////////

  int success = f4_extract_message( (j_common_ptr)&srcinfo, src_coef_arrays, data, length );

////////////////////////////////////////////////////////////////////////////////

  (void) jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);

  /* Close output file, if we opened it */
  fclose(fp);

  /* All done. */
  return ( success ? 0 : 3 );
}
