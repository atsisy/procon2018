#pragma once

#define __DEBUG_MODE

#ifdef __DEBUG_MODE

#include <iostream>
#include <cstdio>

#define _DEBUG_PUTS_SEPARATOR() (puts("----------------------------"))
#define _DEBUG_PRINTPI(_NAME, _VAL) (printf("%s: %d\n", (char *)_NAME, (int)_VAL))

//#define _ENABLE_YASUDA
#define _ENABLE_TAKAI

#define I_AM_ENEMY
//#define I_AM_ME

inline void who()
{
#ifdef I_AM_ENEMY
        puts("enemy");
#endif
#ifdef I_AM_ME
        puts("me");
#endif
}

#endif
