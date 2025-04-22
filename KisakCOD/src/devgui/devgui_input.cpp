#include "devgui.h"
#include <qcommon/cmd.h>
#include <client/client.h>
#include <client_mp/client_mp.h>

int s_butMapsKey[11] = { 154, 155, 156, 157, 13, 27, 9, 32, 161, 162, 171 }; // idb

DevGuiInput s_input;


cmd_function_s DevGui_Toggle_VAR;
void __cdecl DevGui_InputInit()
{
    Cmd_AddCommandInternal("devgui", DevGui_Toggle, &DevGui_Toggle_VAR);
}

void __cdecl DevGui_InputShutdown()
{
    Cmd_RemoveCommand("devgui");
}

void DevGui_InputUpdateMouse()
{
    s_input.sliderScrollTime = 100.0;
    s_input.sliderScrollMaxTimeStep = 0.30000001;
}

char __cdecl DevGui_InputUpdate(int localClientNum, float deltaTime)
{
    int butIndex; // [esp+10h] [ebp-8h]

    DevGui_InputUpdateMouse();
    for (butIndex = 0; butIndex < 11; ++butIndex)
    {
        s_input.prevButtonDown[butIndex] = s_input.buttonDown[butIndex];
        s_input.buttonDown[butIndex] = Key_IsDown(localClientNum, s_butMapsKey[butIndex]) != 0;
    }
    DevGui_UpdateScrollInputs(localClientNum);
    DevGui_UpdateScrollStates(deltaTime, s_input.digitalStates, s_input.digitalAxis, s_input.digitalTimes);
    DevGui_UpdateMenuScroll(deltaTime);
    CL_ClearKeys(localClientNum);
    return 1;
}

void __cdecl DevGui_UpdateScrollInputs(int localClientNum)
{
    float v1; // [esp+0h] [ebp-38h]
    float v2; // [esp+4h] [ebp-34h]
    float v3; // [esp+8h] [ebp-30h]
    float v4; // [esp+Ch] [ebp-2Ch]
    float highScale; // [esp+10h] [ebp-28h]
    float right; // [esp+14h] [ebp-24h]
    float left; // [esp+18h] [ebp-20h]
    float lx; // [esp+1Ch] [ebp-1Ch]
    float lowScale; // [esp+20h] [ebp-18h]
    float up; // [esp+24h] [ebp-14h]
    float down; // [esp+28h] [ebp-10h]
    float ly; // [esp+30h] [ebp-8h]

    lx = 0.0;
    ly = 0.0;
    if (DevGui_IsButtonDown(INPUT_RIGHT))
        v4 = 1.0;
    else
        v4 = 0.0;
    right = v4;
    if (DevGui_IsButtonDown(INPUT_LEFT))
        v3 = 1.0;
    else
        v3 = 0.0;
    left = v3;
    if (DevGui_IsButtonDown(INPUT_UP))
        v2 = 1.0;
    else
        v2 = 0.0;
    up = v2;
    if (DevGui_IsButtonDown(INPUT_DOWN))
        v1 = 1.0;
    else
        v1 = 0.0;
    down = v1;
    if (Key_IsDown(localClientNum, 200))
    {
        lx = s_input.mousePos[0];
        ly = -s_input.mousePos[1];
    }
    s_input.mousePos[0] = 0.0;
    s_input.mousePos[1] = 0.0;
    if ((v4 != 0.0 || v3 != 0.0) && (v2 != 0.0 || v1 != 0.0))
    {
        right = 0.0;
        left = 0.0;
        up = 0.0;
        down = 0.0;
    }
    lowScale = (float)0.0 * 1.0 + 1.0;
    highScale = (float)0.0 * 4.0 + 1.0;
    s_input.scrollScale = lowScale * highScale;
    s_input.digitalAxis[0] = right - left;
    s_input.digitalAxis[1] = up - down;
    s_input.analogAxis[0] = lx;
    s_input.analogAxis[1] = ly;
}

void __cdecl DevGui_UpdateScrollStates(float deltaTime, DevGuiInputState *states, float *axis, float *times)
{
    float v4; // [esp+0h] [ebp-10h]
    DevGuiInputState v5; // [esp+4h] [ebp-Ch]
    int axisIndex; // [esp+Ch] [ebp-4h]

    for (axisIndex = 0; axisIndex < 2; ++axisIndex)
    {
        if (axis[axisIndex] == 0.0)
        {
            states[axisIndex] = SCROLL_NONE;
            times[axisIndex] = 0.0;
        }
        else
        {
            v5 = states[axisIndex];
            if (v5)
            {
                if (v5 == SCROLL_PRESSED)
                {
                    states[axisIndex] = SCROLL_STALLED;
                    times[axisIndex] = 0.0;
                }
                else if (v5 == SCROLL_STALLED)
                {
                    times[axisIndex] = times[axisIndex] + deltaTime;
                    if (times[axisIndex] > 0.25)
                    {
                        states[axisIndex] = SCROLL_HELD;
                        times[axisIndex] = 0.0;
                    }
                }
                else
                {
                    times[axisIndex] = times[axisIndex] + deltaTime;
                }
            }
            else
            {
                v4 = fabs(axis[axisIndex]);
                if (v4 > 0.4000000059604645)
                    states[axisIndex] = SCROLL_PRESSED;
            }
        }
    }
}

