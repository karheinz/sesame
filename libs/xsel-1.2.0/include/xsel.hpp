#ifndef XSELECTION_H
#define XSELECTION_H

#include <mutex>

int xopen_display();

void xclip(
    std::mutex* mutex,
    const char* text
  );

#endif
