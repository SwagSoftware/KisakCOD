#pragma once

void MyAssertHandler(const char* filename, int line, int type, const char* fmt, ...);

#define iassert(expression) (void)(                                                       \
            (!!(expression)) ||                                                          \
            (MyAssertHandler(__FILE__, (unsigned)(__LINE__), 0, "%s", #expression), 0) \
        )

#define vassert(expression, fmt, ...)  (void)(                                                       \
            (!!(expression)) ||                                                          \
            (MyAssertHandler(__FILE__, (unsigned)(__LINE__), 0, "%s\n\t" fmt, #expression, __VA_ARGS__), 0) \
        )

#define bcassert(expression, maxv) vassert(((expression) < (maxv)), #expression "%d does not index [0, %d)", expression, maxv)

#define alwaysfails 0