#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_negotiation.h"

int __fastcall Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState)
{
    int result; // r3

    if (!Path_HasNegotiationNode(&pSelf->Path))
        return 0;
    Actor_SetOrientMode(pSelf, AI_ORIENT_DONT_CHANGE);
    result = 1;
    pSelf->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    pSelf->prevMoveDir[0] = 0.0;
    pSelf->prevMoveDir[1] = 0.0;
    return result;
}

int __fastcall Actor_Negotiation_Think(actor_s *pSelf)
{
    ai_animmode_t v2; // r28
    ai_traverse_mode_t eTraverseMode; // r4
    const char *v4; // r3
    const char *v5; // r3
    const char *v6; // r3
    pathnode_t *HasNegotiationNode; // r3

    v2 = AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY;
    eTraverseMode = pSelf->eTraverseMode;
    if (eTraverseMode == AI_TRAVERSE_GRAVITY)
    {
        v2 = AI_ANIM_USE_BOTH_DELTAS;
    }
    else if (eTraverseMode == AI_TRAVERSE_NOCLIP)
    {
        v2 = AI_ANIM_USE_BOTH_DELTAS_NOCLIP;
    }
    else if (eTraverseMode != AI_TRAVERSE_NOGRAVITY && !alwaysfails)
    {
        v4 = va("pSelf->eTraverseMode is unhandled %i", eTraverseMode);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_negotiation.cpp", 46, 0, v4);
    }
    if (!pSelf->AnimScriptSpecific.func)
    {
        v5 = SL_ConvertToString(pSelf->AnimScriptSpecific.name);
        v6 = va("Invalid negotiation script %s", v5);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_negotiation.cpp",
            51,
            0,
            "%s\n\t%s",
            "pSelf->AnimScriptSpecific.func",
            v6);
    }
    Actor_AnimSpecific(pSelf, &pSelf->AnimScriptSpecific, v2, 1);
    pSelf->pushable = 0;
    if (Actor_IsAnimScriptAlive(pSelf))
    {
        pSelf->pszDebugInfo = "negotiation";
        Actor_PreThink(pSelf);
        Actor_UpdateOriginAndAngles(pSelf);
        return 0;
    }
    else
    {
        HasNegotiationNode = (pathnode_t *)Path_HasNegotiationNode(&pSelf->Path);
        if ((_BYTE)HasNegotiationNode)
        {
            if (pSelf->Path.wNegotiationStartNode >= pSelf->Path.wPathLen)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_negotiation.cpp",
                    61,
                    0,
                    "%s",
                    "pSelf->Path.wNegotiationStartNode < pSelf->Path.wPathLen");
            if (pSelf->Physics.vHitNormal[7 * pSelf->Path.wNegotiationStartNode + 2] < 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_negotiation.cpp",
                    62,
                    0,
                    "%s",
                    "pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum >= 0");
            HasNegotiationNode = Path_ConvertIndexToNode(LODWORD(pSelf->Physics.vHitNormal[7
                * pSelf->Path.wNegotiationStartNode
                + 2]));
            pSelf->sentient->pNearestNode = HasNegotiationNode;
            pSelf->sentient->bNearestNodeBad = 0;
            pSelf->sentient->bNearestNodeValid = 1;
        }
        pSelf->Physics.vWishDelta[0] = 0.0;
        pSelf->Physics.vWishDelta[1] = 0.0;
        pSelf->Physics.vWishDelta[2] = 0.0;
        pSelf->Physics.vVelocity[0] = 0.0;
        pSelf->Physics.vVelocity[1] = 0.0;
        pSelf->Physics.vVelocity[2] = 0.0;
        YawVectors2D(pSelf->fDesiredBodyYaw, (float *)HasNegotiationNode, pSelf->prevMoveDir);
        Actor_RecalcPath(pSelf);
        Actor_PopState(pSelf);
        return 1;
    }
}

