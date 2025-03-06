#pragma once


union HashEntry_unnamed_type_u
{                             
    unsigned int prev;
    unsigned int str;
};

struct HashEntry
{               
    unsigned int status_next;
    HashEntry_unnamed_type_u u;
};

struct __declspec(align(128)) scrStringGlob_t
{                                       
    HashEntry hashTable[20000];         
    bool inited;                        
    HashEntry * nextFreeEntry;
};

 struct $119B815E6C15BED54461C272BD343858
 {                                       
    unsigned __int32 refCount : 16;
    unsigned __int32 user : 8;
    unsigned __int32 byteLen : 8;
 };

 union $62B5D821F4358051071D6AFDBF8DC1EF 
 {                                   
     $119B815E6C15BED54461C272BD343858 subUnion;
     volatile int data;
 };

 struct __declspec(align(4)) RefString
 {
     $62B5D821F4358051071D6AFDBF8DC1EF unionThing;
     char str[1];
 };

#define MT_NODE_SIZE 12
#define MT_SIZE 0xC000

struct scrMemTreePub_t
{                     
    char *mt_buffer;  
};

unsigned int SL_GetString_(const char* str, unsigned int user, int type);
HashEntry_unnamed_type_u SL_GetStringOfSize(const char* str, unsigned int user, unsigned int len, int type);
char* SL_ConvertToString(unsigned int stringValue);
RefString* GetRefString(unsigned int stringValue);

unsigned int SL_FindString(const char* str);
void SL_RemoveRefToString(unsigned int stringValue);
void SL_RemoveRefToStringOfSize(unsigned int stringValue, unsigned int len);