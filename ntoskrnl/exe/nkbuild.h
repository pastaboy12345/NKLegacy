#ifndef NK_BUILD_H
#define NK_BUILD_H

#include <stdint.h>

void nkbuild_start(void);
int nkbuild_process_line(const char* line);
uint32_t nkbuild_get_exe_size(void);
void* nkbuild_get_exe_buffer(void);

#endif
