#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_demo.h"
#include <xanim/xmodel.h>
#include <xanim/dobj.h>
#include <script/scr_stringlist.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <game/g_local.h>
#include <xanim/xanim.h>
#include "client.h"
#include <xanim/xanim_readwrite.h>
#include <server/server.h>
#include <bgame/bg_public.h>
#include <server/sv_public.h>
#include "cl_parse.h"
#include <ui/ui.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_drawsurf.h>

void __cdecl CL_WriteDemoShortCString(MemoryFile *memFile, const char *string)
{
    const char *v4; // r11
    int v6; // r11
    int v7; // r31
    _BYTE v8[48]; // [sp+50h] [-30h] BYREF

    if (string)
    {
        v4 = string;
        while (*(unsigned __int8 *)v4++)
            ;
        v6 = v4 - string - 1;
        v7 = v6 + 1;
        if (v6 + 1 >= 256)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp",
                39,
                0,
                "%s\n\t(len) = %i",
                "(len < 256)",
                v6 + 1);
        v8[0] = v7;
        MemFile_WriteData(memFile, 1, v8);
        MemFile_WriteData(memFile, v7 - 1, string);
    }
    else
    {
        v8[0] = 0;
        MemFile_WriteData(memFile, 1, v8);
    }
}

const char *__cdecl CL_ReadDemoShortCString(MemoryFile *memFile, char *string)
{
    const char *result; // r3
    MemoryFile *v5; // r3
    int v6; // r30
    _BYTE v7[8]; // [sp+50h] [-20h] BYREF

    MemFile_ReadData(memFile, 1, v7);
    if (!v7[0])
        return 0;
    v5 = memFile;
    v6 = (unsigned __int8)(v7[0] - 1);
    MemFile_ReadData(v5, v6, (unsigned char*)string);
    result = string;
    string[v6] = 0;
    return result;
}

void __cdecl CL_WriteDemoDObjModel(MemoryFile *memFile, const DObjModel_s *dobjModel)
{
    const char *Name; // r3
    const char *v5; // r3
    bool v6[8]; // [sp+50h] [-20h] BYREF

    if (!dobjModel)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 67, 0, "%s", "dobjModel");
    Name = XModelGetName(dobjModel->model);
    CL_WriteDemoShortCString(memFile, Name);
    v5 = SL_ConvertToString(dobjModel->boneName);
    CL_WriteDemoShortCString(memFile, v5);
    v6[0] = dobjModel->ignoreCollision;
    MemFile_WriteData(memFile, 1, v6);
}

void __cdecl CL_ReadDemoDObjModel(MemoryFile *memFile, DObjModel_s *dobjModel)
{
    const char *DemoShortCString; // r3
    XModel *Existing; // r3
    const char *v6; // r3
    unsigned char modelFlags; // [sp+50h] [-140h] BYREF
    char v8[304]; // [sp+60h] [-130h] BYREF

    if (!dobjModel)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 81, 0, "%s", "dobjModel");
    DemoShortCString = CL_ReadDemoShortCString(memFile, v8);
    Existing = XModelFindExisting(DemoShortCString);
    dobjModel->model = Existing;
    if (!Existing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 85, 0, "%s", "dobjModel->model");
    v6 = CL_ReadDemoShortCString(memFile, v8);
    dobjModel->boneName = SL_FindString(v6);
    MemFile_ReadData(memFile, 1, &modelFlags);
    //dobjModel->ignoreCollision = (_cntlzw(v7[0]) & 0x20) == 0;
    dobjModel->ignoreCollision = (modelFlags & 0x20); // KISAKTODO: double check this logic
}

void __cdecl CL_WriteAnimTree(MemoryFile *memFile, int entnum, const XAnimTree_s *tree)
{
    int byteCount; // r4
    unsigned int id; // r8
    bool bytes[2]; // [sp+50h] [-20h] BYREF

    byteCount = 1;
    if (tree)
    {
        bytes[0] = 1;
        MemFile_WriteData(memFile, 1, bytes);
        id = G_GetEntAnimTreeId(entnum);
        iassert(id == static_cast<unsigned short>(id));
        *(_WORD *)bytes = id;
        byteCount = 2;
    }
    else
    {
        bytes[0] = 0;
    }
    MemFile_WriteData(memFile, byteCount, bytes);
}

