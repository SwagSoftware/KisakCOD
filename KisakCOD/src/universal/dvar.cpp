#include <qcommon/qcommon.h>

#include <Windows.h>
#include <win32/win_local.h>

static FastCriticalSection g_dvarCritSect;

static dvar_s* dvarHashTable[0x100];
    
static dvar_s dvarPool[0x1000];
static const dvar_s* sortedDvars[0x1000];
static bool areDvarsSorted;
static LONG isSortedDvars;
static int dvarCount;

static int generateHashValue(const char* fname)
{
    if (!fname)
    {
        Com_Error(ERR_DROP, "null name in generateHashValue");
    }
    int hash = 0;
    for (int i = 0; fname[i]; ++i)
        hash += tolower(fname[i]) * (i + 119);
    return (unsigned __int8)hash;
}

static dvar_s* Dvar_FindMalleableVar(const char* dvarName)
{
    InterlockedIncrement(&g_dvarCritSect.readCount);

    while (g_dvarCritSect.writeCount)
    {
        NET_Sleep(0);
    }

    for (dvar_s* var = dvarHashTable[generateHashValue(dvarName)]; var; var = var->hashNext)
    {
        if (!I_stricmp(dvarName, var->name))
        {
            iassert(g_dvarCritSect.readCount > 0);
            InterlockedDecrement(&g_dvarCritSect.readCount);
            return var;
        }
    }

    iassert(g_dvarCritSect.readCount > 0);
    InterlockedDecrement(&g_dvarCritSect.readCount);
    return NULL;
}

const dvar_s* Dvar_FindVar(const char* dvarName)
{
	return Dvar_FindMalleableVar(dvarName);
}

const dvar_s* Dvar_RegisterBool(
    const char* dvarName,
    bool value,
    word flags,
    const char* description)
{
    DvarValue dvarValue;
    DvarLimits blank{ 0 };

    dvarValue.enabled = value;
    return Dvar_RegisterVariant(dvarName, 0, flags, dvarValue, blank, description);
}

const dvar_s* Dvar_RegisterNew(
    const char* dvarName,
    byte type,
    word flags,
    DvarValue value,
    DvarLimits domain,
    const char* description)
{
    Sys_LockWrite(&g_dvarCritSect);

    if (dvarCount >= 0x1000)
    {
        Sys_UnlockWrite(&g_dvarCritSect);
        Com_Error(ERR_FATAL, "Can't create dvar '%s': %i dvars already exist", dvarName, dvarCount);
    }

    dvar_s* dvar = &dvarPool[dvarCount];
    sortedDvars[dvarCount] = dvar;
    areDvarsSorted = false;
    dvarCount++;

    dvar->type = type;
    if ((flags & DVAR_EXTERNAL) != 0)
    {
        dvar->name = Dvar_AllocNameString(dvarName);
    }
    else
    {
        dvar->name = dvarName;
    }

    switch (type)
    {
    case 2:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        break;
    case 3:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->current.vector[2] = value.vector[2];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->latched.vector[2] = value.vector[2];
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        dvar->reset.vector[2] = value.vector[2];
        break;
    case 4:
        dvar->current = value;
        dvar->latched = value;
        dvar->reset = value;
        break;
    case 7:
        Dvar_CopyString(value.string, &dvar->current);
        Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
        Dvar_WeakCopyString(dvar->current.string, &dvar->reset);
        break;
    default:
        dvar->current = value;
        dvar->latched = value;
        dvar->reset = value;
        break;
    }

    dvar->domain = domain;
    dvar->modified = 0;
    dvar->domainFunc = 0;
    dvar->flags = flags;
    dvar->description = description;

    int hashValue = generateHashValue(dvarName);
    dvar->hashNext = dvarHashTable[hashValue];
    dvarHashTable[hashValue] = dvar;
    Sys_UnlockWrite(&g_dvarCritSect);
    
    return dvar;
}

static DvarValue* Dvar_GetReinterpretedResetValue(DvarValue* result, dvar_s* __formal, DvarValue value)
{
    *result = value;
    return result;
}

static const char* Dvar_AllocNameString(const char* name)
{
    return CopyString(name);
}

