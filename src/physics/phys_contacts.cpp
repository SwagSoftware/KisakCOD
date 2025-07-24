#include "phys_local.h"
#include <cgame/cg_local.h>

#include <physics/ode/collision_kernel.h>
#include <universal/profile.h>

int g_phys_msecStep[3] = { 0x11, 0x11, 0x11 };
int g_phys_maxMsecStep[3] = { 67, 67, 34 };
int g_phys_minMsecStep[3] = { 17, 17, 17 };

void __cdecl Phys_CheckOpposingNormals(dxBody *body0, dxBody *body1, ContactList *contacts)
{
    PhysObjUserData *userData; // [esp+0h] [ebp-Ch]
    PhysObjUserData *userDataa; // [esp+0h] [ebp-Ch]
    int contactIdx1; // [esp+4h] [ebp-8h]
    int contactIdx0; // [esp+8h] [ebp-4h]

    contactIdx0 = 0;
LABEL_2:
    if (contactIdx0 < contacts->contactCount - 1)
    {
        for (contactIdx1 = contactIdx0 + 1; ; ++contactIdx1)
        {
            if (contactIdx1 >= contacts->contactCount)
            {
                ++contactIdx0;
                goto LABEL_2;
            }
            if (Vec3Dot(contacts->contacts[contactIdx0].contact.normal, contacts->contacts[contactIdx1].contact.normal) < -0.9900000095367432)
                break;
        }
        if (body0)
        {
            userData = (PhysObjUserData *)dBodyGetData(body0);
            if (userData->debugContacts)
                Com_Printf(0, "Body0 flagged stuck due to normals %d and %d\n", contactIdx0, contactIdx1);
            if (!userData)
                MyAssertHandler(".\\physics\\phys_contacts.cpp", 388, 0, "%s", "userData");
            userData->state = PHYS_OBJ_STATE_STUCK;
        }
        if (body1)
        {
            userDataa = (PhysObjUserData *)dBodyGetData(body1);
            if (userDataa->debugContacts)
                Com_Printf(0, "Body1 flagged stuck due to normals %d and %d\n", contactIdx0, contactIdx1);
            if (!userDataa)
                MyAssertHandler(".\\physics\\phys_contacts.cpp", 398, 0, "%s", "userData");
            userDataa->state = PHYS_OBJ_STATE_STUCK;
        }
    }
}

void __cdecl Phys_ReduceContacts(dxBody *body, const ContactList *in, ContactList *out)
{
    int i; // [esp+74h] [ebp-228h]
    float centroid[3][3]; // [esp+78h] [ebp-224h] BYREF
    int group[128]; // [esp+9Ch] [ebp-200h] BYREF

    if (phys_drawcontacts->current.enabled)
    {
        for (i = 0; i < in->contactCount; ++i)
            Phys_DebugDrawContactPoint(
                in->contacts[i].contact.pos,
                in->contacts[i].contact.normal,
                in->contacts[i].contact.depth,
                colorYellow);
    }
    PROF_SCOPED("Phys_ReduceContacts");

    Phys_AssignInitialGroups(in, group);
    Phys_KMeans(in, centroid, group);
    if (physGlob.dumpContacts)
    {
        Phys_DumpGroups(centroid);
        Phys_DumpContacts(in, group);
    }
    Phys_MergeGroups(in, centroid, group);
    Phys_GenerateGroupContacts(body, in, centroid, group, out);
}

