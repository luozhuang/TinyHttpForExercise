#ifndef __GLOBALDEF_H__
#define __GLOBALDEF_H__
#include <cstdlib>
#define ERROR_EXIT(x) \
    do \
    { \
        perror(x); \
        exit(EXIT_FAILURE); \
    } while(0) \
#endif
