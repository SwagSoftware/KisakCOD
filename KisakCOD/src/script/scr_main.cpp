#include "scr_stringlist.h"

#include <universal/assertive.h>
#include <universal/q_shared.h>

struct scrCompilePub_t
{                                       
    int value_count;                    
    int far_function_count;             
    unsigned int loadedscripts;         
    unsigned int scripts;               
    unsigned int builtinFunc;           
    unsigned int builtinMeth;           
    word canonicalStrings[65536];
    const char *in_ptr;            
    const char *parseBuf;          
    bool script_loading;           
    bool allowedBreakpoint;        
    int developer_statement;       
    byte *opcodePos;    
    unsigned int programLen;       
    int func_table_size;           
    int func_table[1024];          
};
struct __declspec(align(4)) HunkUser 
{
    HunkUser *current;
    HunkUser *next;
    int maxSize;
    int end;
    int pos;
    const char *name;
    bool fixed;
    bool tempMem;
    int type;
    unsigned __int8 buf[1];
};
struct scrVarPub_t // sizeof=0x2007C
{                                       
    const char *fieldBuffer;            
    unsigned __int16 canonicalStrCount; 
    bool developer;                     
    bool developer_script;              
    bool evaluate;                      
    const char *error_message;          
    int error_index;                    
    unsigned int time;                  
    unsigned int timeArrayId;           
    unsigned int pauseArrayId;          
    unsigned int levelId;               
    unsigned int gameId;                
    unsigned int animId;                
    unsigned int freeEntList;           
    unsigned int tempVariable;          
    bool bInited;                       
    unsigned __int16 savecount;
    unsigned int checksum;              
    unsigned int entId;                 
    unsigned int entFieldName;          
    HunkUser *programHunkUser;          
    const char *programBuffer;          
    const char *endScriptBuffer;        
    unsigned __int16 saveIdMap[32768];
    unsigned __int16 saveIdMapRev[32768];
    bool bScriptProfile;               
    float scriptProfileMinTime;        
    bool bScriptProfileBuiltin;        
    float scriptProfileBuiltinMinTime; 
    unsigned int numScriptThreads;     
    unsigned int numScriptValues;      
    unsigned int numScriptObjects;     
    const char *varUsagePos;           
    int ext_threadcount;               
    int totalObjectRefCount;           
    volatile int totalVectorRefCount;  
};

static scrCompilePub_t scrCompilePub;
static scrVarPub_t scrVarPub;

unsigned int SL_TransferToCanonicalString(unsigned int stringValue)
{
	iassert(stringValue);
	SL_TransferRefToUser(stringValue, 2u);

	if (scrCompilePub.canonicalStrings[stringValue])
		return scrCompilePub.canonicalStrings[stringValue];

	scrCompilePub.canonicalStrings[stringValue] = ++scrVarPub.canonicalStrCount;

	return scrVarPub.canonicalStrCount;
}

unsigned int __cdecl SL_GetCanonicalString(char* str)
{
    unsigned int v1; // eax
    unsigned int v3; // eax

    v1 = SL_FindString(str);
    if (scrCompilePub.canonicalStrings[v1])
        return scrCompilePub.canonicalStrings[v1];
    v3 = SL_GetString_(str, 0, 16);
    return SL_TransferToCanonicalString(v3);
}

int SL_BeginLoadScripts()
{
    int result; // eax

    memset((unsigned __int8*)scrCompilePub.canonicalStrings, 0, sizeof(scrCompilePub.canonicalStrings));
    result = 0;
    scrVarPub.canonicalStrCount = 0;
    return result;
}