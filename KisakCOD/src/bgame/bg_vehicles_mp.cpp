#include "bg_public.h"
#include "bg_local.h"

//unsigned short __cdecl BG_VehiclesGetSlotTagName(int) 8217e300 f   bg_vehicles_mp.obj
//int marker_bg_vehicles_mp 828006fc     bg_vehicles_mp.obj

unsigned __int16 __cdecl BG_VehiclesGetSlotTagName(int slotIndex)
{
    if (!slotIndex)
        return scr_const.tag_driver;
    if (slotIndex == 1)
        return scr_const.tag_passenger;
    if (slotIndex != 2)
        MyAssertHandler(".\\bgame\\bg_vehicles_mp.cpp", 24, 0, "%s", "slotIndex == VEHICLE_RIDESLOT_GUNNER");
    return scr_const.tag_gunner;
}