#include "stringed_ingame.h"
#include <universal/assertive.h>
#include <universal/com_memory.h>
#include <universal/q_shared.h>
#include <database/database.h>
#include <universal/com_files.h>

CStringEdPackage *TheStringPackage;

int giFilesFound;
char sTemp[64];

const char *__cdecl SE_GetString(const char *psPackageAndStringReference)
{
    if (useFastFile->current.enabled)
        return SE_GetString_FastFile(psPackageAndStringReference);
    else
        return (const char *)SE_GetString_LoadObj(psPackageAndStringReference);
}

const char *__cdecl SE_GetString_FastFile(const char *psPackageAndStringReference)
{
    LocalizeEntry *localize; // [esp+8h] [ebp-4h]

    localize = DB_FindXAssetHeader(ASSET_TYPE_LOCALIZE_ENTRY, psPackageAndStringReference).localize;
    if (localize)
        return localize->value;
    else
        return 0;
}

char *__cdecl SE_Load(char *psFileName, bool forceEnglish)
{
    char *psParsePos; // [esp+14h] [ebp-4014h] BYREF
    char psDest; // [esp+18h] [ebp-4010h] BYREF
    _BYTE v5[3]; // [esp+19h] [ebp-400Fh] BYREF
    const char *v6; // [esp+4020h] [ebp-8h]
    unsigned __int8 *psLoadedFile; // [esp+4024h] [ebp-4h]

    v6 = 0;
    psLoadedFile = SE_LoadFileData(psFileName);
    if (!psLoadedFile)
        return va("Unable to load \"%s\"!", psFileName);
    psParsePos = (char *)psLoadedFile;
    //CStringEdPackage::SetupNewFileParse(TheStringPackage, psFileName);
    TheStringPackage->SetupNewFileParase(psFileName);
    //while (!v6 && CStringEdPackage::ReadLine(TheStringPackage, (const char **)&psParsePos, &psDest))
    while (!v6 && TheStringPackage->ReadLine(&psParsePos, &psDest))
    {
        if (&v5[strlen(&psDest)] != v5)
        {
            //v6 = CStringEdPackage::ParseLine(TheStringPackage, &psDest, forceEnglish);
            v6 = TheStringPackage->ParseLine(&psDest, forceEnglish);
        }
    }
    if (v6)
        v6 = va("%s in %s", v6, psFileName);
    SE_FreeFileDataAfterLoad(psLoadedFile);
    if (!v6 && !TheStringPackage->m_bEndMarkerFound_ParseOnly)
        return va("Truncated file, failed to find \"%s\" at file end!", "ENDMARKER");
    return (char *)v6;
}

// KISAKTODO: my god this is aids
const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference)
{
    //const std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > const, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > > *v1; // eax
    //std::map<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t> > *p_m_StringEntries; // [esp+18h] [ebp-88h]
    //std::_Tree_nod<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::_Node *Myhead; // [esp+1Ch] [ebp-84h]
    //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::const_iterator _Right; // [esp+70h] [ebp-30h] BYREF
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > _Keyval; // [esp+78h] [ebp-28h] BYREF
    //std::string _Keyval;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *Entry; // [esp+94h] [ebp-Ch]
    //std::string *Entry;
    //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::iterator itEntry; // [esp+98h] [ebp-8h] BYREF

    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 0, 0);
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
    //    &_Keyval,
    //    psPackageAndStringReference,
    //    strlen(psPackageAndStringReference));
    //_Keyval.assign(psPackageAndStringReference);
    //TheStringPackage->m_StringEntries.find()
    //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::find(
    //    &TheStringPackage->m_StringEntries,
    //    &itEntry,
    //    &_Keyval);
    auto itEntry = TheStringPackage->m_StringEntries.find(psPackageAndStringReference);
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 1, 0);
    //auto p_m_StringEntries = &TheStringPackage->m_StringEntries;
    //Myhead = TheStringPackage->m_StringEntries._Myhead;
    //_Right._Myaux = 0;
    //_Right._Ptr = Myhead;
    //if (TheStringPackage == (CStringEdPackage *)-88)
    //    _invalid_parameter_noinfo();
    //_Right._Myaux = p_m_StringEntries->_Myownedaux;

    //if (!std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::const_iterator::operator!=(
    //    &itEntry,
    //    &_Right))
    //    return 0;
    if (itEntry == TheStringPackage->m_StringEntries.end())
    {
        return NULL;
    }

    //auto v1 = *itEntry; // std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::const_iterator::operator*(&itEntry);
    //Entry = v1.second.c_str();

    return (const char *)&itEntry->second;
    //if (v1->second._Myres < 0x10)
    //    return Entry->_Bx._Buf;
    //else
    //    return Entry->_Bx._Ptr;
}

void __cdecl SE_NewLanguage()
{
    if (!TheStringPackage)
        MyAssertHandler(".\\stringed\\stringed_ingame.cpp", 799, 0, "%s", "TheStringPackage");
    //CStringEdPackage::Clear(TheStringPackage);
    TheStringPackage->Clear();
}

void __cdecl SE_Init()
{
    CStringEdPackage *v1; // [esp+0h] [ebp-68h]

    if (TheStringPackage)
        MyAssertHandler(".\\stringed\\stringed_ingame.cpp", 811, 0, "%s", "!TheStringPackage");
    void* v2 = Z_Malloc(120, "CStringEdPackage", 33);
    if (v2)
    {
        //CStringEdPackage::CStringEdPackage(v2);
        v1 = new (v2) CStringEdPackage();
    }
    else
    {
        v1 = 0;
    }
    TheStringPackage = v1;
    //CStringEdPackage::Clear(v1);
    v1->Clear();
}

