#ifndef XSELECTION_H
#define XSELECTION_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

void xclip( pthread_mutex_t* mutex, const char* text );

#ifdef __cplusplus
}
#endif

#endif
