#include <d3dx9.h>
#include <d3dx9shader.h>

HRESULT WINAPI D3DXCreateBuffer(DWORD size, ID3DXBuffer **buffer)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DXCompileShader(const char *src_data, UINT data_len, const D3DXMACRO *defines,
        ID3DXInclude *include, const char *function_name, const char *profile, DWORD flags,
        ID3DXBuffer **shader, ID3DXBuffer **error_messages, ID3DXConstantTable **constant_table)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DXGetShaderConstantTable(const DWORD *function, ID3DXConstantTable **constant_table)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DXGetShaderInputSemantics(const DWORD *function, D3DXSEMANTIC *semantics, UINT *count)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DXGetShaderOutputSemantics(const DWORD *function, D3DXSEMANTIC *semantics, UINT *count)
{
    return E_NOTIMPL;
}
