#ifndef __SAFEWORD_DBG_H
#define __SAFEWORD_DBG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#endif // __SAFEWORD_DBG_H
