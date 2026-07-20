// Fake d3d9.h for the Android port: the zone/database code only stores
// pointers to D3D interfaces inside asset structs, so opaque types suffice.
#pragma once

#include "kisak_win_shim.h"

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DBaseTexture9;
struct IDirect3DTexture9;
struct IDirect3DCubeTexture9;
struct IDirect3DVolumeTexture9;
struct IDirect3DSurface9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexDeclaration9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DQuery9;

enum D3DFORMAT : uint32_t {
    D3DFMT_UNKNOWN = 0,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_A8 = 28,
    D3DFMT_A8B8G8R8 = 32,
    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_D16 = 80,
    D3DFMT_D24S8 = 75,
    D3DFMT_D24X8 = 77,
    D3DFMT_DXT1 = 0x31545844,
    D3DFMT_DXT3 = 0x33545844,
    D3DFMT_DXT5 = 0x35545844,
};

enum D3DCUBEMAP_FACES : uint32_t {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5,
};
