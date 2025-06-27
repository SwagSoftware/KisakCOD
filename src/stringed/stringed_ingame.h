#pragma once
#include <string>
#include <map>

#include <qcommon/qcommon.h>

// LWSS: One of the more nasty disliked files.

struct LocalizeName // sizeof=0x0
{                                       // ...
};
struct mapStringEntriesName_t : LocalizeName // sizeof=0x0
{
};

struct CStringEdPackage // sizeof=0x78
{
    void Clear()
    {
        m_StringEntries.clear();
        m_bEndMarkerFound_ParseOnly = 0;
        m_strCurrentEntryRef_ParseOnly = "";
        m_strCurrentEntryEnglish_ParseOnly = "";
    }
    void REMKill(char *psBuffer)
    {
        char *v2; // eax
        int iWhiteSpaceScanPos; // [esp+14h] [ebp-18h]
        int i; // [esp+18h] [ebp-14h]
        int iDoubleQuoteCount; // [esp+1Ch] [ebp-10h]
        int iDoubleQuotesSoFar; // [esp+20h] [ebp-Ch]

        for (iDoubleQuotesSoFar = 0; ; iDoubleQuotesSoFar = iDoubleQuoteCount)
        {
            v2 = strstr(psBuffer, "//");
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

    // LWSS: I left these std::aids in the header file as a "fuck you" to the reader. Enjoy. 
    std::string * InsideQuotes(
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *result,
        std::string *result,
        const char *psLine)
    {
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v4; // [esp+10h] [ebp-11Ch]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v5; // [esp+14h] [ebp-118h]
        //unsigned int v6; // [esp+18h] [ebp-114h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *Ptr; // [esp+28h] [ebp-104h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *p_Bx; // [esp+3Ch] [ebp-F0h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v9; // [esp+40h] [ebp-ECh]
        //unsigned int v10; // [esp+44h] [ebp-E8h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v11; // [esp+54h] [ebp-D8h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v12; // [esp+58h] [ebp-D4h]
        //unsigned int v13; // [esp+5Ch] [ebp-D0h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *v14; // [esp+6Ch] [ebp-C0h]
//        std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > str; // [esp+10Ch] [ebp-20h] BYREF
        std::string str; // [esp+10Ch] [ebp-20h] BYREF

        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&str, 0, 0);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &str,
        //    &String,
        //    strlen(&String));
        str = "";

        while (*psLine == ' ' || *psLine == '\t')
            ++psLine;
        if (*psLine == '"')
            ++psLine;
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &str,
        //    psLine,
        //    strlen(psLine));
        str.assign(psLine);
        if (*psLine)
        {
            while (1)
            {
                //v14 = str._Myres < 0x10 ? &str._Bx : str._Bx._Ptr;
                //v13 = strlen(v14->_Buf);
                //v12 = str._Myres < 0x10 ? &str._Bx : str._Bx._Ptr;
                //if (v12->_Buf[v13 - 1] != 32)
                if (str[str.size() - 1] != ' ')
                {
                    //v11 = str._Myres < 0x10 ? &str._Bx : str._Bx._Ptr;
                    //v10 = strlen(v11->_Buf);
                    //v9 = str._Myres < 0x10 ? &str._Bx : str._Bx._Ptr;
                    //if (v9->_Buf[v10 - 1] != 9)
                    //    break;

                    if (str[str.size() - 1] != 9)
                    {
                        break;
                    }
                }
                //if (str._Myres < 0x10)
                //    p_Bx = &str._Bx;
                //else
                //    p_Bx = str._Bx._Ptr;
                //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::erase(
                //    &str,
                //    strlen(p_Bx->_Buf) - 1,
                //    1u);
                str.erase(str.size() - 1, 1);
            }
            //if (str._Myres < 0x10)
            //    Ptr = &str._Bx;
            //else
            //    Ptr = str._Bx._Ptr;
            //v6 = strlen(Ptr->_Buf);
            //if (str._Myres < 0x10)
            //    v5 = &str._Bx;
            //else
            //    v5 = str._Bx._Ptr;
            //if (v5->_Buf[v6 - 1] == 34)
            if (str[str.size() - 1] == '"')
            {
                //if (str._Myres < 0x10)
                //    v4 = &str._Bx;
                //else
                //    v4 = str._Bx._Ptr;
                str.erase(str.size() - 1, 1);
                //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::erase(
                //    &str,
                //    strlen(v4->_Buf) - 1,
                //    1u);
            }
        }
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(result, 0, 0);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    result,
        //    &str,
        //    0,
        //    std::string::npos);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&str, 1, 0);
        *result = str;
        return result;
    }

    void AddEntry(const char *psLocalReference)
    {
        char *v2; // eax
        char *v3; // eax
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *v5; // [esp+68h] [ebp-140h]
        char *v6; // [esp+F0h] [ebp-B8h]
        //std::_Tree_nod<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::_Node *Myhead; // [esp+104h] [ebp-A4h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > v9; // [esp+140h] [ebp-68h] BYREF
        //std::_Aux_cont *Myownedaux; // [esp+15Ch] [ebp-4Ch]
        //std::_Tree_nod<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::_Node *v11; // [esp+160h] [ebp-48h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > _Keyval; // [esp+164h] [ebp-44h] BYREF
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > SE_Entry; // [esp+180h] [ebp-28h] BYREF
        //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::iterator itEntry; // [esp+1A0h] [ebp-8h] BYREF

        //if (this->m_strCurrentFileRef_ParseOnly._Myres < 0x10)
        //    v2 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Buf, psLocalReference);
        //else
        //    v2 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Ptr, psLocalReference);

        //std::string _Keyval;
        v2 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly.data(), psLocalReference); // _Ptr

        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 0, 0);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &_Keyval,
        //    v2,
        //    strlen(v2));
        //_Keyval.assign(v2);
        //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::find(
        //    &this->m_StringEntries,
        //    &itEntry,
        //    &_Keyval);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 1, 0);
        auto itEntry = this->m_StringEntries.find(v2);