XAnimTree_s *__cdecl CL_ReadAnimTree(MemoryFile *memFile, int entnum)
{
    XAnimTree_s *tree; // r31
    unsigned char buf[4]; // [sp+50h] [-20h] BYREF

    MemFile_ReadData(memFile, 1, (unsigned char*)buf);
    if (!HIBYTE(buf[0]))
        return 0;

    MemFile_ReadData(memFile, 2, (unsigned char *)buf);
    tree = G_GetEntAnimTreeForId(entnum, buf[0]);
    iassert(tree);
    XAnimClearTree(tree);
    return tree;
}

void __cdecl CL_WriteDemoDObj(int entnum, const DObj_s *obj)
{
    const XAnimTree_s *Tree; // r3
    const DObjModel_s *pModel; // r30
    unsigned __int16 modelCount; // [sp+50h] [-160h] BYREF
    unsigned char modelCountWrite; // [sp+52h] [-15Eh] BYREF
    unsigned __int16 v10[6]; // [sp+54h] [-15Ch] BYREF
    MemoryFile memFile; // [sp+60h] [-150h] BYREF
    XAnimTree_s *v12; // [sp+7Ch] [-134h] BYREF
    DObjModel_s v13[38]; // [sp+80h] [-130h] BYREF

    iassert(obj);
    DObjGetCreateParms(obj, v13, &modelCount, &v12, v10);
    MemFile_InitForWriting(&memFile, 0x100000, (byte*)cls.demobuf, 1, 0);
    Tree = DObjGetTree(obj);
    CL_WriteAnimTree(&memFile, entnum, Tree);
    iassert(modelCount == static_cast<byte>(modelCount));
    modelCountWrite = modelCount;
    MemFile_WriteData(&memFile, 1, &modelCountWrite);

    if (modelCount)
    {
        int i = 0;

        pModel = v13;
        do
        {
            CL_WriteDemoDObjModel(&memFile, pModel);
            ++i;
            ++pModel;
        } while (i < modelCount);
    }

    XAnimSaveAnimTree(obj, &memFile);
    MemFile_StartSegment(&memFile, -1);
    FS_Write((const char*)&memFile.bytesUsed, 4, cls.demofile);
    FS_Write((const char*)memFile.buffer, memFile.bytesUsed, cls.demofile);
}

void __cdecl CL_ReadDemoDObj(int entnum)
{
    void *TempMemory; // r31
    int v3; // r3
    XAnimTree_s *v4; // r28
    unsigned __int16 v5; // r29
    int v6; // r31
    DObjModel_s *v7; // r30
    DObj_s *v8; // r3
    _BYTE v9[16]; // [sp+50h] [-160h] BYREF
    MemoryFile memFile; // [sp+60h] [-150h] BYREF
    DObjModel_s v11[38]; // [sp+80h] [-130h] BYREF

    if (FS_Read((unsigned char *)&memFile.bufferSize, 4, cls.demofile) == 4)
    {
        Hunk_CheckTempMemoryClear();
        TempMemory = Hunk_AllocateTempMemory(memFile.bufferSize, "CL_ReadDemoDObj");
        v3 = FS_Read((unsigned char*)TempMemory, memFile.bufferSize, cls.demofile);
        if (v3 == memFile.bufferSize)
        {
            MemFile_InitForReading(&memFile, memFile.bufferSize, (unsigned char*)TempMemory, 0);
            v4 = CL_ReadAnimTree(&memFile, entnum);
            MemFile_ReadData(&memFile, 1, v9);
            v5 = v9[0];
            v6 = v9[0];
            if (v9[0])
            {
                v7 = v11;
                do
                {
                    CL_ReadDemoDObjModel(&memFile, v7);
                    --v6;
                    ++v7;
                } while (v6);
            }
            v8 = Com_ServerDObjCreate(v11, v5, v4, entnum);
            XAnimLoadAnimTree(v8, &memFile);
            MemFile_MoveToSegment(&memFile, -1);
            Hunk_ClearTempMemory();
        }
    }
}