void __cdecl DevGui_UpdateMenuScroll(float deltaTime)
{
    float v1; // [esp+4h] [ebp-40h]
    float v2; // [esp+8h] [ebp-3Ch]
    float v3; // [esp+Ch] [ebp-38h]
    bool v4; // [esp+10h] [ebp-34h]
    bool v5; // [esp+14h] [ebp-30h]
    float v6; // [esp+18h] [ebp-2Ch]
    float v7; // [esp+1Ch] [ebp-28h]
    float adjustedAnalogAxis[2]; // [esp+30h] [ebp-14h]
    int axisIndex; // [esp+38h] [ebp-Ch]
    bool pressed; // [esp+3Eh] [ebp-6h]
    bool held; // [esp+3Fh] [ebp-5h]
    float axis; // [esp+40h] [ebp-4h]

    v7 = fabs(s_input.analogAxis[0]);
    v6 = fabs(s_input.analogAxis[1]);
    if (v6 >= (double)v7)
    {
        adjustedAnalogAxis[0] = 0.0;
        adjustedAnalogAxis[1] = s_input.analogAxis[1];
    }
    else
    {
        adjustedAnalogAxis[0] = s_input.analogAxis[0];
        adjustedAnalogAxis[1] = 0.0;
    }
    for (axisIndex = 0; axisIndex < 2; ++axisIndex)
    {
        s_input.menuScroll[axisIndex] = 0;
        v5 = s_input.digitalStates[axisIndex] == SCROLL_HELD || s_input.analogStates[axisIndex] == SCROLL_HELD;
        held = v5;
        v4 = s_input.digitalStates[axisIndex] == SCROLL_PRESSED || s_input.analogStates[axisIndex] == SCROLL_PRESSED;
        pressed = v4;
        axis = (s_input.digitalAxis[axisIndex] + adjustedAnalogAxis[axisIndex]) * s_input.scrollScale;
        if (held)
        {
            v3 = fabs(axis);
            for (s_input.menuScrollTime[axisIndex] = deltaTime * v3 + s_input.menuScrollTime[axisIndex];
                s_input.menuScrollTime[axisIndex] > 0.1000000014901161;
                s_input.menuScrollTime[axisIndex] = s_input.menuScrollTime[axisIndex] - 0.1000000014901161)
            {
                if (axis < 0.0)
                    v2 = -1.0;
                else
                    v2 = 1.0;
                s_input.menuScroll[axisIndex] += (int)v2;
            }
        }
        else if (pressed)
        {
            if (axis < 0.0)
                v1 = -1.0;
            else
                v1 = 1.0;
            s_input.menuScroll[axisIndex] += (int)v1;
            s_input.menuScrollTime[axisIndex] = 0.0;
        }
    }
}

void __cdecl DevGui_MouseEvent(int dx, int dy)
{
    s_input.mousePos[0] = (float)dx;
    s_input.mousePos[1] = (float)dy;
}

__int16 __cdecl DevGui_GetMenuScroll(DevGuiInputAxis axis)
{
    return s_input.menuScroll[axis];
}