void __cdecl Phys_AssignInitialGroups(const ContactList *contacts, int *group)
{
    float v2; // [esp+0h] [ebp-48h]
    float v3; // [esp+4h] [ebp-44h]
    int initialPoint; // [esp+18h] [ebp-30h]
    float bestDot; // [esp+1Ch] [ebp-2Ch]
    float bestDota; // [esp+1Ch] [ebp-2Ch]
    int contactIter; // [esp+20h] [ebp-28h]
    int contactItera; // [esp+20h] [ebp-28h]
    float ptPos[2][3]; // [esp+24h] [ebp-24h] BYREF
    float dot; // [esp+3Ch] [ebp-Ch]
    int bestDotContact; // [esp+40h] [ebp-8h]
    float dot2; // [esp+44h] [ebp-4h]

    if (!contacts)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 45, 0, "%s", "contacts");
    if (contacts->contactCount <= 0)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 46, 0, "%s", "contacts->contactCount > 0");
    initialPoint = 104729 % contacts->contactCount;
    ptPos[0][0] = contacts->contacts[initialPoint].contact.normal[0];
    ptPos[0][1] = contacts->contacts[initialPoint].contact.normal[1];
    ptPos[0][2] = contacts->contacts[initialPoint].contact.normal[2];
    bestDot = FLT_MAX;
    bestDotContact = -1;
    for (contactIter = 0; contactIter != contacts->contactCount; ++contactIter)
    {
        group[contactIter] = -1;
        dot = Vec3Dot(ptPos[0], contacts->contacts[contactIter].contact.normal);
        if (bestDot > (double)dot)
        {
            bestDot = dot;
            bestDotContact = contactIter;
        }
    }
    if (bestDotContact == -1)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 67, 0, "%s", "bestDotContact != -1");
    group[initialPoint] = 0;
    group[bestDotContact] = 1;
    ptPos[1][0] = contacts->contacts[bestDotContact].contact.normal[0];
    ptPos[1][1] = contacts->contacts[bestDotContact].contact.normal[1];
    ptPos[1][2] = contacts->contacts[bestDotContact].contact.normal[2];
    bestDota = FLT_MAX;
    bestDotContact = -1;
    for (contactItera = 0; contactItera != contacts->contactCount; ++contactItera)
    {
        dot = Vec3Dot(ptPos[0], contacts->contacts[contactItera].contact.normal);
        dot2 = Vec3Dot(ptPos[1], contacts->contacts[contactItera].contact.normal);
        v3 = dot - dot2;
        if (v3 < 0.0)
            v2 = dot2;
        else
            v2 = dot;
        dot = v2;
        if (bestDota > (double)v2)
        {
            bestDota = dot;
            bestDotContact = contactItera;
        }
    }
    if (bestDotContact == -1)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 90, 0, "%s", "bestDotContact != -1");
    group[bestDotContact] = 2;
}

void __cdecl Phys_KMeans(const ContactList *contacts, float (*centroid)[3], int *group)
{
    float v3; // [esp+0h] [ebp-34h]
    float v4; // [esp+4h] [ebp-30h]
    int v5; // [esp+8h] [ebp-2Ch]
    float *v6; // [esp+18h] [ebp-1Ch]
    float dot; // [esp+1Ch] [ebp-18h]
    int bestGroup; // [esp+20h] [ebp-14h]
    float bestDot; // [esp+24h] [ebp-10h]
    int step; // [esp+28h] [ebp-Ch]
    int contactIter; // [esp+2Ch] [ebp-8h]
    int contactItera; // [esp+2Ch] [ebp-8h]
    int groupIter; // [esp+30h] [ebp-4h]
    int groupItera; // [esp+30h] [ebp-4h]
    int groupIterb; // [esp+30h] [ebp-4h]

    for (step = 0; ; ++step)
    {
        for (groupIter = 0; groupIter != 3; ++groupIter)
        {
            v6 = &(*centroid)[3 * groupIter];
            *v6 = 0.0;
            v6[1] = 0.0;
            v6[2] = 0.0;
        }
        for (contactIter = 0; contactIter != contacts->contactCount; ++contactIter)
        {
            if (group[contactIter] != -1)
                Vec3Add(
                    &(*centroid)[3 * group[contactIter]],
                    contacts->contacts[contactIter].contact.normal,
                    &(*centroid)[3 * group[contactIter]]);
        }
        for (groupItera = 0; groupItera != 3; ++groupItera)
            Vec3Normalize(&(*centroid)[3 * groupItera]);
        if (step == 5)
            break;
        for (contactItera = 0; contactItera != contacts->contactCount; ++contactItera)
        {
            bestGroup = -1;
            bestDot = -FLT_MAX;
            for (groupIterb = 0; groupIterb != 3; ++groupIterb)
            {
                dot = Vec3Dot(&(*centroid)[3 * groupIterb], contacts->contacts[contactItera].contact.normal);
                if (bestDot - dot < 0.0)
                    v5 = groupIterb;
                else
                    v5 = bestGroup;
                bestGroup = v5;
                v4 = bestDot - dot;
                if (v4 < 0.0)
                    v3 = dot;
                else
                    v3 = bestDot;
                bestDot = v3;
            }
            if (bestGroup == -1)
                MyAssertHandler(".\\physics\\phys_contacts.cpp", 141, 0, "%s", "bestGroup != -1");
            group[contactItera] = bestGroup;
        }
    }
}

