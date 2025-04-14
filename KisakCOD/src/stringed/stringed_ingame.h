#pragma once
#include <string>
#include <map>

#include <qcommon/qcommon.h>

struct LocalizeName // sizeof=0x0
{                                       // ...
};
struct mapStringEntriesName_t : LocalizeName // sizeof=0x0
{
};

struct CStringEdPackage // sizeof=0x78
{
    void REMKill(char *psBuffer)
    {
        char *v2; // eax
        int iWhiteSpaceScanPos; // [esp+14h] [ebp-18h]
        int i; // [esp+18h] [ebp-14h]
        int iDoubleQuoteCount; // [esp+1Ch] [ebp-10h]
        int iDoubleQuotesSoFar; // [esp+20h] [ebp-Ch]

        for (iDoubleQuotesSoFar = 0; ; iDoubleQuotesSoFar = iDoubleQuoteCount)
        {
            strstr((unsigned __int8 *)psBuffer, (unsigned __int8 *)"//");
            if (!v2)
                break;
            iDoubleQuoteCount = iDoubleQuotesSoFar;
            for (i = 0; i < v2 - psBuffer; ++i)
            {
                if (psBuffer[i] == 34)
                    ++iDoubleQuoteCount;
            }
            if ((iDoubleQuoteCount & 1) == 0)
            {
                *v2 = 0;
                if (*psBuffer)
                {
                    for (iWhiteSpaceScanPos = strlen(psBuffer) - 1;
                        iWhiteSpaceScanPos >= 0 && isspace(psBuffer[iWhiteSpaceScanPos]);
                        --iWhiteSpaceScanPos)
                    {
                        psBuffer[iWhiteSpaceScanPos] = 0;
                    }
                }
                return;
            }
            psBuffer = v2 + 1;
        }
    }

    int CheckLineForKeyword(
        const char *psKeyword,
        const char **psLine)
    {
        if (I_strnicmp(psKeyword, *psLine, strlen(psKeyword)))
            return 0;
        for (*psLine += strlen(psKeyword); **psLine == 9 || **psLine == 32; ++*psLine)
            ;
        return 1;
    }

    std::string * InsideQuotes(std::string *result, const char *psLine)
    {
        unsigned int v4; // kr04_4
        unsigned int v5; // kr08_4
        unsigned int v6; // kr0C_4
        unsigned int Myres; // edi
        std::string *Ptr; // eax
        unsigned int v9; // eax
        std::string *v10; // ecx
        std::string *v11; // eax

        result->_Myres = 15;
        result->_Mysize = 0;
        result->_Bx._Buf[0] = 0;
        std::string::assign(result, (const char *)&pBlock, 0);
        while (*psLine == ' ' || *psLine == '\t')
            ++psLine;
        if (*psLine == '"')
            ++psLine;
        std::string::assign(result, psLine, strlen(psLine));
        if (*psLine)
        {
            while (1)
            {
                v4 = strlen(std::string::c_str(result));
                if (std::string::c_str(result)[v4 - 1] != 32)
                {
                    v5 = strlen(std::string::c_str(result));
                    if (std::string::c_str(result)[v5 - 1] != 9)
                        break;
                }
                v6 = strlen(std::string::c_str(result));
                std::string::erase(result, v6 - 1, 1u);
            }
            Myres = result->_Myres;
            if (Myres < 0x10)
                Ptr = result;
            else
                Ptr = (std::string *)result->_Bx._Ptr;
            v9 = strlen(Ptr->_Bx._Buf);
            if (Myres < 0x10)
                v10 = result;
            else
                v10 = (std::string *)result->_Bx._Ptr;
            if (v10->_Bx._Buf[v9 - 1] == 34)
            {
                if (Myres < 0x10)
                    v11 = result;
                else
                    v11 = (std::string *)result->_Bx._Ptr;
                std::string::erase(result, strlen(v11->_Bx._Buf) - 1, 1u);
            }
        }
        return result;
    }

