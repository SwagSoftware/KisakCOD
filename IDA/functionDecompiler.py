#Shitty and pasted IDA Python for decompiling targeted Functions by name

import idautils
import idaapi 
import idc
import ida_hexrays
import ida_lines
import ida_funcs
import ida_kernwin
import ida_ida

import sys
import os
import shutil

# Windows users must escape reverse slash in path: \\
outputPath: str = "C:\\Users\\M1911\\Documents\\kisak-COD"
functionsPath: str = os.path.join(outputPath, "functions")

def remove_last_occurrence(text, char_to_remove):
    """Removes the last occurrence of a character from a string.

    Args:
        text: The string to modify.
        char_to_remove: The character to remove.

    Returns:
        The modified string, or the original string if the character is not found.
    """
    last_index = text.rfind(char_to_remove)

    if last_index == -1:
        return text  # Character not found, return original string
    else:
        return text[:last_index] + text[last_index + 1:]

if outputPath == "":
    ErrorLogger("'outputPath' variable has not been set!")
    sys.exit(1)

# --- function definitions ---
def main() -> None:
    decompglob: str = ""
    decompheaderglob: str = ""
    inputlines: str = ""
    with open('C:\\Users\\M1911\\Documents\\kisak-COD\\targetfuncs.txt') as file:
        inputlines = file.readlines()
        
    for line in inputlines:
        IDAConsolePrint("%s" % line)
    
    initHexraysPlugin() 

    IDAConsolePrint("[!] Starting... [!] \n")

    global functionsPath
    realAndSanitizedFunctionNameMapping: dict = {}
    functionCounter: int = 1

    for func in idautils.Functions():
        funcName: str = idc.get_func_name(func)
        funcNameStripped: str = re.sub(r"\s+", "", funcName)
        demangled_name = idc.demangle_name(funcNameStripped, idc.get_inf_attr(idc.INF_SHORT_DEMNAMES))
        
        if demangled_name is None:
            demangled_name = funcNameStripped
        
        demangled_name = demangled_name.split("(")[0]
        
        shouldDump = False
        
        for line in inputlines:
            lineStripped: str = re.sub(r"\s+", "", line)
            lineStripped = lineStripped.replace('_', '')
            demangled_and_detarded: str = demangled_name.replace('_DONE', '').replace('_', '')
            if lineStripped == demangled_and_detarded:
                IDAConsolePrint("found str: " + lineStripped + " in " + demangled_name)
                shouldDump = True
                break
        
        
        if not shouldDump:
            continue
        
        try:
            IDAConsolePrint(f"[{functionCounter}] Decompiling --> {funcName} ({functionsPath}) \n")

            pseudoCodeOBJ: ida_pro.strvec_t = decompileFunction(func)
            pseudoCodeString = pseudoCodeObjToString(pseudoCodeOBJ)
            decompglob += pseudoCodeString.replace('_DONE', '').replace("BOOL", "bool").replace ('__usercall', '').replace('_DWORD', 'unsigned int').replace('DWORD ', 'unsigned int').replace('LONG', 'int').replace('_SL_', 'SL_').replace('_Sys_', 'Sys_').replace('Profile_', '//Profile_').replace('__noreturn', '').replace('PbCaptureConsoleOutput', '//PbCaptureConsoleOutput').replace('&String;', '"";').replace('&String,', '"",').replace('qmemcpy(', 'memcpy(').replace('abs32(', 'abs(')
            decompglob += "\n"
            
            first_line = pseudoCodeString.split("{")[0].replace('_DONE', '').replace("BOOL", "bool").replace('__usercall', '').replace('_DWORD', 'unsigned int').replace('DWORD ', 'unsigned int').replace('LONG', 'int').replace('_SL_', 'SL_').replace('_Sys_', 'Sys_').replace('Profile_', '//Profile_').replace('__noreturn', '').replace('PbCaptureConsoleOutput', '//PbCaptureConsoleOutput').replace('&String;', '"";').replace('&String', '"",').replace('qmemcpy(', 'memcpy(').replace('abs32(', 'abs(')
            decompheaderglob += remove_last_occurrence(first_line, "\n")
            decompheaderglob += ";\n"

            functionCounter += 1
            del pseudoCodeOBJ

        except Exception as e:
            exceptionLogger(e)
            functionCounter -= 1            

    IDAConsolePrint(f"[!] Successfully decompiled %s functions! [!]" % str(functionCounter - 1))
    with open('C:\\Users\\M1911\\Documents\\kisak-COD\\decompout.txt', "w") as outfile:
        outfile.write("%s" % decompglob)
        
    with open('C:\\Users\\M1911\\Documents\\kisak-COD\\decompheaderglob.txt', "w") as outfile:
        outfile.write("%s" % decompheaderglob)

def initHexraysPlugin() -> None:
    if not ida_hexrays.init_hexrays_plugin():
        errorLogger(f"[Error] hexrays (version %s) failed to init \n" % ida_hexrays.get_hexrays_version())
        sys.exit(1)
    else:   
        IDAConsolePrint(f"[*] Hex-rays version %s has been detected \n" % ida_hexrays.get_hexrays_version())


def IDAConsolePrint(message: str) -> None:
    ida_kernwin.msg(message)


def parseIlegalChars(stringToParse: str) -> str:
    ilegalChars: tuple = ("/", "\\", ":", "<", ">", "|", "?", ",", ".", "&")
    parsedString: str = stringToParse   

    for char in ilegalChars:
        if char in stringToParse:
            parsedString = stringToParse.replace(char, "-")
    return parsedString


def decompileFunction(func: int) -> ida_pro.strvec_t:
    try:
        decompiledFunc: ida_hexrays.cfuncptr_t = ida_hexrays.decompile(func);
        pseudoCodeOBJ: ida_pro.strvec_t = decompiledFunc.get_pseudocode()

        return pseudoCodeOBJ

    except Exception as e:
        raise e
    

def pseudoCodeObjToString(pseudoCodeOBJ: ida_pro.strvec_t) -> str:
    convertedObj: str = ""

    for lineOBJ in pseudoCodeOBJ:
        convertedObj += (ida_lines.tag_remove(lineOBJ.line) + "\n")

    return convertedObj


def dumpPseudocodeToRespectiveFile(pseudoCode: str, filename: str) -> None:
    global functionsPath
    sanitizedFilename: str = parseIlegalChars(filename)

    with open(os.path.join(functionsPath, sanitizedFilename), "a") as f:
        f.write(pseudoCode)
        


def dumpToFileRealAndSanitizedFunctionNamesMapping(mapping: dict) -> None:

    try:
        with open(os.path.join(outputPath, "1 - nameMap.txt"), "a") as f:
            for key in mapping:
                f.write(f"{key} : {mapping[key]} \n")
    except Exception as e:
        exceptionLogger(e)


def exceptionLogger(exception: str) -> None:
    IDAConsolePrint(f"[EXCEPTION]:'%s' \n" % str(exception))
    with open(os.path.join(outputPath, "0 - ERROR_LOG.txt"), "a") as f:
        f.write(f"[EXCEPTION]:'%s' \n" % str(exception))
        f.write(f"[Exception info]: %s \n \n" % str(sys.exc_info()))


def errorLogger(message: str) -> None:
    IDAConsolePrint(message)
    with open(os.path.join(outputPath, "0 - ERROR_LOG.txt"), "a") as f:
        f.write(message + "\n") 


if __name__ == "__main__":
    main()