void __cdecl Phys_DumpGroups(const float (*centroid)[3])
{
    int groupIter; // [esp+18h] [ebp-4h]

    for (groupIter = 0; groupIter != 3; ++groupIter)
        Com_Printf(
            0,
            "Group %i: N:%g %g %g\n",
            groupIter,
            (*centroid)[3 * groupIter],
            (*centroid)[3 * groupIter + 1],
            (*centroid)[3 * groupIter + 2]);
}

void __cdecl Phys_DumpContacts(const ContactList *contacts, const int *group)
{
    int contactIter; // [esp+3Ch] [ebp-4h]

    for (contactIter = 0; contactIter != contacts->contactCount; ++contactIter)
        Com_Printf(
            0,
            "Contact %i: G:%i, P:(%g %g %g), N:(%g %g %g), D:%g\n",
            contactIter,
            group[contactIter],
            contacts->contacts[contactIter].contact.pos[0],
            contacts->contacts[contactIter].contact.pos[1],
            contacts->contacts[contactIter].contact.pos[2],
            contacts->contacts[contactIter].contact.normal[0],
            contacts->contacts[contactIter].contact.normal[1],
            contacts->contacts[contactIter].contact.normal[2],
            contacts->contacts[contactIter].contact.depth);
}

void __cdecl Phys_MergeGroups(const ContactList *contacts, float (*centroid)[3], int *group)
{
    float dot; // [esp+0h] [ebp-10h]
    int groupIter2; // [esp+4h] [ebp-Ch]
    int groupIter; // [esp+8h] [ebp-8h]
    int contactIter; // [esp+Ch] [ebp-4h]

    for (groupIter = 0; groupIter != 2; ++groupIter)
    {
        for (groupIter2 = groupIter + 1; groupIter2 != 3; ++groupIter2)
        {
            dot = Vec3Dot(&(*centroid)[3 * groupIter], &(*centroid)[3 * groupIter2]);
            if (dot > 0.9990000128746033)
            {
                if (physGlob.dumpContacts)
                    Com_Printf(0, "Group %i merged into %i.\n", groupIter2, groupIter);
                for (contactIter = 0; contactIter != contacts->contactCount; ++contactIter)
                {
                    if (group[contactIter] == groupIter2)
                        group[contactIter] = groupIter;
                }
            }
        }
    }
}

