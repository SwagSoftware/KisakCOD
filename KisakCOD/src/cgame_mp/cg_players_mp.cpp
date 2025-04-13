#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <aim_assist/aim_assist.h>
#include <bgame/bg_local.h>

#include <client_mp/client_mp.h>

#include <EffectsCore/fx_system.h>

void __cdecl CG_AddAllPlayerSpriteDrawSurfs(int localClientNum)
{
    int entityIndex; // [esp+0h] [ebp-14h]
    centity_s *cent; // [esp+8h] [ebp-Ch]
    snapshot_s *nextSnap; // [esp+Ch] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    nextSnap = cgArray[0].nextSnap;
    for (entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex)
    {
        cent = CG_GetEntity(localClientNum, nextSnap->entities[entityIndex].number);
        if (cent->nextState.eType == 1)
            CG_AddPlayerSpriteDrawSurfs(localClientNum, cent);
    }
}

void __cdecl CG_AddPlayerSpriteDrawSurfs(int localClientNum, const centity_s *cent)
{
    team_t iClientTeam; // [esp+10h] [ebp-18h]
    int secondaryHeight; // [esp+14h] [ebp-14h]
    Material *hMaterial; // [esp+18h] [ebp-10h]
    team_t iTeam; // [esp+1Ch] [ebp-Ch]
    playerState_s *ps; // [esp+20h] [ebp-8h]
    char *pszIcon; // [esp+24h] [ebp-4h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cent->nextState.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            134,
            0,
            "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cent->nextState.clientNum,
            64);
    if (cgArray[0].bgs.clientinfo[cent->nextState.clientNum].infoValid)
    {
        if (cent->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_players_mp.cpp",
                137,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                64);
        iTeam = cgArray[0].bgs.clientinfo[cent->nextState.clientNum].team;
        ps = &cgArray[0].nextSnap->ps;
        if (cgArray[0].nextSnap->ps.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_players_mp.cpp",
                141,
                0,
                "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cgArray[0].nextSnap->ps.clientNum,
                64);
        if (cgArray[0].bgs.clientinfo[ps->clientNum].infoValid)
        {
            if (ps->clientNum >= 0x40u)
                MyAssertHandler(
                    ".\\cgame_mp\\cg_players_mp.cpp",
                    144,
                    0,
                    "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    ps->clientNum,
                    64);
            iClientTeam = cgArray[0].bgs.clientinfo[ps->clientNum].team;
            secondaryHeight = 0;
            if (cent->nextState.iHeadIcon
                && (!cent->nextState.iHeadIconTeam
                    || iClientTeam == TEAM_SPECTATOR
                    || cent->nextState.iHeadIconTeam == iClientTeam))
            {
                pszIcon = CL_GetConfigString(localClientNum, cent->nextState.iHeadIcon + 2266);
                hMaterial = Material_RegisterHandle(pszIcon, 7);
                if (hMaterial)
                {
                    CG_AddPlayerSpriteDrawSurf(
                        localClientNum,
                        cent,
                        hMaterial,
                        cg_scriptIconSize->current.value,
                        0,
                        cg_constantSizeHeadIcons->current.enabled);
                    secondaryHeight = (int)cg_scriptIconSize->current.value + 16;
                }
            }
            if (cent->nextState.number == cgArray[0].clientNum && cgArray[0].inKillCam)
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.youInKillCamMaterial,
                    cg_youInKillCamSize->current.value,
                    secondaryHeight,
                    1);
            }
            else if ((cent->nextState.lerp.eFlags & 0x80) != 0)
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.connectionMaterial,
                    cg_connectionIconSize->current.value,
                    secondaryHeight,
                    0);
            }
            else if ((iTeam == iClientTeam || iClientTeam == TEAM_SPECTATOR) && (cent->nextState.lerp.eFlags & 0x200000) != 0)
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.balloonMaterial,
                    cg_voiceIconSize->current.value,
                    secondaryHeight - 5,
                    0);
            }
        }
    }
}

