#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <cstdlib>
#include <cstdio>
#include <error.h>

#define ERROR_HANDLE(s) \
    do { \
        perror(s); \
        exit(EXIT_FAILURE); \
    } while (0); \

#endif