void __cdecl Phys_GenerateGroupContacts(
    dxBody *body,
    const ContactList *inContacts,
    float (*centroid)[3],
    int *group,
    ContactList *outContacts)
{
    int v5; // [esp+8h] [ebp-98h]
    float v6; // [esp+Ch] [ebp-94h]
    float diff[3]; // [esp+20h] [ebp-80h] BYREF
    float *v8; // [esp+2Ch] [ebp-74h]
    float dot; // [esp+30h] [ebp-70h]
    float dist; // [esp+34h] [ebp-6Ch]
    int contactIter2; // [esp+38h] [ebp-68h]
    float MIN_DIST; // [esp+3Ch] [ebp-64h]
    float normal[3]; // [esp+40h] [ebp-60h] BYREF
    float maxD; // [esp+4Ch] [ebp-54h]
    int maxDContact; // [esp+50h] [ebp-50h]
    int maxContact[2]; // [esp+54h] [ebp-4Ch]
    float maxProjDist[2]; // [esp+5Ch] [ebp-44h]
    int pointCount; // [esp+64h] [ebp-3Ch]
    int minContact[2]; // [esp+68h] [ebp-38h]
    float uvBasis[2][3]; // [esp+70h] [ebp-30h] BYREF
    const dContactGeomExt *contact; // [esp+88h] [ebp-18h]
    float minProjDist[2]; // [esp+8Ch] [ebp-14h]
    int dimIter; // [esp+94h] [ebp-Ch]
    int groupIter; // [esp+98h] [ebp-8h]
    int contactIter; // [esp+9Ch] [ebp-4h]

    outContacts->contactCount = 0;
    for (groupIter = 0; groupIter != 3; ++groupIter)
    {
        v8 = &(*centroid)[3 * groupIter];
        *v8 = 0.0;
        v8[1] = 0.0;
        v8[2] = 0.0;
    }
    for (contactIter = 0; contactIter != inContacts->contactCount; ++contactIter)
    {
        if (group[contactIter] == -1)
            MyAssertHandler(".\\physics\\phys_contacts.cpp", 253, 0, "%s", "group[contactIter] != -1");
        Vec3Add(
            &(*centroid)[3 * group[contactIter]],
            inContacts->contacts[contactIter].contact.normal,
            &(*centroid)[3 * group[contactIter]]);
    }
    for (groupIter = 0; groupIter != 3; ++groupIter)
    {
        Vec3NormalizeTo(&(*centroid)[3 * groupIter], normal);
        Phys_CreateBasisFromNormal(normal, uvBasis[0], uvBasis[1]);
        for (dimIter = 0; dimIter != 2; ++dimIter)
        {
            minProjDist[dimIter] = FLT_MAX;
            minContact[dimIter] = -1;
            maxProjDist[dimIter] = -FLT_MAX;
            maxContact[dimIter] = -1;
        }
        maxDContact = -1;
        maxD = -FLT_MAX;
        pointCount = 0;
        for (contactIter = 0; contactIter != inContacts->contactCount; ++contactIter)
        {
            if (group[contactIter] == groupIter)
            {
                MIN_DIST = 0.1f;
                for (contactIter2 = contactIter + 1; contactIter2 != inContacts->contactCount; ++contactIter2)
                {
                    if (group[contactIter2] == groupIter)
                    {
                        Vec3Sub(inContacts->contacts[contactIter2].contact.pos, inContacts->contacts[contactIter].contact.pos, diff);
                        dist = Vec3LengthSq(diff);
                        v6 = MIN_DIST * MIN_DIST;
                        if (dist >= v6)
                            v5 = group[contactIter2];
                        else
                            v5 = -1;
                        group[contactIter2] = v5;
                    }
                }
                ++pointCount;
                contact = &inContacts->contacts[contactIter];
                for (dimIter = 0; dimIter != 2; ++dimIter)
                {
                    dot = Vec3Dot(contact->contact.pos, uvBasis[dimIter]);
                    if (minProjDist[dimIter] > dot)
                    {
                        minProjDist[dimIter] = dot;
                        minContact[dimIter] = contactIter;
                    }
                    if (maxProjDist[dimIter] < dot)
                    {
                        maxProjDist[dimIter] = dot;
                        maxContact[dimIter] = contactIter;
                    }
                }
                if (maxD < (float)contact->contact.depth)
                {
                    maxD = contact->contact.depth;
                    maxDContact = contactIter;
                }
            }
        }
        if (pointCount)
        {
            memcpy(
                &outContacts->contacts[outContacts->contactCount++],
                &inContacts->contacts[minContact[0]],
                sizeof(outContacts->contacts[outContacts->contactCount++]));
            if (minContact[1] != minContact[0])
                memcpy(
                    &outContacts->contacts[outContacts->contactCount++],
                    &inContacts->contacts[minContact[1]],
                    sizeof(outContacts->contacts[outContacts->contactCount++]));
            if (maxContact[0] != minContact[1] && maxContact[0] != minContact[0])
                memcpy(
                    &outContacts->contacts[outContacts->contactCount++],
                    &inContacts->contacts[maxContact[0]],
                    sizeof(outContacts->contacts[outContacts->contactCount++]));
            if (maxContact[1] != maxContact[0] && maxContact[1] != minContact[1] && maxContact[1] != minContact[0])
                memcpy(
                    &outContacts->contacts[outContacts->contactCount++],
                    &inContacts->contacts[maxContact[1]],
                    sizeof(outContacts->contacts[outContacts->contactCount++]));
            if (maxDContact != maxContact[1]
                && maxDContact != maxContact[0]
                && maxDContact != minContact[1]
                && maxDContact != minContact[0])
            {
                memcpy(
                    &outContacts->contacts[outContacts->contactCount++],
                    &inContacts->contacts[maxDContact],
                    sizeof(outContacts->contacts[outContacts->contactCount++]));
            }
        }
    }
}

