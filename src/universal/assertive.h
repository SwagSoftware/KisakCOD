#pragma once

#if defined(_DEBUG) || defined(RELEASE_ASSERTS)
#define USE_ASSERTS
#endif

void MyAssertHandler(const char* filename, int line, int type, const char* fmt, ...);

#ifdef USE_ASSERTS
#define iassert(expression) (void)(                                                       \
            (!!(expression)) ||                                                          \
            (MyAssertHandler(__FILE__, (unsigned)(__LINE__), 0, "%s", #expression), 0) \
        )

#define vassert(expression, fmt, ...)  (void)(                                                       \
            (!!(expression)) ||                                                          \
            (MyAssertHandler(__FILE__, (unsigned)(__LINE__), 0, "%s\n\t" fmt, #expression, __VA_ARGS__), 0) \
        )

#define bcassert(expression, maxv) vassert(((expression) < (maxv)), #expression "%d does not index [0, %d)", expression, maxv)
#define bcassert2(expression, maxv) vassert(((expression) <= (maxv)), #expression "%d does not index [0, %d]", expression, maxv)
#define rangeassert(expression, minv, maxv) vassert(((expression) <= (maxv) && (expression >= (minv))), #expression "not in [" #minv ", " #maxv "]\n\t%i not in [%i, %i]", expression, minv, maxv)
#define nanassertvec3(vec) iassert( !IS_NAN((vec)[0]) && !IS_NAN((vec)[1]) && !IS_NAN((vec)[2]) )
#define alwaysfails 0
#else
#define iassert(expression)
#define vassert(expression, fmt, ...)
#define bcassert(expression, maxv)
#define bcassert2(expression, maxv)
#define rangeassert(expression, minv, maxv)
#define nanassertvec3(vec) 
#define alwaysfails 0
#endif