int CL_WriteDemoDObjs()
{
    __int16 v0; // r11
    const DObj_s *ServerDObj; // r31
    _WORD v3[24]; // [sp+50h] [-30h] BYREF

    if (!cls.demorecording)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 194, 0, "%s", "cls.demorecording");
    if (!cls.demofile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 195, 0, "%s", "cls.demofile");
    v0 = 1;
    v3[0] = 1;
    do
    {
        ServerDObj = Com_GetServerDObj(v0);
        if (ServerDObj)
        {
            FS_Write((const char*)v3, 2, cls.demofile);
            CL_WriteDemoDObj(v3[0], ServerDObj);
        }
        v0 = ++v3[0];
    } while (v3[0] < 2176);
    v3[0] = 0;
    return FS_Write((const char*)v3, 2, cls.demofile);
}

void CL_ClearDemoDObjs()
{
    int i; // r31

    for (i = 1; i < 2176; ++i)
        Com_SafeServerDObjFree(i);
}

int CL_ReadDemoDObjs()
{
    int i; // r31
    int result; // r3
    int j; // r11
    _WORD v3[32]; // [sp+50h] [-40h] BYREF

    if (!cls.demoplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 224, 0, "%s", "cls.demoplaying");
    if (!cls.demofile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 225, 0, "%s", "cls.demofile");
    Hunk_CheckTempMemoryClear();
    for (i = 1; i < 2176; ++i)
        Com_SafeServerDObjFree(i);
    v3[0] = 0;
    result = FS_Read((unsigned char*)v3, 2, cls.demofile);
    for (j = v3[0]; v3[0]; j = v3[0])
    {
        if (j < 1 || j >= 2176)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp",
                237,
                0,
                "%s",
                "entnum >= MAX_CLIENTS && entnum < MAX_GENTITIES");
        CL_ReadDemoDObj(v3[0]);
        v3[0] = 0;
        result = FS_Read((unsigned char *)v3, 2, cls.demofile);
    }
    return result;
}

void __cdecl CL_WriteDemoEntityState(const entityState_s *es)
{
    _BYTE v2[16]; // [sp+50h] [-1F0h] BYREF
    MemoryFile v3; // [sp+60h] [-1E0h] BYREF
    _BYTE v4[424]; // [sp+80h] [-1C0h] BYREF

    if (!es)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 250, 0, "%s", "es");
    MemFile_InitForWriting(&v3, 419, v4, 1, 0);
    MemFile_WriteData(&v3, 164, es);
    G_ArchiveSpecialEntityInfo(es, &v3);
    MemFile_StartSegment(&v3, -1);
    v2[0] = v3.bytesUsed;
    if ((v3.bytesUsed & 0xFF) != v3.bytesUsed)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp",
            258,
            0,
            "%s\n\t(memFile.bytesUsed) = %i",
            "(byteLen == memFile.bytesUsed)",
            v3.bytesUsed);
    FS_Write((const char*)v2, 1, cls.demofile);
    FS_Write((const char *)v4, v2[0], cls.demofile);
}

void __cdecl CL_ReadDemoEntityState(entityState_s *es)
{
    _BYTE v2[16]; // [sp+50h] [-1F0h] BYREF
    MemoryFile v3; // [sp+60h] [-1E0h] BYREF
    _BYTE v4[424]; // [sp+80h] [-1C0h] BYREF

    if (!es)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 270, 0, "%s", "es");
    FS_Read(v2, 1, cls.demofile);
    if (v2[0])
    {
        FS_Read(v4, v2[0], cls.demofile);
        MemFile_InitForReading(&v3, v2[0], v4, 0);
        MemFile_ReadData(&v3, 164, (unsigned char*)es);
        G_ArchiveSpecialEntityInfo((const entityState_s*)es, &v3);
        MemFile_MoveToSegment(&v3, -1);
    }
    G_UpdateDemoEntity(es);
}

void __cdecl CL_WriteDemoSnapshotData()
{
    int i; // r31
    gentity_s *EntityState; // r3

    if (!clients[0].snap.valid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 295, 0, "%s", "cl->snap.valid");
    if (!cls.demorecording)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 296, 0, "%s", "cls.demorecording");
    if (!cls.demofile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 297, 0, "%s", "cls.demofile");
    for (i = 0; i < clients[0].snap.numEntities; ++i)
    {
        EntityState = SV_GetEntityState(*(int *)((char *)clients[0].parseEntityNums + ((4 * (clients[0].snap.parseEntitiesNum + i)) & 0x1FFC)));
        CL_WriteDemoEntityState(&EntityState->s);
    }
    CL_WriteDemoDObjs();
}

