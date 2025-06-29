#pragma once

void MyAssertHandler(const char* filename, int line, int type, const char* fmt, ...);

#ifdef _DEBUG 
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

#define alwaysfails 0
#else
#define iassert(expression)
#define vassert(expression, fmt, ...)
#define bcassert(expression, maxv)
#define bcassert2(expression, maxv)
#define alwaysfails 0
#endif