#pragma once

#include "libsm64.h"
#include <stdio.h>

extern SM64DebugPrintFunctionPtr g_debug_print_func;

#define DEBUG_PRINT( ... ) do { \
    if( g_debug_print_func ) { \
        char debugStr[1024]; \
        sprintf( debugStr, __VA_ARGS__ ); \
        g_debug_print_func( debugStr ); \
    } else { \
        printf( __VA_ARGS__ ); \
        printf( "\n" ); \
    } \
} while(0)