void __cdecl CL_ReadDemoSnapshotData()
{
    int i; // r31
    gentity_s *EntityState; // r3

    if (!clients[0].snap.valid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 317, 0, "%s", "cl->snap.valid");
    if (!cls.demoplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 318, 0, "%s", "cls.demoplaying");
    if (!cls.demofile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 319, 0, "%s", "cls.demofile");
    G_ClearDemoEntities();
    for (i = 0; i < clients[0].snap.numEntities; ++i)
    {
        EntityState = SV_GetEntityState(*(int *)((char *)clients[0].parseEntityNums
            + ((4 * (clients[0].snap.parseEntitiesNum + i)) & 0x1FFC)));
        CL_ReadDemoEntityState(&EntityState->s);
    }
    CL_ReadDemoDObjs();
}

void __cdecl CL_WriteDemoMessage(msg_t *msg, int headerBytes)
{
    int v4; // r28
    unsigned int v5[12]; // [sp+50h] [-30h] BYREF

    v5[0] = clientConnections[0].serverMessageSequence;
    FS_Write((const char*)v5, 4, cls.demofile);
    v5[0] = msg->cursize - headerBytes;
    v4 = v5[0];
    FS_Write((const char *)v5, 4, cls.demofile);
    FS_Write((const char *)&msg->data[headerBytes], v4, cls.demofile);
}

void __cdecl CL_StopRecord_f()
{
    void *demobuf; // r3
    unsigned int v1[4]; // [sp+50h] [-20h] BYREF

    if (cls.demorecording)
    {
        v1[0] = -1;
        FS_Write((const char *)v1, 4, cls.demofile);
        FS_Write((const char *)v1, 4, cls.demofile);
        FS_FCloseFile(cls.demofile);
        cls.demofile = 0;
        cls.demorecording = 0;
        demobuf = cls.demobuf;
        if (!cls.demobuf)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 383, 0, "%s", "cls.demobuf");
            demobuf = cls.demobuf;
        }
        Z_VirtualFree(demobuf);
        cls.demobuf = 0;
        Com_Printf(0, "Stopped demo.\n");
    }
    else
    {
        Com_Printf(0, "Not recording a demo.\n");
    }
}

