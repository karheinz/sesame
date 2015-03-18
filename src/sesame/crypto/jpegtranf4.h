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


#ifndef SESAME_CRYPTO_JPEGTRANF4_H
#define SESAME_CRYPTO_JPEGTRANF4_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Takes image in <tt>filename_in</tt>, embeds <tt>data</tt> and
 * writes result to <tt>filename_out</tt>.
 *
 * @param filename_in source image
 * @param filename_out target image
 * @param data data to embed
 * @param length length of data to embed
 *
 * @return 0 => success,
 *         1 => internal error,
 *         2 => internal error,
 *         3 => couldn't open filename_in,
 *         4 => embedding failed,
 *         5 => couldn't open filename_out
 */
int f4_embed(
   const char *filename_in,
   const char* filename_out,
   const char* data,
   const size_t length
   );

/**
 * Extracts data out of <tt>filename_in</tt> and writes
 * it to <tt>data</tt>.
 *
 * @param filename_in source image
 * @param filename_out target image
 * @param data data to embed
 * @param length length of data to embed
 *
 * @return 0 => success,
 *         1 => internal error,
 *         2 => couldn't open filename_in,
 *         3 => extracting failed
 */
int f4_extract(
   const char *filename,
   char* data,
   const size_t length
   );

#ifdef __cplusplus
}
#endif

#endif