        //Myhead = this->m_StringEntries._Myhead;
        //Myownedaux = 0;
        //v11 = Myhead;
        //if (this == -88)
        //    _invalid_parameter_noinfo();
        //Myownedaux = this->m_StringEntries._Myownedaux;
        //if (!itEntry._Myaux || itEntry._Myaux != Myownedaux)
        //    _invalid_parameter_noinfo();

        //if (itEntry._Ptr == v11)
        if (itEntry == this->m_StringEntries.end())
        {
            std::string SE_Entry;

            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&SE_Entry, 0, 0);
            //if (this->m_strCurrentFileRef_ParseOnly._Myres < 0x10)
            //    v3 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Buf, psLocalReference);
            //else
            //    v3 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Ptr, psLocalReference);
            v3 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);

            //v6 = v3;
            std::string v9;
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&v9, 0, 0);
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(&v9, v6, strlen(v6));
            v9.assign(v3);
            //v5 = std::map<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>>::operator[](
            //    &this->m_StringEntries,
            //    &v9);
            m_StringEntries[v9] = SE_Entry;
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
            //    v5,
            //    &SE_Entry,
            //    0,
            //    std::string::npos);
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&v9, 1, 0);
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&SE_Entry, 1, 0);
        }
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &this->m_strCurrentEntryRef_ParseOnly,
        //    psLocalReference,
        //    strlen(psLocalReference));
        this->m_strCurrentEntryRef_ParseOnly.assign(psLocalReference);
    }

    void SetString(
        const char *psLocalReference,
        const char *psNewString,
        int bSentenceIsEnglish)
    {
        char *v4; // eax
        //std::_Tree_nod<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::_Node *Myhead; // [esp+8Ch] [ebp-6Ch]
        //char *_Ptr; // [esp+BCh] [ebp-3Ch]
        //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::const_iterator _Right; // [esp+C8h] [ebp-30h] BYREF
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > _Keyval; // [esp+D0h] [ebp-28h] BYREF
        //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > >, mapStringEntriesName_t>, 0> >::iterator itEntry; // [esp+ECh] [ebp-Ch] BYREF
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *Entry; // [esp+F4h] [ebp-4h]

        //if (this->m_strCurrentFileRef_ParseOnly._Myres < 0x10)
        //    v4 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Buf, psLocalReference);
        //else
        //    v4 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly._Bx._Ptr, psLocalReference);

        v4 = va("%s_%s", m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);

        //_Ptr = v4;
        
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 0, 0);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &_Keyval,
        //    _Ptr,
        //    strlen(_Ptr));

        //std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::find(
        //    &this->m_StringEntries,
        //    &itEntry,
        //    &_Keyval);

        auto itEntry = this->m_StringEntries.find(v4);

        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&_Keyval, 1, 0);

        //Myhead = this->m_StringEntries._Myhead;
        //_Right._Myaux = 0;
        //_Right._Ptr = Myhead;
        //if (this == -88)
        //    _invalid_parameter_noinfo();
        //_Right._Myaux = this->m_StringEntries._Myownedaux;
        //if (!std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>,
        //    std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>,
        //    std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::const_iterator::operator!=(
        //    &itEntry,
        //    &_Right))
        //    MyAssertHandler(".\\stringed\\stringed_ingame.cpp", 685, 0, "%s", "itEntry != m_StringEntries.end()");

        if (itEntry == m_StringEntries.end())
        {
            MyAssertHandler(".\\stringed\\stringed_ingame.cpp", 685, 0, "%s", "itEntry != m_StringEntries.end()");
        }


        //Entry = &std::_Tree<std::_Tmap_traits<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>,
        //    std::less<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, Allocator<std::pair<std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>, 
        //    std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>>, mapStringEntriesName_t>, 0>>::const_iterator::operator*(&itEntry)->second;

        auto& Entry = (*itEntry).second; // KISAKTODO: ensure this actually ref's and writes

        if (bSentenceIsEnglish)
        {
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
            //    Entry,
            //    psNewString,
            //    strlen(psNewString));

            Entry.assign(psNewString);

            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
            //    &this->m_strCurrentEntryEnglish_ParseOnly,
            //    psNewString,
            //    strlen(psNewString));

            m_strCurrentEntryEnglish_ParseOnly.assign(psNewString);
        }
        else if (I_stricmp(psNewString, "#same"))
        {
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
            //    Entry,
            //    psNewString,
            //    strlen(psNewString));
            Entry.assign(psNewString);
        }
        else
        {
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
            //    Entry,
            //    &this->m_strCurrentEntryEnglish_ParseOnly,
            //    0,
            //    std::string::npos);

            Entry.assign(m_strCurrentEntryEnglish_ParseOnly);
        }
    }

    void ConvertCRLiterals_Read(
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *result,
        std::string *result,
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > string)
        std::string string)
    {
        int v4[2]; // [esp-8h] [ebp-A8h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *p_Bx; // [esp+0h] [ebp-A0h]
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > str; // [esp+7Ch] [ebp-24h] BYREF
        std::string str;
        int loc; // [esp+98h] [ebp-8h]

        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&str, 0, 0);
        str.assign(string);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    &str,
        //    &string,
        //    0,
        //    std::string::npos);
        while (1)
        {
            v4[0] = 0;
            v4[1] = strlen("\\n");
            //loc = std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::find(&str, "\\n", *v4);
            auto loc = str.find("\\n", 0);
            if (loc == std::string::npos)
                break;
            //if (loc > str._Mysize)
            //    _invalid_parameter_noinfo();
            //if (str._Myres < 0x10)
            //    p_Bx = &str._Bx;
            //else
            //    p_Bx = str._Bx._Ptr;
            
            //p_Bx->_Buf[loc] = 10;
            str[loc] = 10;

            str.erase(loc + 1, 1);
            //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::erase(&str, loc + 1, 1u);
        }
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(result, 0, 0);
        *result = str;
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::assign(
        //    result,
        //    &str,
        //    0,
        //    std::string::npos);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&str, 1, 0);
        //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName>>::_Tidy(&string, 1, 0);
        //return result;
    }

    char IsStringFormatCorrect(char *string)
    {
        const char *v2; // eax
        const char *v4; // eax
        CStringEdPackage *thisa; // [esp+0h] [ebp-1Ch]
        int argIndex; // [esp+4h] [ebp-18h]
        const char *convString; // [esp+8h] [ebp-14h]
        bool args[12]; // [esp+Ch] [ebp-10h] BYREF

        thisa = this;
        memset(args, 0, 9);
        for (convString = strstr(string, "&&"); convString; convString = v4)
        {
            convString += 2;
            if (!isdigit(*convString))
                return 0;
            argIndex = *convString - 48;
            if (args[--argIndex])
                return 0;
            args[argIndex] = 1;

            v4 = strstr(++convString, "");
        }
        return 1;
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
        bool v16; // eax
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
            if (CStringEdPackage::CheckLineForKeyword("VERSION", &psReference))
            {
                CStringEdPackage::InsideQuotes(&sentence, psReference);
                //Ptr = (std::string *)sentence._Bx._Ptr;
                //if (sentence._Myres < 0x10)
                //    Ptr = &sentence;
                //v5 = atoi(Ptr->_Bx._Buf);
                v5 = atoi(sentence.data());
                if (v5 != 1)
                    psErrorMessage = va("Unexpected version number %d, expecting %d!", v5, 1);
                //if (sentence._Myres >= 0x10)
                //    operator delete(sentence._Bx._Ptr);
            }
            else if (!CStringEdPackage::CheckLineForKeyword("CONFIG", &psReference)
                && !CStringEdPackage::CheckLineForKeyword("FILENOTES", &psReference)
                && !CStringEdPackage::CheckLineForKeyword("NOTES", &psReference)
                && !CStringEdPackage::CheckLineForKeyword("FLAGS", &psReference))
            {
                if (CStringEdPackage::CheckLineForKeyword("REFERENCE", &psReference))
                {
                    CStringEdPackage::InsideQuotes(&sentence, psReference);
                    //p_sentence = (std::string *)sentence._Bx._Ptr;
                    //if (sentence._Myres < 0x10)
                    //    p_sentence = &sentence;
                    //CStringEdPackage::AddEntry(p_sentence->_Bx._Buf);
                    p_sentence = &sentence;
                    CStringEdPackage::AddEntry(p_sentence->data());
                    //std::string::~string(&sentence);
                }
                else if (CStringEdPackage::CheckLineForKeyword("ENDMARKER", &psReference))
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
                        //if (this->m_strCurrentEntryRef_ParseOnly._Myres >= 0x10)
                        //    p_m_strCurrentEntryRef_ParseOnly = (std::string *)p_m_strCurrentEntryRef_ParseOnly->_Bx._Ptr;
                        //v9 = p_m_strCurrentEntryRef_ParseOnly->_Bx._Buf[0] == 0;
                        v9 = p_m_strCurrentEntryRef_ParseOnly->data()[0] == 0;
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
                            strncpy(sThisLanguage, (char*)v11, v13);
                            CStringEdPackage::InsideQuotes(&v19, (const char *)&v11[strlen(sThisLanguage)]);
                            CStringEdPackage::ConvertCRLiterals_Read(&sentence, v19);
                            //v14 = (std::string *)sentence._Bx._Ptr;
                            //if (sentence._Myres < 0x10)
                            //    v14 = &sentence;
                            v14 = &sentence;
                            //if (!CStringEdPackage::IsStringFormatCorrect(v14->_Bx._Buf))
                            if (!CStringEdPackage::IsStringFormatCorrect(v14->data()))
                            {
                                //v15 = (std::string *)sentence._Bx._Ptr;
                                //if (sentence._Myres < 0x10)
                                //    v15 = &sentence;
                                v15 = &sentence;
                                //psErrorMessage = va("Illegal string format \"%s\"", v15->_Bx._Buf);
                                psErrorMessage = va("Illegal string format \"%s\"", v15->data());
                            }
                            v16 = I_stricmp(sThisLanguage, "english") == 0;
                            if (!psErrorMessage && (v16 || !forceEnglish))
                            {
                                //v17 = (std::string *)sentence._Bx._Ptr;
                                //if (sentence._Myres < 0x10)
                                //    v17 = &sentence;
                                v17 = &sentence;
                                //CStringEdPackage::SetString(psReference, v17->_Bx._Buf, v16);
                                CStringEdPackage::SetString(psReference, v17->data(), v16);
                            }
                            //std::string::~string(&sentence);
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
        v3 = (int)strchr(*psParsePos, 10);
        if (v3)
        {
            iCharsToCopy = v3 - (_DWORD)*psParsePos;
            strncpy(psDest, *psParsePos, iCharsToCopy);
            psDest[iCharsToCopy] = 0;
            for (*psParsePos += iCharsToCopy; **psParsePos; ++*psParsePos)
            {
                v4 = (int)strchr("\r\n", **psParsePos);
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
        char* v4; // eax
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
        p = sString;
        strrchr((unsigned __int8 *)sString, '\\');
        p2 = sString;
        strrchr((unsigned __int8 *)sString, '/');
        v4 = sString;
        if (p && (!p2 || p > p2) && (!v4 || p > v4))
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

    //std::map<std::string, mapStringEntriesName_t> m_StringEntries;
    std::map<std::string, std::string> m_StringEntries;
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