void __cdecl CL_Record_f()
{
    int nesting; // r7
    const char *v1; // r3
    int i; // r30
    unsigned __int16 *configstrings; // r30
    const char *v4; // r3
    const char *v5; // r3
    char *v6; // r30
    char *v7; // r11
    int v9; // r3
    _BYTE v10[4]; // [sp+50h] [-41E0h] BYREF
    unsigned int v11[3]; // [sp+54h] [-41DCh] BYREF
    MemoryFile v12; // [sp+60h] [-41D0h] BYREF
    msg_t v13; // [sp+80h] [-41B0h] BYREF
    char v14[64]; // [sp+B0h] [-4180h] BYREF
    char v15[256]; // [sp+F0h] [-4140h] BYREF
    unsigned __int8 v16[64]; // [sp+1F0h] [-4040h] BYREF

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 2)
    {
        if (cls.demorecording)
        {
            Com_Printf(0, "Already recording.\n");
        }
        else if (clientUIActives[0].connectionState == CA_ACTIVE)
        {
            if (Cmd_Argc() == 2)
            {
                v1 = Cmd_Argv(1);
                I_strncpyz(v14, v1, 64);
                Com_sprintf(v15, 256, "demos/%s.spd", v14);
            }
            else
            {
                for (i = 0; i <= 9999; ++i)
                {
                    Com_sprintf(v14, 64, "demo%04i", i);
                    Com_sprintf(v15, 256, "demos/%s.spd", v14);
                    v11[0] = FS_ReadFile(v15, 0);
                    if (v11[0] <= 0)
                        break;
                }
            }
            Com_Printf(0, "recording to %s.\n", v15);
            cls.demofile = FS_FOpenFileWrite(v15);
            if (cls.demofile)
            {
                cls.demorecording = 1;
                if (cls.demobuf)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 473, 0, "%s", "!cls.demobuf");
                cls.demobuf = Z_VirtualAlloc(0x100000, "demo", 0);
                I_strncpyz(cls.demoName, v14, 64);
                MSG_Init(&v13, v16, 0x4000);
                MSG_WriteLong(&v13, clientConnections[0].serverCommands.header.sent);
                configstrings = clients[0].configstrings;
                do
                {
                    if (!*configstrings)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 489, 0, "%s", "cl->configstrings[i]");
                    v4 = SL_ConvertToString(*configstrings);
                    MSG_WriteString(&v13, (char*)v4);
                    ++configstrings;
                } while ((int)configstrings < (int)clients[0].mapname);
                v5 = SL_ConvertToString(clients[0].configstrings[0]);
                v6 = (char*)Info_ValueForKey(v5, "mapname");
                v7 = v6;
                while (*v7++)
                    ;
                v10[0] = (_BYTE)v7 - (_BYTE)v6 - 1;
                FS_Write((const char*)v10, 1, cls.demofile);
                v9 = FS_Write(v6, v10[0], cls.demofile);
                Hunk_CheckTempMemoryClear();
                MemFile_InitForWriting(&v12, 0x100000, (unsigned char*)cls.demobuf, 1, 0);
                CL_ArchiveClientState(&v12, 0);
                MemFile_StartSegment(&v12, -1);
                FS_Write((const char *)&v12.bytesUsed, 4, cls.demofile);
                FS_Write((const char *)v12.buffer, v12.bytesUsed, cls.demofile);
                v11[0] = 0;
                FS_Write((const char *)v11, 4, cls.demofile);
                v11[0] = v13.cursize;
                FS_Write((const char *)v11, 4, cls.demofile);
                FS_Write((const char *)v13.data, v13.cursize, cls.demofile);
            }
            else
            {
                Com_PrintError(0, "ERROR: couldn't open.\n");
            }
        }
        else
        {
            Com_Printf(0, "You must be in a level to record.\n");
        }
    }
    else
    {
        Com_Printf(0, "record <demoname>\n");
    }
}

void CL_DemoPlaybackStartup()
{
    unsigned __int8 v0; // r11
    int v1; // r30
    int demofile; // r5
    void *v3; // r28
    int v4; // r3
    _BYTE v5[4]; // [sp+50h] [-A0h] BYREF
    int v6[3]; // [sp+54h] [-9Ch] BYREF
    MemoryFile v7; // [sp+60h] [-90h] BYREF
    char v8[112]; // [sp+80h] [-70h] BYREF

    v5[0] = 0;
    if (FS_Read(v5, 1, cls.demofile) == 1)
    {
        v0 = v5[0];
        if (v5[0] < 0x3Fu)
        {
            v1 = v5[0];
            if (FS_Read((unsigned char *)v8, v5[0], cls.demofile) == v0)
            {
                demofile = cls.demofile;
                v8[v1] = 0;
                if (FS_Read((unsigned char*)v6, 4, demofile) == 4)
                {
                    v3 = Z_VirtualAlloc(v6[0], "CL_DemoPlaybackStartup", 10);
                    v4 = FS_Read((unsigned char *)v3, v6[0], cls.demofile);
                    if (v4 == v6[0])
                    {
                        Dvar_SetBoolByName("sv_cheats", 1);
                        FS_ConvertPath(v8);
                        SV_SpawnServer(v8, 0);
                        if (!cls.demofile)
                            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 715, 0, "%s", "cls.demofile");
                        if (!cls.demoplaying)
                            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 716, 0, "%s", "cls.demoplaying");
                        MemFile_InitForReading(&v7, v6[0], (unsigned char *)v3, 0);
                        CL_ArchiveClientState(&v7, 0);
                        MemFile_MoveToSegment(&v7, -1);
                        Z_VirtualFree(v3);
                        R_SyncRenderThread();
                        Material_Sort();
                        R_SortWorldSurfaces();
                    }
                    else
                    {
                        Z_VirtualFree(v3);
                    }
                }
            }
        }
    }
}