void __cdecl Phys_CreateBasisFromNormal(const float *normal, float *binormal, float *tangent)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float independent[3]; // [esp+20h] [ebp-Ch] BYREF

    v3 = I_fabs(normal[2]);
    independent[0] = 0.0;
    if (v3 < 0.5)
    {
        independent[1] = 0.0;
        independent[2] = 1.0;
    }
    else
    {
        independent[1] = 1.0;
        independent[2] = 0.0;
    }
    Vec3Cross(normal, independent, binormal);
    Vec3Normalize(binormal);
    Vec3Cross(normal, binormal, tangent);
    Vec3Normalize(tangent);
}

void __cdecl Phys_DebugDrawContactPoint(const float *pos, const float *normal, float depth, const float *color)
{
    float scale; // [esp+10h] [ebp-28h]
    float mins[3]; // [esp+14h] [ebp-24h] BYREF
    float endpos[3]; // [esp+20h] [ebp-18h] BYREF
    float maxs[3]; // [esp+2Ch] [ebp-Ch] BYREF

    mins[0] = -1.0;
    mins[1] = -1.0;
    mins[2] = -1.0;
    maxs[0] = 1.0;
    maxs[1] = 1.0;
    maxs[2] = 1.0;
    CG_DebugBox(pos, mins, maxs, 0.0, color, 0, 3);
    scale = depth * 10.0;
    Vec3Mad(pos, scale, normal, endpos);
    CG_DebugLine(pos, endpos, color, 0, 3);
}

void __cdecl Phys_DumpContact(int contactNum, const dContactGeom *contact)
{
    Com_Printf(
        0,
        "Contact %i: P:(%g %g %g), N:(%g %g %g), D:%g\n",
        contactNum,
        contact->pos[0],
        contact->pos[1],
        contact->pos[2],
        contact->normal[0],
        contact->normal[1],
        contact->normal[2],
        contact->depth);
}