void  CG_AddPlayerSpriteDrawSurf(
    int localClientNum,
    const centity_s *cent,
    Material *material,
    float additionalRadiusSize,
    int height,
    bool fixedScreenSize)
{
    double v7; // st7
    unsigned int v8[3]; // [esp+18h] [ebp-5Ch] BYREF
    FxSprite sprite; // [esp+24h] [ebp-50h] BYREF
    float v10; // [esp+44h] [ebp-30h]
    float v11; // [esp+48h] [ebp-2Ch]
    float origin[3]; // [esp+4Ch] [ebp-28h]
    DObj_s *obj; // [esp+58h] [ebp-1Ch]
    int radius; // [esp+5Ch] [ebp-18h]
    int renderFxFlags; // [esp+60h] [ebp-14h]
    int v16; // [esp+68h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+6Ch] [ebp-8h]
    const cg_s *retaddr; // [esp+74h] [ebp+0h]

    v16 = a1;
    cgameGlob = retaddr;
    if (additionalRadiusSize < 0.0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            74,
            0,
            "%s\n\t(additionalRadiusSize) = %g",
            "(additionalRadiusSize >= 0)",
            additionalRadiusSize);
    if (!cg_headIconMinScreenRadius)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 75, 0, "%s", "cg_headIconMinScreenRadius");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    renderFxFlags = (int)cgArray;
    radius = MEMORY[0x98F45C];
    obj = (DObj_s *)((*(unsigned int *)(MEMORY[0x98F45C] + 32) & 6) != 0 && cent->nextState.number == *(unsigned int *)(radius + 232));
    if (!(_BYTE)obj || *(unsigned int *)(renderFxFlags + 287032))
    {
        if (fixedScreenSize)
        {
            LODWORD(origin[2]) = 3;
            v7 = (additionalRadiusSize + 10.0) * 0.00430000014603138;
        }
        else
        {
            origin[2] = 0.0;
            v7 = additionalRadiusSize + 10.0;
        }
        origin[1] = v7;
        LODWORD(origin[0]) = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (LODWORD(origin[0])
            && CG_DObjGetWorldTagPos(&cent->pose, (DObj_s *)LODWORD(origin[0]), scr_const.j_head, (float *)&sprite.flags))
        {
            v11 = (double)height + 21.0 + v11;
        }
        else
        {
            LODWORD(sprite.minScreenRadius) = cent->pose.origin;
            *(float *)&sprite.flags = cent->pose.origin[0];
            v10 = cent->pose.origin[1];
            v11 = cent->pose.origin[2];
            v11 = (double)height + 82.0 + v11;
        }
        v8[1] = sprite.flags;
        *(float *)&v8[2] = v10;
        *(float *)&sprite.material = v11;
        sprite.pos[0] = NAN;
        v8[0] = material;
        sprite.pos[1] = origin[1];
        sprite.pos[2] = cg_headIconMinScreenRadius->current.value;
        *(float *)sprite.rgbaColor = origin[2];
        if (!cg_headIconMinScreenRadius)
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 113, 0, "%s", "cg_headIconMinScreenRadius");
        FX_SpriteAdd((FxSprite *)v8);
    }
}