void __cdecl SE_ShutDown()
{
    CStringEdPackage *v0; // [esp+44h] [ebp-4h]

    if (TheStringPackage)
    {
        //CStringEdPackage::Clear(TheStringPackage);
        TheStringPackage->Clear();
        v0 = TheStringPackage;
        if (TheStringPackage)
        {
            //CStringEdPackage::~CStringEdPackage(TheStringPackage);
            //Z_Free((char *)v0, 33);
        }
        TheStringPackage = 0;
    }
}

char *__cdecl SE_LoadLanguage(bool forceEnglish)
{
    char *psErrorMessage; // [esp+30h] [ebp-28h]
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > strResults; // [esp+34h] [ebp-24h] BYREF
    std::string strResults;
    const char *p; // [esp+54h] [ebp-4h]

    psErrorMessage = 0;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&strResults, 0, 0);
    strResults.clear();
    SE_NewLanguage();
    SE_BuildFileList("localizedstrings", &strResults);
    while (1)
    {
        p = SE_GetFoundFile(&strResults);
        if (!p || psErrorMessage)
            break;
        psErrorMessage = SE_Load((char *)p, forceEnglish);
    }
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&strResults, 1, 0);
    return psErrorMessage;
}

char *__cdecl SE_GetFoundFile(std::string *strResult)
{
    _BYTE *v2; // eax
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *p_Bx; // [esp+14h] [ebp-38h]

    //if (strResult->_Myres < 0x10)
    //    p_Bx = (std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *) & strResult->_Bx;
    //else
    //    p_Bx = (std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *)strResult->_Bx._Ptr;
    //if (!strlen(p_Bx->_Buf))
    //    return 0;
    if (!strResult->size())
    {
        return NULL;
    }
    //if (strResult->_Myres < 0x10)
    //    strncpy((unsigned __int8 *)sTemp, (unsigned __int8 *)&strResult->_Bx, 0x3Fu);
    //else
    //    strncpy((unsigned __int8 *)sTemp, (unsigned __int8 *)strResult->_Bx._Ptr, 0x3Fu);

    strncpy(sTemp, strResult->data(), 63);
    sTemp[63] = 0;
    v2 = (_BYTE *)strchr(sTemp, ';');
    if (v2)
    {
        *v2 = 0;
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::erase(
        //    strResult,
        //    0,
        //    v2 - sTemp + 1);
        strResult->erase(0, (char*)v2 - sTemp + 1);
    }
    else
    {
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::erase(
        //    strResult,
        //    0,
        //    std::string::npos);
        strResult->erase(0, std::string::npos);
    }
    return sTemp;
}

unsigned __int8 *__cdecl SE_LoadFileData(const char *psFileName)
{
    int len; // [esp+0h] [ebp-8h]
    unsigned __int8 *pvLoadedData; // [esp+4h] [ebp-4h] BYREF

    len = FS_ReadFile(psFileName, (void **)&pvLoadedData);
    return len <= 0 ? 0 : pvLoadedData;
}

void __cdecl SE_FreeFileDataAfterLoad(unsigned __int8 *psLoadedFile)
{
    if (!psLoadedFile)
        MyAssertHandler(".\\stringed\\stringed_interface.cpp", 34, 0, "%s", "psLoadedFile");
    FS_FreeFile((char *)psLoadedFile);
}

int __cdecl SE_BuildFileList(
    const char *psStartDir,
    std::string *strResults)
{
    giFilesFound = 0;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
    //    strResults,
    //    "",
    //    &String + strlen("") + 1 - algn_85A0F1);
    strResults->assign("");
    SE_R_ListFiles("str", psStartDir, strResults);
    return giFilesFound;
}

void __cdecl SE_R_ListFiles(
    const char *psExtension,
    const char *psDir,
    std::string *strResults)
{
    char sFilename[64]; // [esp+54h] [ebp-98h] BYREF
    char sDirName[64]; // [esp+94h] [ebp-58h] BYREF
    int numdirs; // [esp+D8h] [ebp-14h] BYREF
    int numSysFiles; // [esp+DCh] [ebp-10h] BYREF
    int i; // [esp+E0h] [ebp-Ch]
    const char **sysFiles; // [esp+E4h] [ebp-8h]
    const char **dirFiles; // [esp+E8h] [ebp-4h]

    dirFiles = FS_ListFiles(psDir, "/", FS_LIST_PURE_ONLY, &numdirs);
    for (i = 0; i < numdirs; ++i)
    {
        if (*dirFiles[i])
        {
            if (*dirFiles[i] != 46)
            {
                sprintf(sDirName, "%s/%s", psDir, dirFiles[i]);
                SE_R_ListFiles(psExtension, sDirName, strResults);
            }
        }
    }
    sysFiles = FS_ListFiles(psDir, psExtension, FS_LIST_PURE_ONLY, &numSysFiles);
    for (i = 0; i < numSysFiles; ++i)
    {
        sprintf(sFilename, "%s/%s", psDir, sysFiles[i]);
        strResults->append(sFilename);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::append(
        //    strResults,
        //    sFilename,
        //    &sFilename[strlen(sFilename) + 1] - &sFilename[1]);
        strResults->append(1, ';');
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::append(strResults, 1u, 59);
        ++giFilesFound;
    }
    FS_FreeFileList(sysFiles);
    FS_FreeFileList(dirFiles);
}