void __cdecl Phys_CreateJointForEachContact(
    ContactList *contactList,
    dxBody *body1,
    dxBody *body2,
    const dSurfaceParameters *surfParms,
    PhysWorld worldIndex)
{
    double pz; // [esp+18h] [ebp-64h]
    double pza; // [esp+18h] [ebp-64h]
    float *v7; // [esp+20h] [ebp-5Ch]
    float *Position; // [esp+24h] [ebp-58h]
    float *v9; // [esp+28h] [ebp-54h]
    float *contactCentroid; // [esp+2Ch] [ebp-50h]
    dContactGeom *contact; // [esp+30h] [ebp-4Ch]
    float pos[2][3]; // [esp+34h] [ebp-48h] BYREF
    PhysObjUserData *userData; // [esp+4Ch] [ebp-30h]
    dxJoint *joint; // [esp+50h] [ebp-2Ch]
    bool debug; // [esp+57h] [ebp-25h]
    dxBody *body; // [esp+58h] [ebp-24h]
    int bodyIndex; // [esp+5Ch] [ebp-20h]
    bool ignoreOpposingNormals[4]; // [esp+60h] [ebp-1Ch]
    int contactIter; // [esp+64h] [ebp-18h]
    bool useCentroids; // [esp+6Bh] [ebp-11h]
    float pointVel[4]; // [esp+6Ch] [ebp-10h] BYREF

    if ((unsigned int)worldIndex >= PHYS_WORLD_COUNT)
        MyAssertHandler(
            ".\\physics\\phys_contacts.cpp",
            591,
            0,
            "worldIndex doesn't index PHYS_WORLD_COUNT\n\t%i not in [0, %i)",
            worldIndex,
            3);
    Phys_CheckOpposingNormals(body1, body2, contactList);
    useCentroids = physGlob.worldData[worldIndex].useContactCentroids;
    debug = 0;
    bodyIndex = 0;
    body = body1;
    while (bodyIndex < 2)
    {
        ignoreOpposingNormals[bodyIndex] = 0;
        if (body)
        {
            userData = (PhysObjUserData *)dBodyGetData(body);
            if (!userData)
                MyAssertHandler(".\\physics\\phys_contacts.cpp", 607, 0, "%s", "userData");
            ignoreOpposingNormals[bodyIndex] = userData->state == PHYS_OBJ_STATE_STUCK;
            if (userData->debugContacts)
                debug = 1;
            if (useCentroids)
            {
                v9 = pos[bodyIndex];
                contactCentroid = userData->contactCentroid;
                *v9 = userData->contactCentroid[0];
                v9[1] = contactCentroid[1];
                v9[2] = contactCentroid[2];
            }
            else
            {
                v7 = pos[bodyIndex];
                Position = (float*)dBodyGetPosition(body);
                *v7 = *Position;
                v7[1] = Position[1];
                v7[2] = Position[2];
            }
        }
        ++bodyIndex;
        body = body2;
    }
    if (debug)
    {
        Com_Printf(0, "Pre Oppose Check:\n");
        for (contactIter = 0; contactIter != contactList->contactCount; ++contactIter)
            Phys_DumpContact(contactIter, &contactList->contacts[contactIter].contact);
    }
    if (ignoreOpposingNormals[0])
    {
        Phys_RemoveOpposingNormalContacts(pos[0], contactList);
    }
    else if (ignoreOpposingNormals[1])
    {
        Phys_RemoveOpposingNormalContacts(pos[1], contactList);
    }
    if (debug)
        Com_Printf(0, "Final Contacts:\n");
    for (contactIter = 0; contactIter != contactList->contactCount; ++contactIter)
    {
        contact = &contactList->contacts[contactIter].contact;
        if (Vec3Dot(contact->normal, contact->normal) <= 0.000001)
            MyAssertHandler(
                ".\\physics\\phys_contacts.cpp",
                646,
                0,
                "%s",
                "dDOT( contact->normal, contact->normal ) > ZERO_EPSILON");
        if (ignoreOpposingNormals[0]
            && (dBodyGetPointVel(body1, contact->pos[0], contact->pos[1], contact->pos[2], pointVel),
                dNormalize3(pointVel),
                Vec3Dot(pointVel, contact->normal) > 0.009999999776482582))
        {
            if (debug)
            {
                pz = Vec3Dot(pointVel, contact->normal);
                Com_Printf(
                    0,
                    "Killing contact %d, pointvel %f %f %f dot %f\n",
                    contactIter,
                    pointVel[0],
                    pointVel[1],
                    pointVel[2],
                    pz);
            }
        }
        else if (ignoreOpposingNormals[1]
            && (dBodyGetPointVel(body2, contact->pos[0], contact->pos[1], contact->pos[2], pointVel),
                dNormalize3(pointVel),
                Vec3Dot(pointVel, contact->normal) > 0.009999999776482582))
        {
            if (debug)
            {
                pza = Vec3Dot(pointVel, contact->normal);
                Com_Printf(
                    0,
                    "Killing contact %d, pointvel %f %f %f dot %f\n",
                    contactIter,
                    pointVel[0],
                    pointVel[1],
                    pointVel[2],
                    pza);
            }
        }
        else
        {
            Phys_ApplyContactJitter(worldIndex, contact, body1, body2);
            if (physGlob.dumpContacts || debug)
                Phys_DumpContact(contactIter, contact);
            if (phys_noIslands->current.enabled && body1 && body2)
            {
                joint = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], surfParms, contact);
                if (!joint)
                    goto LABEL_55;
                if (debug)
                    joint->debug = 1;
                dJointAttach(joint, body1, 0);
                joint = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], surfParms, contact);
                if (!joint)
                {
                LABEL_55:
                    Com_PrintWarning(21, "Maximum number of ODE physics contact points exceeded.\n");
                    return;
                }
                if (debug)
                    joint->debug = 1;
                dJointAttach(joint, 0, body2);
            }
            else
            {
                joint = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], surfParms, contact);
                if (!joint)
                    goto LABEL_55;
                if (debug)
                    joint->debug = 1;
                dJointAttach(joint, body1, body2);
            }
            if (phys_drawcontacts->current.enabled)
                Phys_DebugDrawContactPoint(contact->pos, contact->normal, contact->depth, colorRed);
        }
    }
}