void __cdecl CG_Player(int localClientNum, centity_s *cent)
{
    double v2; // st7
    bool v3; // [esp+4h] [ebp-50h]
    snapshot_s *nextSnap; // [esp+10h] [ebp-44h]
    GfxScaledPlacement placement; // [esp+14h] [ebp-40h] BYREF
    DObj_s *obj; // [esp+34h] [ebp-20h]
    cg_s *cgameGlob; // [esp+38h] [ebp-1Ch]
    int iClientNum; // [esp+3Ch] [ebp-18h]
    entityState_s *p_nextState; // [esp+40h] [ebp-14h]
    clientInfo_t *ci; // [esp+44h] [ebp-10h]
    float lightingOrigin[3]; // [esp+48h] [ebp-Ch] BYREF

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0 && (p_nextState->lerp.eFlags & 0x20000) == 0)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgameGlob = cgArray;
        nextSnap = cgArray[0].nextSnap;
        v3 = (nextSnap->ps.otherFlags & 6) != 0 && p_nextState->number == nextSnap->ps.clientNum;
        if (!v3 || cgameGlob->renderingThirdPerson)
        {
            iClientNum = p_nextState->clientNum;
            if (p_nextState->clientNum >= 0x40u)
                MyAssertHandler(
                    ".\\cgame_mp\\cg_players_mp.cpp",
                    459,
                    0,
                    "%s",
                    "(unsigned)es->clientNum < ARRAY_COUNT( cgameGlob->bgs.clientinfo )");
            obj = Com_GetClientDObj(p_nextState->number, localClientNum);
            if (obj)
            {
                if ((unsigned int)iClientNum >= 0x40)
                    MyAssertHandler(
                        ".\\cgame_mp\\cg_players_mp.cpp",
                        465,
                        0,
                        "iClientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        iClientNum,
                        64);
                ci = &cgameGlob->bgs.clientinfo[iClientNum];
                BG_PlayerAnimation(localClientNum, p_nextState, ci);
                if ((p_nextState->lerp.eFlags & 0x300) != 0)
                    CG_PlayerTurretPositionAndBlend(localClientNum, cent);
                CG_Player_PreControllers(obj, cent);
                CL_ResetStats_f();
                CG_UpdateWeaponVisibility(localClientNum, cent);
                lightingOrigin[0] = cent->pose.origin[0];
                lightingOrigin[1] = cent->pose.origin[1];
                lightingOrigin[2] = cent->pose.origin[2];
                if ((p_nextState->lerp.eFlags & 8) != 0)
                {
                    lightingOrigin[2] = lightingOrigin[2] + 12.0;
                }
                else
                {
                    if ((p_nextState->lerp.eFlags & 4) != 0)
                        v2 = lightingOrigin[2] + 20.0;
                    else
                        v2 = lightingOrigin[2] + 32.0;
                    lightingOrigin[2] = v2;
                }
                R_AddDObjToScene(obj, &cent->pose, p_nextState->number, 4u, lightingOrigin, 0.0);
                if ((p_nextState->lerp.eFlags & 0x20000) == 0)
                {
                    placement.base.origin[0] = cent->pose.origin[0];
                    placement.base.origin[1] = cent->pose.origin[1];
                    placement.base.origin[2] = cent->pose.origin[2];
                    AnglesToQuat(cent->pose.angles, placement.base.quat);
                    placement.scale = 1.0;
                    CG_AddPlayerWeapon(localClientNum, &placement, 0, cent, 1);
                    if (cent->nextValid && cgameGlob->predictedPlayerState.clientNum != cent->nextState.number)
                        AimTarget_ProcessEntity(localClientNum, cent);
                }
            }
        }
    }
}
void __cdecl CG_PlayerTurretPositionAndBlend(int localClientNum, centity_s *cent)
{
    char *AnimDebugName; // eax
    char *v3; // eax
    int v4; // eax
    double v5; // st7
    float v6; // [esp+18h] [ebp-1ACh]
    float v7; // [esp+1Ch] [ebp-1A8h]
    float goalWeight; // [esp+20h] [ebp-1A4h]
    float fHeightRatio; // [esp+4Ch] [ebp-178h]
    unsigned int iPrevBlend; // [esp+50h] [ebp-174h]
    float fPrevTransZ; // [esp+54h] [ebp-170h]
    DObj_s *obj; // [esp+58h] [ebp-16Ch]
    int numVertChildren; // [esp+5Ch] [ebp-168h]
    DObj_s *turretObj; // [esp+60h] [ebp-164h]
    float trans2[3]; // [esp+68h] [ebp-15Ch] BYREF
    float yaw; // [esp+74h] [ebp-150h]
    float trans[3]; // [esp+78h] [ebp-14Ch] BYREF
    float tagOrigin[3]; // [esp+84h] [ebp-140h] BYREF
    float start[3]; // [esp+90h] [ebp-134h] BYREF
    float end[3]; // [esp+9Ch] [ebp-128h] BYREF
    int iBlend; // [esp+A8h] [ebp-11Ch]
    unsigned int heightAnim; // [esp+ACh] [ebp-118h]
    float fDelta; // [esp+B0h] [ebp-114h]
    float fPrevBlend; // [esp+B4h] [ebp-110h]
    float rot[2]; // [esp+B8h] [ebp-10Ch] BYREF
    float tagAxis[3][3]; // [esp+C0h] [ebp-104h] BYREF
    unsigned int leafAnim1; // [esp+E4h] [ebp-E0h]
    trace_t trace; // [esp+E8h] [ebp-DCh] BYREF
    float endpos[3]; // [esp+114h] [ebp-B0h] BYREF
    int numHorChildren; // [esp+120h] [ebp-A4h]
    const clientInfo_t *ci; // [esp+124h] [ebp-A0h]
    float tagHeight; // [esp+128h] [ebp-9Ch]
    int i; // [esp+12Ch] [ebp-98h]
    unsigned int baseAnim; // [esp+130h] [ebp-94h]
    int clientNum; // [esp+134h] [ebp-90h]
    const centity_s *pTurretCEnt; // [esp+138h] [ebp-8Ch]
    const lerpFrame_t *pLerpAnim; // [esp+13Ch] [ebp-88h]
    const WeaponDef *weapDef; // [esp+140h] [ebp-84h]
    float fBlend; // [esp+144h] [ebp-80h]
    float axis[4][3]; // [esp+148h] [ebp-7Ch] BYREF
    XAnimTree_s *pAnimTree; // [esp+178h] [ebp-4Ch]
    XAnim_s *pXAnims; // [esp+17Ch] [ebp-48h]
    unsigned int leafAnim2; // [esp+180h] [ebp-44h]
    float localYaw; // [esp+184h] [ebp-40h]
    float turretAxis[4][3]; // [esp+188h] [ebp-3Ch] BYREF
    float vDelta[3]; // [esp+1B8h] [ebp-Ch] BYREF

    if (cent->nextState.otherEntityNum >= 64 && cent->nextState.otherEntityNum != 1023)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        clientNum = cent->nextState.clientNum;
        if ((unsigned int)clientNum >= 0x40)
            MyAssertHandler(
                ".\\cgame_mp\\cg_players_mp.cpp",
                273,
                0,
                "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                clientNum,
                64);
        ci = &cgArray[0].bgs.clientinfo[clientNum];
        if (ci->infoValid)
        {
            pLerpAnim = &ci->legs;
            if (ci->legs.animationNumber)
            {
                if (pLerpAnim->animation)
                {
                    if ((pLerpAnim->animation->flags & 4) != 0)
                    {
                        pTurretCEnt = CG_GetEntity(localClientNum, cent->nextState.otherEntityNum);
                        if (pTurretCEnt->nextValid)
                        {
                            turretObj = Com_GetClientDObj(pTurretCEnt->nextState.number, localClientNum);
                            if (turretObj)
                            {
                                obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                                if (obj)
                                {
                                    if (CG_DObjGetWorldTagMatrix(&pTurretCEnt->pose, turretObj, scr_const.tag_weapon, tagAxis, tagOrigin))
                                    {
                                        if (MEMORY[0x9D555C])
                                        {
                                            if (!pTurretCEnt->nextState.weapon)
                                                MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 304, 0, "%s", "pTurretCEnt->nextState.weapon");
                                            weapDef = BG_GetWeaponDef(pTurretCEnt->nextState.weapon);
                                            if (weapDef->weapClass != WEAPCLASS_TURRET)
                                                MyAssertHandler(
                                                    ".\\cgame_mp\\cg_players_mp.cpp",
                                                    306,
                                                    0,
                                                    "%s",
                                                    "weapDef->weapClass == WEAPCLASS_TURRET");
                                            if (weapDef->fAnimHorRotateInc == 0.0)
                                                MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 307, 0, "%s", "weapDef->fAnimHorRotateInc");
                                            pAnimTree = ci->pXAnimTree;
                                            pXAnims = (XAnim_s *)MEMORY[0xA7B0B0];
                                            baseAnim = pLerpAnim->animationNumber & 0xFFFFFDFF;
                                            yaw = vectosignedyaw(tagAxis[0]);
                                            localYaw = AngleDelta(yaw, pTurretCEnt->pose.angles[1]);
                                            AnglesToAxis(pTurretCEnt->pose.angles, turretAxis);
                                            turretAxis[3][0] = pTurretCEnt->pose.origin[0];
                                            turretAxis[3][1] = pTurretCEnt->pose.origin[1];
                                            turretAxis[3][2] = pTurretCEnt->pose.origin[2];
                                            Vec3Sub(cent->pose.origin, turretAxis[3], vDelta);
                                            tagHeight = Vec3Dot(vDelta, turretAxis[2]);
                                            fDelta = tagHeight - (tagOrigin[2] - turretAxis[3][2]);
                                            numVertChildren = XAnimGetNumChildren(pXAnims, baseAnim);
                                            fPrevTransZ = 0.0;
                                            fPrevBlend = 0.0;
                                            iPrevBlend = 0;
                                            leafAnim2 = 0;
                                            if (!numVertChildren)
                                            {
                                                AnimDebugName = XAnimGetAnimDebugName(pXAnims, baseAnim);
                                                Com_Error(ERR_DROP, &byte_86CEEC, AnimDebugName);
                                            }
                                            i = 0;
                                            do
                                            {
                                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - 1 - i);
                                                numHorChildren = XAnimGetNumChildren(pXAnims, heightAnim);
                                                if (!numHorChildren)
                                                {
                                                    v3 = XAnimGetAnimDebugName(pXAnims, heightAnim);
                                                    Com_Error(ERR_DROP, &byte_86CEEC, v3);
                                                }
                                                fBlend = (double)numHorChildren * 0.5 + localYaw / weapDef->fAnimHorRotateInc;
                                                if (fBlend >= 0.0)
                                                {
                                                    if (fBlend >= (double)(numHorChildren - 1))
                                                        fBlend = (float)(numHorChildren - 1);
                                                }
                                                else
                                                {
                                                    fBlend = 0.0;
                                                }
                                                v4 = (int)fBlend;
                                                iBlend = v4;
                                                fBlend = fBlend - (double)v4;
                                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, v4);
                                                XAnimGetAbsDelta(pXAnims, leafAnim1, rot, trans, 0.0);
                                                if (fBlend != 0.0)
                                                {
                                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iBlend + 1);
                                                    XAnimGetAbsDelta(pXAnims, leafAnim2, rot, trans2, 0.0);
                                                    Vec3Lerp(trans, trans2, fBlend, trans);
                                                }
                                                if (fDelta <= (double)trans[2])
                                                    break;
                                                fPrevTransZ = trans[2];
                                                iPrevBlend = iBlend;
                                                fPrevBlend = fBlend;
                                                ++i;
                                            } while (i < numVertChildren);
                                            XAnimClearTreeGoalWeightsStrict(pAnimTree, baseAnim, 0.0);
                                            goalWeight = 1.0 - fBlend;
                                            XAnimSetGoalWeight(obj, leafAnim1, goalWeight, 0.0, 1.0, 0, 0, 0);
                                            if (fBlend != 0.0)
                                                XAnimSetGoalWeight(obj, leafAnim2, fBlend, 0.0, 1.0, 0, 0, 0);
                                            if (i && i != numVertChildren)
                                            {
                                                if (trans[2] - fPrevTransZ == 0.0)
                                                    MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 383, 0, "%s", "trans[2] - fPrevTransZ");
                                                fHeightRatio = (fDelta - fPrevTransZ) / (trans[2] - fPrevTransZ);
                                                XAnimSetGoalWeight(obj, heightAnim, fHeightRatio, 0.0, 1.0, 0, 0, 0);
                                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - i);
                                                v7 = 1.0 - fHeightRatio;
                                                XAnimSetGoalWeight(obj, heightAnim, v7, 0.0, 1.0, 0, 0, 0);
                                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend);
                                                v6 = 1.0 - fPrevBlend;
                                                XAnimSetGoalWeight(obj, leafAnim1, v6, 0.0, 1.0, 0, 0, 0);
                                                if (fPrevBlend != 0.0)
                                                {
                                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend + 1);
                                                    XAnimSetGoalWeight(obj, leafAnim2, fPrevBlend, 0.0, 1.0, 0, 0, 0);
                                                }
                                            }
                                            else
                                            {
                                                XAnimSetGoalWeight(obj, heightAnim, 1.0, 0.0, 1.0, 0, 0, 0);
                                            }
                                            XAnimCalcAbsDelta(obj, baseAnim, rot, trans);
                                            VectorAngleMultiply(trans, yaw);
                                            axis[3][0] = trans[0] + tagOrigin[0];
                                            axis[3][1] = trans[1] + tagOrigin[1];
                                            axis[3][2] = tagHeight + turretAxis[3][2];
                                            v5 = RotationToYaw(rot);
                                            yaw = v5 + yaw;
                                            YawToAxis(yaw, axis);
                                            AxisToAngles(axis, cent->pose.angles);
                                            cent->pose.origin[0] = axis[3][0];
                                            cent->pose.origin[1] = axis[3][1];
                                            cent->pose.origin[2] = axis[3][2];
                                            start[0] = cent->pose.origin[0];
                                            start[1] = cent->pose.origin[1];
                                            start[2] = cent->pose.origin[2];
                                            end[0] = cent->pose.origin[0];
                                            end[1] = cent->pose.origin[1];
                                            end[2] = cent->pose.origin[2];
                                            start[2] = pTurretCEnt->pose.origin[2];
                                            CG_TraceCapsule(
                                                &trace,
                                                start,
                                                (float *)vec3_origin,
                                                (float *)vec3_origin,
                                                end,
                                                cent->nextState.number,
                                                (int)&sv.svEntities[446].baseline.s.lerp.apos.trBase[2] + 1);
                                            if (trace.fraction < 1.0)
                                            {
                                                Vec3Lerp(start, end, trace.fraction, endpos);
                                                cent->pose.origin[2] = endpos[2];
                                            }
                                        }
                                    }
                                    else
                                    {
                                        Com_PrintWarning(
                                            17,
                                            "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_Corpse(int localClientNum, centity_s *cent)
{
    double v2; // st7
    DObj_s *obja; // [esp+8h] [ebp-12Ch]
    DObj_s *obj; // [esp+8h] [ebp-12Ch]
    entityState_s *p_nextState; // [esp+10h] [ebp-124h]
    unsigned int corpseIndex; // [esp+14h] [ebp-120h]
    clientInfo_t *ci; // [esp+18h] [ebp-11Ch]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+1Ch] [ebp-118h] BYREF
    float lightingOrigin[3]; // [esp+128h] [ebp-Ch] BYREF

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20000) == 0)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 526, 0, "%s", "es->lerp.eFlags & EF_DEAD");
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        corpseIndex = p_nextState->number - 64;
        if (corpseIndex >= 8)
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 533, 0, "%s", "(unsigned)corpseIndex < MAX_CLIENT_CORPSES");
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        ci = &cgsArray[0].corpseinfo[corpseIndex];
        obja = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, obja, 0, 0);
        BG_UpdatePlayerDObj(localClientNum, obja, p_nextState, ci, 0);
        obj = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, p_nextState->number, obj, 0, 0);
        if (obj)
        {
            BG_PlayerAnimation(localClientNum, p_nextState, ci);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            if ((cent->nextState.lerp.eFlags & 8) != 0)
            {
                lightingOrigin[2] = lightingOrigin[2] + 12.0;
            }
            else
            {
                if ((cent->nextState.lerp.eFlags & 4) != 0)
                    v2 = lightingOrigin[2] + 20.0;
                else
                    v2 = lightingOrigin[2] + 32.0;
                lightingOrigin[2] = v2;
            }
            R_AddDObjToScene(obj, &cent->pose, p_nextState->number, 0, lightingOrigin, 0.0);
        }
    }
}

