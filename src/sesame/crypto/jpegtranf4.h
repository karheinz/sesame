#ifndef SESAME_UTILS_JPEGTRANF4_H
#define SESAME_UTILS_JPEGTRANF4_H

#ifdef __cplusplus
extern "C" {
#endif
int f4_embed(
   const char *filename_in,
   const char* filename_out,
   const char* data,
   const size_t length
   );

int f4_extract(
   const char *filename,
   char* data,
   const size_t length
   );
#ifdef __cplusplus
}
#endif

#endif
