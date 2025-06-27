#include "database.h"

#include <zlib/zlib.h>

// Note: On XBox it seems to use tomcrypt, and is more involved. 

int __cdecl DB_AuthLoad_InflateInit(z_stream_s *stream, bool isSecure)
{
    iassert(!isSecure);
    return inflateInit_(stream, "1.1.4", sizeof(z_stream));
}

void __cdecl DB_AuthLoad_InflateEnd(z_stream_s *stream)
{
    inflateEnd(stream);
}

unsigned int __cdecl DB_AuthLoad_Inflate(z_stream_s *stream, int flush)
{
    return inflate(stream, flush);
}