void __cdecl CG_UpdatePlayerDObj(int localClientNum, centity_s *cent)
{
    DObj_s *dobj; // [esp+0h] [ebp-114h]
    DObj_s *dobja; // [esp+0h] [ebp-114h]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+Ch] [ebp-108h] BYREF

    if (cent->nextValid)
    {
        if (cent->nextState.clientNum != cent->nextState.number)
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 579, 0, "%s", "es->clientNum == es->number");
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (cent->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_players_mp.cpp",
                581,
                0,
                "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                64);
        dobj = Com_GetClientDObj(cent->nextState.clientNum, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, dobj, 0, 0);
        BG_UpdatePlayerDObj(
            localClientNum,
            dobj,
            &cent->nextState,
            &cgArray[0].bgs.clientinfo[cent->nextState.clientNum],
            0);
        dobja = Com_GetClientDObj(cent->nextState.clientNum, localClientNum);
        FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, cent->nextState.clientNum, dobja, 0, 0);
    }
}

void __cdecl CG_ResetPlayerEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent, int resetAnimation)
{
    XAnimTree_s *pAnimTree; // [esp+18h] [ebp-10h]
    DObj_s *obj; // [esp+1Ch] [ebp-Ch]
    clientInfo_t *ci; // [esp+24h] [ebp-4h]

    if (cent->nextState.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            605,
            0,
            "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cent->nextState.clientNum,
            64);
    ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
    if ((cent->nextState.lerp.eFlags & 0x20000) == 0 && resetAnimation)
    {
        pAnimTree = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].pXAnimTree;
        if (!pAnimTree)
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 613, 0, "%s", "pAnimTree");
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj)
        {
            if (!DObjGetTree(obj))
                MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 618, 0, "%s", "DObjGetTree( obj )");
            if (DObjGetTree(obj) != pAnimTree)
                MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 619, 0, "%s", "DObjGetTree( obj ) == pAnimTree");
            XAnimClearTreeGoalWeights(pAnimTree, 0, 0.0);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animScriptData.torsoAnim, 0.0, 0.0, 1.0, 0, 0, 0);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animScriptData.legsAnim, 1.0, 0.0, 1.0, 0, 0, 0);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animScriptData.turningAnim, 0.0, 0.0, 1.0, 0, 0, 0);
            memset((unsigned __int8 *)&ci->legs, 0, sizeof(ci->legs));
            ci->legs.yawAngle = ci->playerAngles[1];
            ci->legs.yawing = 0;
            ci->legs.pitchAngle = 0.0;
            ci->legs.pitching = 0;
            memset((unsigned __int8 *)&ci->torso, 0, sizeof(ci->torso));
            ci->torso.yawAngle = ci->playerAngles[1];
            ci->torso.yawing = 0;
            ci->torso.pitchAngle = ci->playerAngles[0];
            ci->torso.pitching = 0;
        }
    }
    if (cg_debugPosition->current.enabled)
        Com_Printf(
            17,
            "%i ResetPlayerEntity yaw=%i\n",
            cent->nextState.number,
            (unsigned int)COERCE_UNSIGNED_INT64(ci->torso.yawAngle));
}