int __cdecl DevGui_UpdateIntScroll(float deltaTime, int value, int min, int max, DevGuiInputAxis axis)
{
    float v6; // [esp+0h] [ebp-4Ch]
    float v7; // [esp+4h] [ebp-48h]
    float v8; // [esp+8h] [ebp-44h]
    float v9; // [esp+14h] [ebp-38h]
    int range; // [esp+40h] [ebp-Ch]
    float stepTime; // [esp+44h] [ebp-8h]
    float stepTimea; // [esp+44h] [ebp-8h]
    int scroll; // [esp+48h] [ebp-4h]
    int valuea; // [esp+58h] [ebp+Ch]

    range = max - min;
    if (max - min < 0)
        MyAssertHandler(".\\devgui\\devgui_input.cpp", 454, 0, "%s", "range >= 0");
    if ((unsigned int)axis >= SCROLL_AXIS_COUNT)
        MyAssertHandler(".\\devgui\\devgui_input.cpp", 455, 0, "%s", "axis >= SCROLL_XAXIS && axis < SCROLL_AXIS_COUNT");
    if (!range)
        return 0;
    scroll = 0;
    if (s_input.digitalStates[axis] == SCROLL_HELD)
    {
        stepTime = 0.1000000014901161 / s_input.scrollScale;
        if (s_input.sliderScrollMaxTimeStep < (double)stepTime)
            stepTime = s_input.sliderScrollMaxTimeStep;
        for (s_input.digitalSliderTime = s_input.digitalSliderTime + deltaTime;
            stepTime < (double)s_input.digitalSliderTime;
            s_input.digitalSliderTime = s_input.digitalSliderTime - stepTime)
        {
            scroll += (int)(s_input.digitalAxis[axis] + 9.313225746154785e-10);
        }
    }
    else if (s_input.digitalStates[axis] == SCROLL_PRESSED)
    {
        scroll = (int)(s_input.digitalAxis[axis] + 9.313225746154785e-10);
    }
    else
    {
        s_input.digitalSliderTime = 0.0;
    }
    if (s_input.analogAxis[axis] == 0.0)
    {
        s_input.analogSliderTime = 0.0;
    }
    else
    {
        v9 = s_input.scrollScale * s_input.analogAxis[axis];
        v8 = fabs(v9);
        stepTimea = s_input.sliderScrollTime / (double)range / v8;
        if (s_input.sliderScrollMaxTimeStep < (double)stepTimea)
            stepTimea = s_input.sliderScrollMaxTimeStep;
        s_input.analogSliderTime = s_input.analogSliderTime + deltaTime;
        while (stepTimea <= (double)s_input.analogSliderTime)
        {
            if (s_input.analogAxis[axis] < 0.0)
                v7 = -1.0;
            else
                v7 = 1.0;
            scroll += (int)v7;
            s_input.analogSliderTime = s_input.analogSliderTime - stepTimea;
            v6 = fabs(s_input.analogAxis[axis]);
            if (v6 <= 2.0)
            {
                s_input.analogSliderTime = 0.0;
                break;
            }
        }
    }
    valuea = scroll + value;
    if (valuea <= min)
        return min;
    if (valuea < max)
        return valuea;
    return max;
}

double __cdecl DevGui_UpdateFloatScroll(
    float deltaTime,
    float value,
    float min,
    float max,
    float step,
    DevGuiInputAxis axis)
{
    float analog; // [esp+0h] [ebp-10h]
    float range; // [esp+4h] [ebp-Ch]
    float stepTime; // [esp+8h] [ebp-8h]
    float scroll; // [esp+Ch] [ebp-4h]
    float valuea; // [esp+1Ch] [ebp+Ch]

    range = max - min;
    if (range < 0.0)
        MyAssertHandler(".\\devgui\\devgui_input.cpp", 547, 0, "%s", "range >= 0");
    if (step == 0.0)
        MyAssertHandler(".\\devgui\\devgui_input.cpp", 548, 0, "%s", "step");
    if ((unsigned int)axis >= SCROLL_AXIS_COUNT)
        MyAssertHandler(".\\devgui\\devgui_input.cpp", 549, 0, "%s", "axis >= SCROLL_XAXIS && axis < SCROLL_AXIS_COUNT");
    analog = s_input.analogAxis[axis] * s_input.scrollScale / s_input.sliderScrollTime;
    scroll = analog * range * deltaTime;
    if (s_input.digitalStates[axis] == SCROLL_HELD)
    {
        stepTime = 0.1000000014901161 / s_input.scrollScale;
        if (s_input.sliderScrollMaxTimeStep < (double)stepTime)
            stepTime = s_input.sliderScrollMaxTimeStep;
        for (s_input.digitalSliderTime = s_input.digitalSliderTime + deltaTime;
            stepTime < (double)s_input.digitalSliderTime;
            s_input.digitalSliderTime = s_input.digitalSliderTime - stepTime)
        {
            scroll = s_input.digitalAxis[axis] * step + scroll;
        }
    }
    else if (s_input.digitalStates[axis] == SCROLL_PRESSED)
    {
        scroll = s_input.digitalAxis[axis] * step + scroll;
    }
    valuea = value + scroll;
    if (min >= (double)valuea)
        return min;
    if (max > (double)valuea)
        return valuea;
    return max;
}

bool __cdecl DevGui_IsButtonDown(DevGuiInputButton button)
{
    return s_input.buttonDown[button];
}

bool __cdecl DevGui_IsButtonPressed(DevGuiInputButton button)
{
    return s_input.buttonDown[button] && !s_input.prevButtonDown[button];
}

bool __cdecl DevGui_IsButtonReleased(DevGuiInputButton button)
{
    return !s_input.buttonDown[button] && s_input.prevButtonDown[button];
}