    const char *ParseLine(char *psLine, bool forceEnglish)
    {
        std::string *Ptr; // eax
        int v5; // eax
        std::string *p_sentence; // eax
        const char *v7; // edi
        std::string *p_m_strCurrentEntryRef_ParseOnly; // eax
        bool v9; // zf
        char v10; // al
        unsigned __int8 *v11; // edi
        unsigned __int8 *i; // esi
        unsigned int v13; // esi
        std::string *v14; // eax
        std::string *v15; // eax
        BOOL v16; // eax
        std::string *v17; // ecx
        std::string v19; // [esp-1Ch] [ebp-450h] BYREF
        const char *psErrorMessage; // [esp+Ch] [ebp-428h]
        const char *psReference; // [esp+10h] [ebp-424h] BYREF
        std::string sentence; // [esp+14h] [ebp-420h] BYREF
        char sThisLanguage[1024]; // [esp+30h] [ebp-404h] BYREF

        psReference = psLine;
        psErrorMessage = 0;
        if (psLine)
        {
            if (CStringEdPackage::CheckLineForKeyword(this, "VERSION", &psReference))
            {
                CStringEdPackage::InsideQuotes(this, &sentence, psReference);
                Ptr = (std::string *)sentence._Bx._Ptr;
                if (sentence._Myres < 0x10)
                    Ptr = &sentence;
                v5 = atoi(Ptr->_Bx._Buf);
                if (v5 != 1)
                    psErrorMessage = va("Unexpected version number %d, expecting %d!", v5, 1);
                if (sentence._Myres >= 0x10)
                    operator delete(sentence._Bx._Ptr);
            }
            else if (!CStringEdPackage::CheckLineForKeyword(this, "CONFIG", &psReference)
                && !CStringEdPackage::CheckLineForKeyword(this, "FILENOTES", &psReference)
                && !CStringEdPackage::CheckLineForKeyword(this, "NOTES", &psReference)
                && !CStringEdPackage::CheckLineForKeyword(this, "FLAGS", &psReference))
            {
                if (CStringEdPackage::CheckLineForKeyword(this, "REFERENCE", &psReference))
                {
                    CStringEdPackage::InsideQuotes(this, &sentence, psReference);
                    p_sentence = (std::string *)sentence._Bx._Ptr;
                    if (sentence._Myres < 0x10)
                        p_sentence = &sentence;
                    CStringEdPackage::AddEntry(this, p_sentence->_Bx._Buf);
                    std::string::~string(&sentence);
                }
                else if (CStringEdPackage::CheckLineForKeyword(this, "ENDMARKER", &psReference))
                {
                    this->m_bEndMarkerFound_ParseOnly = 1;
                }
                else
                {
                    v7 = psReference;
                    if (I_strnicmp("LANG_", psReference, 5))
                    {
                        return va("Unknown keyword at linestart: \"%s\"", v7);
                    }
                    else
                    {
                        p_m_strCurrentEntryRef_ParseOnly = &this->m_strCurrentEntryRef_ParseOnly;
                        if (this->m_strCurrentEntryRef_ParseOnly._Myres >= 0x10)
                            p_m_strCurrentEntryRef_ParseOnly = (std::string *)p_m_strCurrentEntryRef_ParseOnly->_Bx._Ptr;
                        v9 = p_m_strCurrentEntryRef_ParseOnly->_Bx._Buf[0] == 0;
                        psReference = (const char *)p_m_strCurrentEntryRef_ParseOnly;
                        if (v9)
                        {
                            return "Error parsing file: Unexpected \"LANG_\"";
                        }
                        else
                        {
                            v10 = v7[5];
                            v11 = (unsigned __int8 *)(v7 + 5);
                            for (i = v11; v10; v10 = *++i)
                            {
                                if (v10 == 32)
                                    break;
                                if (v10 == 9)
                                    break;
                            }
                            memset(sThisLanguage, 0, sizeof(sThisLanguage));
                            v13 = i - v11;
                            if (v13 > 0x3FF)
                                v13 = 1023;
                            strncpy((unsigned __int8 *)sThisLanguage, v11, v13);
                            CStringEdPackage::InsideQuotes(this, &v19, (const char *)&v11[strlen(sThisLanguage)]);
                            CStringEdPackage::ConvertCRLiterals_Read(this, &sentence, v19);
                            v14 = (std::string *)sentence._Bx._Ptr;
                            if (sentence._Myres < 0x10)
                                v14 = &sentence;
                            if (!CStringEdPackage::IsStringFormatCorrect(this, v14->_Bx._Buf))
                            {
                                v15 = (std::string *)sentence._Bx._Ptr;
                                if (sentence._Myres < 0x10)
                                    v15 = &sentence;
                                psErrorMessage = va("Illegal string format \"%s\"", v15->_Bx._Buf);
                            }
                            v16 = I_stricmp(sThisLanguage, "english") == 0;
                            if (!psErrorMessage && (v16 || !forceEnglish))
                            {
                                v17 = (std::string *)sentence._Bx._Ptr;
                                if (sentence._Myres < 0x10)
                                    v17 = &sentence;
                                CStringEdPackage::SetString(this, psReference, v17->_Bx._Buf, v16);
                            }
                            std::string::~string(&sentence);
                        }
                    }
                }
            }
        }
        return psErrorMessage;
    }