const char *__cdecl CG_GetTeamName(team_t team)
{
    const char *result; // eax
    const char *v2; // eax

    switch (team)
    {
    case TEAM_FREE:
        result = "TEAM_FREE";
        break;
    case TEAM_AXIS:
        result = "TEAM_AXIS";
        break;
    case TEAM_ALLIES:
        result = "TEAM_ALLIES";
        break;
    case TEAM_SPECTATOR:
        result = "TEAM_SPECTATOR";
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("Unhandled team index %i!", team);
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 664, 0, v2);
        }
        result = "";
        break;
    }
    return result;
}

const char *__cdecl CG_GetOpposingTeamName(team_t team)
{
    const char *result; // eax
    const char *v2; // eax

    switch (team)
    {
    case TEAM_FREE:
    case TEAM_SPECTATOR:
        result = CG_GetTeamName(team);
        break;
    case TEAM_AXIS:
        result = CG_GetTeamName(TEAM_ALLIES);
        break;
    case TEAM_ALLIES:
        result = CG_GetTeamName(TEAM_AXIS);
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("Unhandled team index %i!", team);
            MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 682, 0, v2);
        }
        result = "";
        break;
    }
    return result;
}

const char *__cdecl CG_GetPlayerTeamName(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (MEMORY[0xE7A7CC][0] < 8)
        return CG_GetTeamName(TEAM_FREE);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            713,
            0,
            "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cgArray[0].clientNum,
            64);
    if (cgArray[0].bgs.clientinfo[cgArray[0].clientNum].infoValid)
        return CG_GetTeamName(cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team);
    else
        return CG_GetTeamName(TEAM_FREE);
}