static void Dvar_PerformUnregistration(dvar_s* dvar)
{
    DvarValue resetString; // [esp+0h] [ebp-14h] BYREF

    iassert(dvar);

    if ((dvar->flags & DVAR_EXTERNAL) == 0)
    {
        dvar->flags |= DVAR_EXTERNAL;
        dvar->name = Dvar_AllocNameString(dvar->name);
    }
    if (dvar->type != DVAR_TYPE_STRING)
    {
        Dvar_CopyString(Dvar_DisplayableLatchedValue(dvar), &dvar->current);
        if (Dvar_ShouldFreeLatchedString(dvar))
            Dvar_FreeString(&dvar->latched);
        dvar->latched.integer = 0;
        Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
        if (Dvar_ShouldFreeResetString(dvar))
            Dvar_FreeString(&dvar->reset);
        dvar->reset.integer = 0;
        Dvar_AssignResetStringValue(dvar, &resetString, Dvar_DisplayableResetValue(dvar));
        dvar->reset.integer = resetString.integer;
        dvar->type = DVAR_TYPE_STRING;
    }
}

static void Dvar_FreeNameString(const char* name)
{
    FreeString(name, 10);
}

static void Dvar_ReinterpretDvar(
    dvar_s* dvar,
    const char* dvarName,
    byte type,
    word flags,
    DvarValue value,
    DvarLimits domain)
{
    DvarValue result; 
    DvarValue resetValue; 

    if ((dvar->flags & DVAR_EXTERNAL) != 0 && (flags & DVAR_EXTERNAL) == 0)
    {
        resetValue = *Dvar_GetReinterpretedResetValue(&result, dvar, value);
        Dvar_PerformUnregistration(dvar);
        Dvar_FreeNameString(dvar->name);
        dvar->name = dvarName;
        dvar->flags &= ~DVAR_EXTERNAL;
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }
}

static void __cdecl Dvar_Reregister(
    dvar_s* dvar,
    const char* dvarName,
    byte type,
    word flags,
    DvarValue resetValue,
    DvarLimits domain,
    const char* description)
{
    iassert(dvar);
    iassert(dvarName);
    iassert(dvar->type == type || (dvar->flags & DVAR_EXTERNAL));

    if (((flags ^ dvar->flags) & DVAR_EXTERNAL) != 0)
    {
        Dvar_ReinterpretDvar(dvar, dvarName, type, flags, resetValue, domain);
    }

    if ((dvar->flags & DVAR_EXTERNAL) != 0 && dvar->type != type)
    {
        iassert(dvar->type == DVAR_TYPE_STRING);
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }

    iassert(dvar->type == type);
    iassert((dvar->flags & (DVAR_CHANGEABLE_RESET | DVAR_SAVED | DVAR_AUTOEXEC)) || Dvar_ValuesEqual(type, dvar->reset, resetValue));

    dvar->flags |= flags;
    if (description)
    {
        dvar->description = description;
    }

    if ((dvar->flags & DVAR_CHEAT) != 0 && dvar_cheats && !dvar_cheats->current.enabled)
    {
        Dvar_SetVariant(dvar, dvar->reset, DVAR_SOURCE_INTERNAL);
        Dvar_SetLatchedValue(dvar, dvar->reset);
    }
    if ((dvar->flags & DVAR_LATCH) != 0)
    {
        Dvar_MakeLatchedValueCurrent(dvar);
    }
}

const dvar_s* Dvar_RegisterVariant(
    const char* dvarName,
    byte type,
    word flags,
    DvarValue value,
    DvarLimits domain,
    const char* description)
{
   // if ((flags & 0x4000) == 0 && !CanKeepStringPointer(dvarName))
   //     MyAssertHandler(
   //         ".\\universal\\dvar.cpp",
   //         1662,
   //         0,
   //         "%s\n\t(dvarName) = %s",
   //         "((flags & (1 << 14)) || CanKeepStringPointer( dvarName ))",
   //         dvarName);

    dvar_s* dvar = Dvar_FindMalleableVar(dvarName);
    if (!dvar)
    {
        return Dvar_RegisterNew(dvarName, type, flags, value, domain, description);
    }
    Dvar_Reregister(dvar, dvarName, type, flags, value, domain, description);
    return dvar;
}