void __cdecl Phys_ApplyContactJitter(PhysWorld worldIndex, dContactGeom *contact, dxBody *body1, dxBody *body2)
{
    double v4; // [esp+8h] [ebp-6Ch]
    float v5; // [esp+10h] [ebp-64h]
    float v6; // [esp+14h] [ebp-60h]
    float v7; // [esp+18h] [ebp-5Ch]
    dxBody *v8; // [esp+1Ch] [ebp-58h]
    float v9; // [esp+20h] [ebp-54h]
    float value; // [esp+24h] [ebp-50h]
    PhysWorldData *worldData; // [esp+28h] [ebp-4Ch]
    float delta[3]; // [esp+2Ch] [ebp-48h] BYREF
    Jitter *jitter; // [esp+38h] [ebp-3Ch]
    float displacement; // [esp+3Ch] [ebp-38h]
    float distanceScale; // [esp+40h] [ebp-34h]
    float massScale; // [esp+44h] [ebp-30h]
    dxBody *body; // [esp+48h] [ebp-2Ch]
    float force[3]; // [esp+4Ch] [ebp-28h] BYREF
    float distance; // [esp+58h] [ebp-1Ch]
    int jitterIdx; // [esp+5Ch] [ebp-18h]
    float time; // [esp+60h] [ebp-14h]
    float mass; // [esp+64h] [ebp-10h]
    float upVec[3]; // [esp+68h] [ebp-Ch] BYREF

    if ((!body1 || !body2) && contact->normal[2] >= 0.699999988079071)
    {
        if (body1)
            v8 = body1;
        else
            v8 = body2;
        body = v8;
        worldData = &physGlob.worldData[worldIndex];
        upVec[0] = 0.0;
        upVec[1] = 0.0;
        upVec[2] = 1.0;
        v9 = v8->mass.mass;
        value = phys_jitterMaxMass->current.value;
        v7 = value - v9;
        if (v7 < 0.0)
            v6 = value;
        else
            v6 = v9;
        mass = v6;
        massScale = v6 / phys_jitterMaxMass->current.value;
        massScale = (1.0 - massScale * massScale * massScale) * v6;
        time = 1000.0 / (double)g_phys_msecStep[worldIndex];
        for (jitterIdx = 0; jitterIdx < worldData->numJitterRegions; ++jitterIdx)
        {
            jitter = &worldData->jitterRegions[jitterIdx];
            Vec3Sub(contact->pos, jitter->origin, delta);
            delta[2] = 0.0;
            distance = Vec3LengthSq(delta);
            if (jitter->outerRadiusSq > (double)distance)
            {
                v5 = sqrt(distance);
                distance = v5;
                distanceScale = 1.0 / (jitter->outerRadius - jitter->innerRadius);
                if (jitter->innerRadius >= (double)v5)
                    distanceScale = 1.0;
                else
                    distanceScale = (jitter->outerRadius - distance) * distanceScale;
                v4 = jitter->maxDisplacement - jitter->minDisplacement;
                displacement = random() * v4 + jitter->minDisplacement;
                displacement = displacement * distanceScale * massScale * time;
                Vec3Scale(upVec, displacement, force);
                Phys_ObjAddForce(worldIndex, body, contact->pos, force);
            }
        }
    }
}

