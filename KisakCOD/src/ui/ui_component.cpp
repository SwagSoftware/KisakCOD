#include "ui.h"
#include <client_mp/client_mp.h>

void __cdecl UI_Component::InitAssets()
{
    memset(&UI_Component::g, 0, sizeof(UI_Component::g));
    UI_Component::g.screenWidth = cls.vidConfig.displayWidth;
    UI_Component::g.screenHeight = cls.vidConfig.displayHeight;
    UI_Component::g.charWidth = 12.0;
    UI_Component::g.charHeight = 16.0;
    UI_Component::g.scrollBarSize = 16.0;
    if (UI_Component::g.screenWidth == 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 59, 0, "%s", "g.screenWidth");
    if (UI_Component::g.screenHeight == 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 60, 0, "%s", "g.screenHeight");
    UI_Component::g.cursor = UI_Component::RegisterMaterialNoMip((char*)"ui_cursor", 1);
    UI_Component::g.filledCircle = UI_Component::RegisterMaterialNoMip((char*)"ui_sliderbutt_1", 1);
}

Material *__cdecl UI_Component::RegisterMaterialNoMip(char *name, int imageTrack)
{
    return Material_RegisterHandle(name, imageTrack);
}

void __cdecl UI_Component_Init()
{
	UI_Component::InitAssets();
}