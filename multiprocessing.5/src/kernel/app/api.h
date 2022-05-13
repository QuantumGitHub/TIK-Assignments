#ifndef API_H
#define API_H

#include "../scalls.h"
#include "../types.h"

enum PROGRAMS { TESTING, HELLO_JR };

void put(const char a);
void evoke(const char arg1, const char arg2);
uint64_t mmap(const char arg1);
int getpid(void);
uint64_t syscall(const char ID, const char arggetpid1, const char arg2);
uint64_t get_arg(void);
#endif