void __cdecl CL_PlayDemo_f()
{
    int nesting; // r7
    const char *v1; // r3
    int v2; // r3
    const char *v3; // r3
    const char *v4; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] == 2)
    {
        if (com_sv_running->current.enabled)
        {
            Com_Printf(0, "listen server cannot play a demo.\n");
        }
        else
        {
            v2 = Cmd_LocalClientNum();
            CL_Disconnect(v2);
            v3 = Cmd_Argv(1);
            I_strncpyz(cls.demoName, v3, 64);
            Com_StripExtension(cls.demoName, cls.demoName);
            cls.demoPending = 1;
            v4 = Cmd_Argv(0);
            cls.isTimeDemo = I_stricmp(v4, "timedemo") == 0;
        }
    }
    else
    {
        v1 = Cmd_Argv(0);
        Com_Printf(0, "%s <demoname>\n", v1);
    }
}

void __cdecl CL_CheckStartPlayingDemo()
{
    char buf[256]; // [sp+50h] [-110h] BYREF

    if (cls.demoPending)
    {
        cls.demoPending = 0;
        Com_sprintf(buf, 256, "demos/%s.spd", cls.demoName);
        FS_FOpenFileRead(buf, &cls.demofile);
        if (!cls.demofile)
        {
            Com_Error(ERR_DROP, va("EXE_ERR_CANT_READ", buf));
        }
        Con_Close(0);
        cls.demoplaying = 1;
        CL_DemoPlaybackStartup();
    }
}

void __cdecl CL_NextDemo()
{
    const char *String; // r31
    int v1; // r3

    String = Dvar_GetString("nextdemo");
    Com_DPrintf(14, "CL_NextDemo: %s\n", String);
    if (*String)
    {
        Cbuf_AddText(0, String);
        Cbuf_AddText(0, "\n");
        Dvar_SetStringByName("nextdemo", "");
        v1 = CL_ControllerIndexFromClientNum(0);
        Cbuf_Execute(0, v1);
    }
}

int __cdecl CL_DemoPlaybackTime()
{
    iassert(cls.demoplaying);
    return clients[0].snap.serverTime;
}

bool __cdecl CL_DemoPlaying()
{
    return cls.demoplaying;
}

bool __cdecl CL_DemoRecording()
{
    return cls.demorecording;
}

int __cdecl CL_TimeDemoPlaying()
{
    int result; // r3

    if (!cls.demoplaying)
        return 0;
    result = 1;
    if (!cls.isTimeDemo)
        return 0;
    return result;
}

// local variable allocation has failed, the output may be wrong!
void CL_DemoCompleted()
{
    //__int64 v1; // r9 OVERLAPPED
    int timeMS; // r3
    int timeElapsedMS; // r10

    iassert(cls.demoplaying);
    iassert(cls.demofile);

    for (int i = 1; i < 2176; ++i)
        Com_SafeServerDObjFree(i);

    if (cls.isTimeDemo)
    {
        timeMS = Sys_Milliseconds();
        if (timeMS - cls.timeDemoStart > 0)
        {
            timeElapsedMS = timeMS - cls.timeDemoStart;
            Com_Printf(
                14,
                "%i frames, %3.1f seconds: %3.1f fps\n",
                cls.timeDemoFrames,
                timeElapsedMS * 0.001f,
                (timeElapsedMS * 1000.0f / cls.timeDemoFrames)
            );
        }
    }
    if (cls.timeDemoLog)
    {
        FS_FCloseFile(cls.timeDemoLog);
        cls.timeDemoLog = 0;
    }
    FS_FCloseFile(cls.demofile);
    cls.demofile = 0;
    cls.demoplaying = 0;
    if (!UI_IsFullscreen() || UI_AutoContinue())
    {
        Com_Shutdown("EXE_DEMOFINISHED");
        CL_NextDemo();
    }
}

