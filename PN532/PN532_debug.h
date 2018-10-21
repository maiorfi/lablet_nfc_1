#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "mbed.h"
#include "SWO.h"

// #define DEBUG

#ifdef DEBUG

extern SWO_Channel swo;

#define DMSG(args...)   swo.printf(args)
#define DMSG_STR(str)   swo.printf("%s\n", str)
#define DMSG_INT(num)   swo.printf("%d\n", num)
#define DMSG_HEX(num)   swo.printf("%2X ", num)

#else

#define DMSG(args...)
#define DMSG_STR(str)
#define DMSG_INT(num)
#define DMSG_HEX(num)

#endif  // DEBUG

#endif  // __DEBUG_H__
