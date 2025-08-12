#ifndef KISAK_SP
#error This File is SinglePlayer Only
#endif

#include "net_chan.h"
#include <universal/q_shared.h>


const char *NET_AdrToString(netadr_t a)
{
    static	char	s[64];

    Com_sprintf(s, sizeof(s), "unknown");

    if (a.type == NA_LOOPBACK) {
        Com_sprintf(s, sizeof(s), "loopback");
    }
    else if (a.type == NA_BOT) {
        Com_sprintf(s, sizeof(s), "bot");
    }
    else if (a.type == NA_IP) {
        Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i",
            a.ip[0], a.ip[1], a.ip[2], a.ip[3], BigShort(a.port));
    }
    else if (a.type == NA_BAD) {
        Com_sprintf(s, sizeof(s), "BAD");
    }
    // Remove ipx
    //else {
    //    Com_sprintf(s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%i",
    //        a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
    //        BigShort(a.port));
    //}

    return s;
}