const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (MEMORY[0xE7A7CC][0] < 8)
        return CG_GetOpposingTeamName(TEAM_FREE);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            750,
            0,
            "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cgArray[0].clientNum,
            64);
    if (cgArray[0].bgs.clientinfo[cgArray[0].clientNum].infoValid)
        return CG_GetOpposingTeamName(cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team);
    else
        return CG_GetOpposingTeamName(TEAM_FREE);
}

bool __cdecl CG_IsPlayerDead(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (client_state[0] < 8)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            772,
            0,
            "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cgArray[0].clientNum,
            64);
    if (!cgArray[0].bgs.clientinfo[cgArray[0].clientNum].infoValid)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 774, 0, "%s", "localPlayer->infoValid");
    return !cgArray[0].nextSnap->ps.stats[0]
        || (cgArray[0].nextSnap->ps.otherFlags & 4) == 0
        || (cgArray[0].nextSnap->ps.otherFlags & 2) != 0;
}

int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum)
{
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ps = &cgArray[0].nextSnap->ps;
    return ps->ammoclip[BG_ClipForWeapon(cgArray[0].nextSnap->ps.weapon)];
}

void __cdecl CG_UpdateWeaponVisibility(int localClientNum, centity_s *cent)
{
    bool IsKnifeMeleeAnim; // [esp+0h] [ebp-58h]
    unsigned __int8 boneIndex; // [esp+7h] [ebp-51h] BYREF
    DObj_s *obj; // [esp+8h] [ebp-50h]
    float origin[3]; // [esp+Ch] [ebp-4Ch] BYREF
    cg_s *cgameGlob; // [esp+18h] [ebp-40h]
    int addKnife; // [esp+1Ch] [ebp-3Ch]
    entityState_s *p_nextState; // [esp+20h] [ebp-38h]
    unsigned int boneHandle; // [esp+24h] [ebp-34h]
    clientInfo_t *ci; // [esp+28h] [ebp-30h]
    XModel *weapModel; // [esp+2Ch] [ebp-2Ch]
    float axis[3][3]; // [esp+30h] [ebp-28h] BYREF
    WeaponDef *weapDef; // [esp+54h] [ebp-4h]

    if (localClientNum >= 1)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1078, 0, "%s", "localClientNum < MAX_LOCAL_CLIENTS");
    if (!cent)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1079, 0, "%s", "cent");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    p_nextState = &cent->nextState;
    if (cent->nextState.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_players_mp.cpp",
            1085,
            0,
            "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            p_nextState->clientNum,
            64);
    ci = &cgameGlob->bgs.clientinfo[p_nextState->clientNum];
    if (ci->iDObjWeapon)
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weapModel = weapDef->worldModel[ci->weaponModel];
        if (weapDef->worldKnifeModel)
            IsKnifeMeleeAnim = BG_IsKnifeMeleeAnim(ci, cent->nextState.torsoAnim);
        else
            IsKnifeMeleeAnim = 0;
        addKnife = IsKnifeMeleeAnim;
        if (!IsKnifeMeleeAnim || ci->usingKnife)
        {
            if (!addKnife && ci->usingKnife)
            {
                ci->usingKnife = 0;
                ci->dobjDirty = 1;
            }
        }
        else
        {
            ci->usingKnife = 1;
            ci->dobjDirty = 1;
        }
        if (weapModel && (useFastFile->current.enabled || !XModelBad(weapModel)))
        {
            boneHandle = CG_GetWeaponAttachBone(ci, weapDef->weapType);
            obj = Com_GetClientDObj(p_nextState->number, localClientNum);
            boneIndex = -2;
            if (DObjGetBoneIndex(obj, boneHandle, &boneIndex))
            {
                if (CG_DObjGetWorldBoneMatrix(&cent->pose, obj, boneIndex, axis, origin))
                {
                    if (CG_IsWeaponVisible(localClientNum, cent, weapModel, origin, axis[0]))
                    {
                        if (ci->hideWeapon)
                        {
                            ci->hideWeapon = 0;
                            ci->dobjDirty = 1;
                        }
                    }
                    else if (!ci->hideWeapon)
                    {
                        ci->hideWeapon = 1;
                        ci->dobjDirty = 1;
                    }
                }
            }
        }
    }
}