void __cdecl Phys_RemoveOpposingNormalContacts(const float *com, ContactList *contacts)
{
    float c0PlaneDist; // [esp+14h] [ebp-2Ch]
    int contact1Idx; // [esp+18h] [ebp-28h]
    dContactGeomExt *contact1; // [esp+1Ch] [ebp-24h]
    int contact0Idx; // [esp+20h] [ebp-20h]
    dContactGeomExt *contact0; // [esp+24h] [ebp-1Ch]
    float normal1[3]; // [esp+28h] [ebp-18h] BYREF
    float normal0[3]; // [esp+34h] [ebp-Ch] BYREF

    for (contact0Idx = 0; contact0Idx < contacts->contactCount - 1; ++contact0Idx)
    {
        contact0 = &contacts->contacts[contact0Idx];
        normal0[0] = contact0->contact.normal[0];
        normal0[1] = contact0->contact.normal[1];
        normal0[2] = contact0->contact.normal[2];
        for (contact1Idx = contact0Idx + 1; contact1Idx < contacts->contactCount; ++contact1Idx)
        {
            contact1 = &contacts->contacts[contact1Idx];
            normal1[0] = contact1->contact.normal[0];
            normal1[1] = contact1->contact.normal[1];
            normal1[2] = contact1->contact.normal[2];
            if (Vec3Dot(normal0, normal1) < -0.9900000095367432)
            {
                c0PlaneDist = Vec3Dot(contact0->contact.normal, contact0->contact.pos);
                if (Vec3Dot(normal0, com) - c0PlaneDist < 0.0)
                {
                    memcpy(contact0, contact1, sizeof(dContactGeomExt));
                    normal0[0] = contact0->contact.normal[0];
                    normal0[1] = contact0->contact.normal[1];
                    normal0[2] = contact0->contact.normal[2];
                }
                memcpy(contact1, &contacts->contacts[--contacts->contactCount], sizeof(dContactGeomExt));
            }
        }
    }
}

void __cdecl Phys_AddCollisionContact(PhysWorld worldId, const PhysContact *physContact, dxBody *obj0, dxBody *obj1)
{
    dxGeom *v4; // [esp+0h] [ebp-1860h]
    dxGeom *FirstGeom; // [esp+4h] [ebp-185Ch]
    float *normal; // [esp+8h] [ebp-1858h]
    ContactList contactList; // [esp+20h] [ebp-1840h] BYREF
    dSurfaceParameters surfParms; // [esp+182Ch] [ebp-34h] BYREF
    dxGeom *v9; // [esp+1858h] [ebp-8h]
    ContactList *p_contactList; // [esp+185Ch] [ebp-4h]

    if (obj0)
        FirstGeom = ODE_BodyGetFirstGeom(obj0);
    else
        FirstGeom = 0;
    v9 = FirstGeom;
    if (obj1)
        v4 = ODE_BodyGetFirstGeom(obj1);
    else
        v4 = 0;
    surfParms.mode = 12316;
    if (!phys_contact_cfm)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 767, 0, "%s", "phys_contact_cfm");
    if (!phys_contact_erp)
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 768, 0, "%s", "phys_contact_erp");
    surfParms.soft_cfm = phys_contact_cfm->current.value;
    surfParms.soft_erp = phys_contact_erp->current.value;
    surfParms.mu = physContact->friction;
    surfParms.mu2 = 0.0f;
    surfParms.bounce = physContact->bounce;
    surfParms.bounce_vel = 0.1f;
    contactList.contactCount = 1;
    p_contactList = &contactList;
    contactList.contacts[0].contact.depth = physContact->depth;
    if (!Vec3IsNormalized(physContact->normal))
        MyAssertHandler(".\\physics\\phys_contacts.cpp", 781, 0, "%s", "Vec3IsNormalized( physContact->normal )");
    normal = p_contactList->contacts[0].contact.normal;
    p_contactList->contacts[0].contact.normal[0] = physContact->normal[0];
    normal[1] = physContact->normal[1];
    normal[2] = physContact->normal[2];
    normal[3] = 0.0f;
    p_contactList->contacts[0].contact.pos[0] = physContact->pos[0];
    p_contactList->contacts[0].contact.pos[1] = physContact->pos[1];
    p_contactList->contacts[0].contact.pos[2] = physContact->pos[2];
    p_contactList->contacts[0].contact.pos[3] = 0.0;
    p_contactList->contacts[0].contact.g1 = v9;
    p_contactList->contacts[0].contact.g2 = v4;
    p_contactList->contacts[0].surfFlags = 0;
    Phys_CreateJointForEachContact(&contactList, obj0, obj1, &surfParms, worldId);
}