int __cdecl CL_GetDemoMessage(msg_t *buf, unsigned __int8 *bufData, int bufDataSize)
{
    int *p_cursize; // r31
    int result; // r3
    int v8; // [sp+50h] [-40h] BYREF

    if (!cls.demoplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 581, 0, "%s", "cls.demoplaying");
    if (!buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 582, 0, "%s", "buf");
    if (!bufData)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 583, 0, "%s", "bufData");
    if (bufDataSize <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp", 584, 0, "%s", "bufDataSize > 0");
    if (!cls.demofile)
        goto LABEL_17;
    if (FS_Read((unsigned char*)&v8, 4, cls.demofile) != 4)
        goto LABEL_17;
    clientConnections[0].serverMessageSequence = v8;
    MSG_Init(buf, bufData, bufDataSize);
    p_cursize = &buf->cursize;
    if (FS_Read((unsigned char*)&buf->cursize, 4, cls.demofile) != 4 || *p_cursize == -1)
        goto LABEL_17;
    if (*p_cursize > buf->maxsize)
        Com_Error(ERR_DROP, "CL_GetDemoMessage: demoMsglen > MAX_MSGLEN");
    if (FS_Read(buf->data, *p_cursize, cls.demofile) != *p_cursize)
    {
        Com_Printf(14, "Demo file was truncated.\n");
    LABEL_17:
        CL_DemoCompleted();
        return 0;
    }
    result = 1;
    clientConnections[0].lastPacketTime = cls.realtime;
    buf->readcount = 0;
    return result;
}

void __cdecl CL_ReadDemoMessage()
{
    msg_t v0; // [sp+50h] [-4040h] BYREF
    unsigned __int8 v1[16]; // [sp+80h] [-4010h] BYREF

    if (CL_GetDemoMessage(&v0, v1, 0x4000))
        CL_ParseServerMessage(&v0);
}

void __cdecl CL_ReadDemoMessagesUntilNextSnap()
{
    int serverTime; // r30

    serverTime = clients[0].snap.serverTime;
    do
    {
        if (!cls.demoplaying)
            break;
        CL_ReadDemoMessage();
    } while (serverTime == clients[0].snap.serverTime);
}

void __cdecl CL_FinishLoadingDemo()
{
    int v0; // r30
    char *v1; // r31
    unsigned int ConfigstringConst; // r3
    const char *v3; // r3
    int v4; // r10
    const char *v5; // r3
    const char *v6; // r3
    msg_t v7; // [sp+50h] [-4470h] BYREF
    char v8[1024]; // [sp+80h] [-4440h] BYREF
    unsigned __int8 v9[64]; // [sp+480h] [-4040h] BYREF

    if (CL_GetDemoMessage(&v7, v9, 0x4000))
    {
        if (clientConnections[0].serverMessageSequence)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp",
                666,
                0,
                "%s",
                "clc->serverMessageSequence == 0");
        v0 = 0;
        clientConnections[0].serverCommands.header.sent = MSG_ReadLong(&v7);
        do
        {
            MSG_ReadString(&v7, v8, 1024);
            if (v8[0] && (unsigned int)(v0 - 1155) <= 0x1FF)
                G_SetModelIndex(v0 - 1155, v8);
            else
                SV_SetConfigstring(v0, v8);
            v1 = v8;
            ConfigstringConst = SV_GetConfigstringConst(v0);
            v3 = SL_ConvertToString(ConfigstringConst);
            do
            {
                v4 = *(unsigned __int8 *)v3 - (unsigned __int8)*v1;
                if (!*v3)
                    break;
                ++v3;
                ++v1;
            } while (!v4);
            if (v4)
            {
                v5 = (const char *)SV_GetConfigstringConst(v0);
                v6 = va("%s != %s", v5, v8);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\client\\cl_demo.cpp",
                    677,
                    0,
                    "%s\n\t%s",
                    "!strcmp( SL_ConvertToString( SV_GetConfigstringConst( csIndex ) ), cs )",
                    v6);
            }
            ++v0;
        } while (v0 < 2815);
        SV_SendGameState();
    }
}

void __cdecl CL_StartPlayingDemo()
{
    iassert(cls.demoplaying);
    iassert(cls.demofile);

    I_strncpyz(cls.servername, cls.demoName, 256);
    Dvar_SetIntByName("cl_paused", 0);
    CL_SetSkipRendering(0);
    SND_FadeAllSounds(1.0, 0);
    iassert(clients[0].snap.valid);
    CL_ReadDemoMessagesUntilNextSnap();
    G_SetClientDemoTime(clients[0].snap.serverTime);
    cls.timeDemoBaseTime = clients[0].snap.serverTime;
    com_time = clients[0].snap.serverTime - 50;
    cls.timeDemoFrames = 0;
}

