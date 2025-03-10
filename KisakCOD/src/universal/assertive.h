#pragma once

void MyAssertHandler(const char* filename, int line, int type, const char* fmt, ...);

#define iassert(expression) (void)(                                                       \
            (!!(expression)) ||                                                          \
            (MyAssertHandler(__FILE__, (unsigned)(__LINE__), 0, "%s", #expression), 0) \
        )



#define alwaysfails 0