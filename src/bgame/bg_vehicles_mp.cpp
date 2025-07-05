#include "bg_public.h"
#include "bg_local.h"

#include <script/scr_const.h>

//unsigned short __cdecl BG_VehiclesGetSlotTagName(int) 8217e300 f   bg_vehicles_mp.obj
//int32_t marker_bg_vehicles_mp 828006fc     bg_vehicles_mp.obj

uint16 BG_VehiclesGetSlotTagName(int32_t slotIndex)
{
    if (!slotIndex)
        return scr_const.tag_driver;
    if (slotIndex == 1)
        return scr_const.tag_passenger;
    iassert(slotIndex == VEHICLE_RIDESLOT_GUNNER);
    return scr_const.tag_gunner;
}