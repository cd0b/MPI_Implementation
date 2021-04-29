#ifndef _MEM_H
#define _MEM_H

#include <string.h>

#define MAX_PROC_COUNT 65536 // 2^16

void init_mem(void*, unsigned short);
unsigned short get_proc_count(const void* const);
unsigned short alloc_proc_rank(void* const);
void free_proc(void*, int);
int memwrite(void*, unsigned short, unsigned short, const void*, unsigned int, int);
int memread(void*, unsigned short, unsigned short, void*, unsigned int, int);

#endif