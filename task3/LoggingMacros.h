#ifndef LOGGINGMACROS_H
#define LOGGINGMACROS_H

#include <iostream>


#define ENABLE_LOG 1
#define ENABLE_DEBUG 0

#if ENABLE_LOG
#define LOG(msg) std::cout << "[LOG] " << msg << std::endl
#else
#define LOG(msg)
#endif

#if ENABLE_DEBUG
#define DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
#define DEBUG(msg)
#endif

#if ENABLE_DEBUG
#define DEBUG_VAR(var) std::cout << "[DEBUG] " << #var << " = " << (var) << std::endl
#else
#define DEBUG_VAR(var)
#endif

#endif