bool __cdecl CG_IsWeaponVisible(int localClientNum, centity_s *cent, XModel *weapModel, float *origin, float *forward)
{
    float stock[3]; // [esp+Ch] [ebp-58h] BYREF
    float end[3]; // [esp+1Ch] [ebp-48h] BYREF
    trace_t trace; // [esp+28h] [ebp-3Ch] BYREF
    float weapLen; // [esp+54h] [ebp-10h] BYREF
    float eye[3]; // [esp+58h] [ebp-Ch] BYREF

    if (!weapModel)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1030, 0, "%s", "weapModel");
    if (!cent)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1031, 0, "%s", "cent");
    CG_CalcWeaponVisTrace(weapModel, origin, forward, stock, end, &weapLen);
    CG_TraceCapsule(&trace, stock, vec3_origin, vec3_origin, end, cent->nextState.number, 4097);
    if (!cg_drawWVisDebug)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1039, 0, "%s", "cg_drawWVisDebug");
    if (cg_drawWVisDebug->current.enabled)
    {
        if (trace.fraction == 1.0)
            CG_DebugLine(stock, end, colorRed, 1, 0);
        else
            CG_DebugLine(stock, end, colorYellow, 1, 0);
    }
    if (weapLen - weapLen * trace.fraction <= 3.0)
        return 1;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    eye[0] = cgArray[0].refdef.vieworg[0];
    eye[1] = cgArray[0].refdef.vieworg[1];
    eye[2] = cgArray[0].refdef.vieworg[2];
    CG_TraceCapsule(&trace, eye, vec3_origin, vec3_origin, stock, cent->nextState.number, 4097);
    if (cg_drawWVisDebug->current.enabled)
    {
        if (trace.fraction == 1.0)
            CG_DebugLine(eye, stock, colorGreen, 1, 0);
        else
            CG_DebugLine(eye, stock, colorBlue, 1, 0);
    }
    return trace.fraction == 1.0;
}

void __cdecl CG_CalcWeaponVisTrace(
    XModel *weapModel,
    float *origin,
    float *forward,
    float *start,
    float *end,
    float *modelLen)
{
    float mins[3]; // [esp+Ch] [ebp-20h] BYREF
    float stockDist; // [esp+18h] [ebp-14h]
    float maxs[3]; // [esp+1Ch] [ebp-10h] BYREF
    const DObjAnimMat *baseMat; // [esp+28h] [ebp-4h]

    if (!weapModel)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1005, 0, "%s", "weapModel");
    if (!modelLen)
        MyAssertHandler(".\\cgame_mp\\cg_players_mp.cpp", 1006, 0, "%s", "modelLen");
    XModelGetBounds(weapModel, mins, maxs);
    baseMat = XModelGetBasePose(weapModel);
    stockDist = mins[0] - baseMat->trans[0];
    *modelLen = maxs[0] - mins[0];
    Vec3Mad(origin, stockDist, forward, start);
    Vec3Mad(start, *modelLen, forward, end);
}