    int ReadLine(char **psParsePos, char *psDest)
    {
        int v3; // eax
        int v4; // eax
        char v5; // cl
        char *v7; // [esp+28h] [ebp-18h]
        char *v8; // [esp+2Ch] [ebp-14h]
        int iWhiteSpaceScanPos; // [esp+34h] [ebp-Ch]
        unsigned int iCharsToCopy; // [esp+38h] [ebp-8h]

        if (!**psParsePos)
            return 0;
        strchr(*psParsePos, 10);
        if (v3)
        {
            iCharsToCopy = v3 - (_DWORD)*psParsePos;
            strncpy((unsigned __int8 *)psDest, (unsigned __int8 *)*psParsePos, iCharsToCopy);
            psDest[iCharsToCopy] = 0;
            for (*psParsePos += iCharsToCopy; **psParsePos; ++*psParsePos)
            {
                strchr("\r\n", **psParsePos);
                if (!v4)
                    break;
            }
        }
        else
        {
            v8 = *psParsePos;
            v7 = psDest;
            do
            {
                v5 = *v8;
                *v7++ = *v8++;
            } while (v5);
            *psParsePos += strlen(*psParsePos);
        }
        if (*psDest)
        {
            for (iWhiteSpaceScanPos = strlen(psDest) - 1;
                iWhiteSpaceScanPos >= 0 && isspace(psDest[iWhiteSpaceScanPos]);
                --iWhiteSpaceScanPos)
            {
                psDest[iWhiteSpaceScanPos] = 0;
            }
            REMKill(psDest);
        }
        return 1;
    }
    char * Filename_WithoutExt(const char *psFilename)
    {
        char *v2; // eax
        char *v3; // eax
        unsigned int v4; // eax
        char v6; // [esp+3h] [ebp-1Dh]
        char *v7; // [esp+8h] [ebp-18h]
        char *p2; // [esp+18h] [ebp-8h]
        char *p; // [esp+1Ch] [ebp-4h]

        static char sString[64];
        v7 = sString;
        do
        {
            v6 = *psFilename;
            *v7++ = *psFilename++;
        } while (v6);
        strrchr((unsigned __int8 *)sString, '.');
        p = v2;
        strrchr((unsigned __int8 *)sString, '\\');
        p2 = v3;
        strrchr((unsigned __int8 *)sString, '/');
        if (p && (!p2 || p > p2) && (!v4 || (unsigned int)p > v4))
            *p = 0;
        return sString;
    }

    char * Filename_WithoutPath(const char *psFilename)
    {
        static char sString_0[64];

        char v2; // dl
        char *v4; // [esp+8h] [ebp-10h]
        const char *v5; // [esp+Ch] [ebp-Ch]
        const char *psCopyPos; // [esp+14h] [ebp-4h]

        psCopyPos = psFilename;
        while (*psFilename)
        {
            if (*psFilename == '/' || *psFilename == '\\')
                psCopyPos = psFilename + 1;
            ++psFilename;
        }
        v5 = psCopyPos;
        v4 = sString_0;
        do
        {
            v2 = *v5;
            *v4++ = *v5++;
        } while (v2);
        return sString_0;
    }
    void SetupNewFileParase(const char *psFileName)
    {
        const char *v2; // eax
        char v3; // al
        char *v4; // [esp+18h] [ebp-94h]
        char *v5; // [esp+1Ch] [ebp-90h]
        char sString[68]; // [esp+64h] [ebp-48h] BYREF

        v2 = Filename_WithoutExt(psFileName);
        v5 = Filename_WithoutPath(v2);
        v4 = sString;
        do
        {
            v3 = *v5;
            *v4++ = *v5++;
        } while (v3);
        I_strupr(sString);
        m_strCurrentFileRef_ParseOnly = sString;
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &this->m_strCurrentFileRef_ParseOnly,
        //    sString,
        //    strlen(sString));
    }

    int m_bEndMarkerFound_ParseOnly;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > m_strCurrentEntryRef_ParseOnly;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > m_strCurrentEntryEnglish_ParseOnly;
    //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > m_strCurrentFileRef_ParseOnly;
    std::string m_strCurrentEntryRef_ParseOnly;
    std::string m_strCurrentEntryEnglish_ParseOnly;
    std::string m_strCurrentFileRef_ParseOnly;
    //std::map<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t> > m_StringEntries;

    std::map<std::string, mapStringEntriesName_t> m_StringEntries;
};

const char *__cdecl SE_GetString(const char *psPackageAndStringReference);
const char *__cdecl SE_GetString_FastFile(const char *psPackageAndStringReference);
char *__cdecl SE_Load(char *psFileName, bool forceEnglish);
const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference);
void __cdecl SE_NewLanguage();
void __cdecl SE_Init();
void __cdecl SE_ShutDown();
char *__cdecl SE_LoadLanguage(bool forceEnglish);
char *__cdecl SE_GetFoundFile(std::string *strResult);
unsigned __int8 *__cdecl SE_LoadFileData(const char *psFileName);
void __cdecl SE_FreeFileDataAfterLoad(unsigned __int8 *psLoadedFile);
int __cdecl SE_BuildFileList(
    const char *psStartDir,
    std::string *strResults);
void __cdecl SE_R_ListFiles(
    const char *psExtension,
    const char *psDir,
    std::string *strResults);
