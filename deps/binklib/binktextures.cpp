#define D3D_OVERLOADS
#include <d3d9.h> // change to your 9.0 or up path
#include <d3dx9.h>
#include "binktextures.h"


//
// define USE_PS_1_x if you want Bink to use pixel shader 1.x 
//   shaders - otherwise we use 2.0 shaders.  Use 2.0 when
//   you can - they are about 25% faster.
//

//#define USE_PS_1_x
#ifdef USE_PS_1_x
  #define INSERT_PIXEL_SHADER_VERSION "#define PS_1_x\n"
  #define PIXEL_SHADER_TARGET "ps_1_1"
#else
  #define INSERT_PIXEL_SHADER_VERSION
  #define PIXEL_SHADER_TARGET "ps_2_0"
#endif


//
// define USE_SEPARATE_BLIT_TEXTURE to use one set of system textures
//   to decompress into and another one render with - this is about
//   10% faster on D3D 9, but uses more memory (and 10% faster isn't
//   really necessary, since even low end hardware can do upwards
//   of 1000 fps).  On software renderers, you should never use a
//   separate blit-only texture.  By default, we leave it disabled.
//

//#define USE_SEPARATE_BLIT_TEXTURE


//
// define what type of sampling to use - usually use bilinear, but 
//   if you are using a software renderer, point sampling is faster.
//   

#define SAMPLING_TYPE D3DTEXF_LINEAR
//#define SAMPLING_TYPE D3DTEXF_POINT


//
// pointers to our local vertex and pixel shader
//

static IDirect3DPixelShader9  * YCrCbToRGBNoPixelAlpha = 0;
static IDirect3DPixelShader9  * YCrCbAToRGBA = 0;
static LPDIRECT3DVERTEXDECLARATION9 vertex_decl = 0;

typedef struct POS_TC_VERTEX
{
  F32 sx, sy, sz, rhw;  // Screen coordinates
  F32 tu, tv;           // Texture coordinates
#ifdef USE_PS_1_x
  F32 tu1, tv1;         // PS 1.x requires a separate set of TC 
  F32 tu2, tv2;         //   for each texture
  F32 tu3, tv3;         
#endif
} POS_TC_VERTEX;


//
// Common defines for the pixel shaders
//

#define CommonPixelShaderDefines \
"\
#ifdef PS_1_x\n\
\
#define TEX2D( value ) tex2D( tex##value, In.T##value )\n\
#define EXTRA_TEXCOORD( value ) float2 T##value : TEXCOORD##value;\n\
#define CONSTANT_SCALE( value ) ( ( (value) > 2.0 ) ? 1.0 : ( (value) / 2.0 ) )\n\
#define RESCALE( value ) value += value\n\
\
#else\n\
\
#define TEX2D( value ) tex2D( tex##value, In.T0 )\n\
#define EXTRA_TEXCOORD( value )\n\
#define CONSTANT_SCALE( value ) value\n\
#define RESCALE( value )\n\
\
#endif\n\
"

//
// simple pixel shader to apply the yuvtorgb matrix
//

static const char StrYCrCbToRGBNoPixelAlpha[] =
INSERT_PIXEL_SHADER_VERSION \
CommonPixelShaderDefines \
"\
sampler tex0   : register( s0 );\
sampler tex1   : register( s1 );\
sampler tex2   : register( s2 );\
float4  consta : register( c0 );\
\
\
struct VS_OUT\
{\
  float2 T0: TEXCOORD0; EXTRA_TEXCOORD( 1 ) EXTRA_TEXCOORD( 2 )\
};\
\
\
float4 main( VS_OUT In ) : COLOR\
{\
  const float4 crc = { CONSTANT_SCALE( 1.595794678f ), -CONSTANT_SCALE( 0.813476563f ), 0, 0.0 };\
  const float4 crb = { 0, -CONSTANT_SCALE( 0.391448975f ), CONSTANT_SCALE( 2.017822266f ), 0.0 };\
  const float4 adj = { -CONSTANT_SCALE( 0.87065506f ), CONSTANT_SCALE( 0.529705048f ), -CONSTANT_SCALE( 1.081668854f ), 0 };\
  float4 p;\
\
  float y = TEX2D( 0 ).a;\
  float cr = TEX2D( 1 ).a;\
  float cb = TEX2D( 2 ).a;\
\
  p = y * CONSTANT_SCALE( 1.164123535f );\
\
  p += crc * cr;\
  p += crb * cb;\
  p += adj;\
\
  RESCALE( p );\
\
  p.w = 1.0;\
  p *= consta;\
  return p;\
}\
";


//
// simple pixel shader to apply the yuvtorgb matrix with alpha
//

static const char StrYCrCbAToRGBA[] =
INSERT_PIXEL_SHADER_VERSION \
CommonPixelShaderDefines \
"\
sampler tex0   : register( s0 );\
sampler tex1   : register( s1 );\
sampler tex2   : register( s2 );\
sampler tex3   : register( s3 );\
float4  consta : register( c0 );\
\
\
struct VS_OUT\
{\
  float2 T0: TEXCOORD0; EXTRA_TEXCOORD( 1 ) EXTRA_TEXCOORD( 2 ) EXTRA_TEXCOORD( 3 )\
};\
\
\
float4 main( VS_OUT In ) : COLOR\
{\
  const float4 crc = { CONSTANT_SCALE( 1.595794678f ), -CONSTANT_SCALE( 0.813476563f ), 0, 0.0 };\
  const float4 crb = { 0, -CONSTANT_SCALE( 0.391448975f ), CONSTANT_SCALE( 2.017822266f ), 0.0 };\
  const float4 adj = { -CONSTANT_SCALE( 0.87065506f ), CONSTANT_SCALE( 0.529705048f ), -CONSTANT_SCALE( 1.081668854f ), 0 };\
  float4 p;\
\
  float y = TEX2D( 0 ).a;\
  float cr = TEX2D( 1 ).a;\
  float cb = TEX2D( 2 ).a;\
  float a = TEX2D( 3 ).a;\
\
  p = y * CONSTANT_SCALE( 1.164123535f );\
\
  p += crc * cr;\
  p += crb * cb;\
  p += adj;\
\
  RESCALE( p );\
\
  p.w = a;\
  p *= consta;\
  return p;\
}\
";


static D3DVERTEXELEMENT9 vertex_def[] =
    { { 0, 0,               D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
      { 0, (4*sizeof(F32)), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
#ifdef USE_PS_1_x
      { 0, (4*sizeof(F32))+(2*sizeof(F32)), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
      { 0, (4*sizeof(F32))+(4*sizeof(F32)), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  2 },
      { 0, (4*sizeof(F32))+(6*sizeof(F32)), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  3 },
#endif
      D3DDECL_END() };


//############################################################################
//##                                                                        ##
//## Free the shaders that we use.                                          ##
//##                                                                        ##
//############################################################################

void Free_Bink_shaders( void )
{
  if ( vertex_decl )
  {
    vertex_decl->Release();
    vertex_decl = 0;
  }

  if ( YCrCbToRGBNoPixelAlpha )
  {
    YCrCbToRGBNoPixelAlpha->Release();
    YCrCbToRGBNoPixelAlpha = 0;
  }

  if ( YCrCbAToRGBA )
  {
    YCrCbAToRGBA->Release();
    YCrCbAToRGBA = 0;
  }

}


//############################################################################
//##                                                                        ##
//## Create the three shaders that we use.                                  ##
//##                                                                        ##
//############################################################################

S32 Create_Bink_shaders( LPDIRECT3DDEVICE9 d3d_device )
{
  HRESULT hr;
  ID3DXBuffer* buffer = 0;

  //
  // Define the vertex buffer layout
  //

  if ( vertex_decl == 0 )
  {
    if ( FAILED( d3d_device->CreateVertexDeclaration( vertex_def, &vertex_decl ) ) )
      return( 0 );
  }
  
  //
  // create a pixel shader that goes from YcRcB to RGB (without alpha)
  //

  if ( YCrCbToRGBNoPixelAlpha == 0 )
  {
    hr = D3DXCompileShader( StrYCrCbToRGBNoPixelAlpha, sizeof( StrYCrCbToRGBNoPixelAlpha ),
                            0, 0, "main", PIXEL_SHADER_TARGET, 0, &buffer, NULL, NULL );
    if ( SUCCEEDED( hr ) )
    {
      hr = d3d_device->CreatePixelShader( (DWORD*) buffer->GetBufferPointer(), &YCrCbToRGBNoPixelAlpha );
      buffer->Release();

      if ( FAILED( hr ) )
      {
        Free_Bink_shaders( );
        return( 0 );
      }
    }
  }

  //
  // create a pixel shader that goes from YcRcB to RGB with an alpha plane
  //

  if ( YCrCbAToRGBA == 0 )
  {
    hr = D3DXCompileShader( StrYCrCbAToRGBA, sizeof( StrYCrCbAToRGBA ),
                            0, 0, "main", PIXEL_SHADER_TARGET, 0, &buffer, NULL, NULL );
    if ( SUCCEEDED( hr ) )
    {
      hr = d3d_device->CreatePixelShader( (DWORD*) buffer->GetBufferPointer(), &YCrCbAToRGBA );
      buffer->Release();

      if ( FAILED( hr ) )
      {
        Free_Bink_shaders( );
        return( 0 );
      }
    }
  }

  return( 1 );
}


//############################################################################
//##                                                                        ##
//## Free the textures that we allocated.                                   ##
//##                                                                        ##
//############################################################################

void Free_Bink_textures( LPDIRECT3DDEVICE9 d3d_device,
                         BINKTEXTURESET * set_textures )
{
  BINKFRAMETEXTURES * abt[] = { &set_textures->textures[ 0 ], &set_textures->textures[ 1 ], &set_textures->tex_draw };
  BINKFRAMETEXTURES * bt;
  int i;

  // Free the texture memory and then the textures directly
  for ( i = 0; i < sizeof( abt )/sizeof( *abt ); ++i )
  {
    bt = abt[ i ];
    if ( bt->Ytexture )
    {
      bt->Ytexture->Release();
      bt->Ytexture = NULL;
    }
    if ( bt->cRtexture )
    {
      bt->cRtexture->Release();
      bt->cRtexture = NULL;
    }
    if ( bt->cBtexture )
    {
      bt->cBtexture->Release();
      bt->cBtexture = NULL;
    }
    if ( bt->Atexture )
    {
      bt->Atexture->Release();
      bt->Atexture = NULL;
    }
  }
}


//############################################################################
//##                                                                        ##
//## Create a texture while allocating the memory ourselves.                ##
//##                                                                        ##
//############################################################################

static S32 make_texture( LPDIRECT3DDEVICE9 d3d_device,
                         U32 width, U32 height, DWORD usage, D3DFORMAT format, D3DPOOL pool, U32 pixel_size,
                         LPDIRECT3DTEXTURE9 * out_texture, void ** out_ptr, U32 * out_pitch, U32 * out_size )
{
  LPDIRECT3DTEXTURE9 texture = NULL;

  //
  // Create a dynamic texture.
  //

  if ( SUCCEEDED( d3d_device->CreateTexture(width, height, 1,
                                            usage, format, pool,
                                            &texture, 0 ) ) )
  {
    *out_texture = texture;
    *out_size = width * height * pixel_size;

    if ( out_ptr && out_pitch )
    {
      D3DLOCKED_RECT lr;

      if ( FAILED( texture->LockRect( 0, &lr, 0, 0 ) ) )
        return( 0 );

      *out_pitch = lr.Pitch;
      *out_ptr = lr.pBits;
      texture->UnlockRect( 0 );
    }

    return( 1 );
  }

  //
  // Failed
  //

  if (texture)
  {
    texture->Release();
  }

  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Create 2 sets of textures for Bink to decompress into...               ##
//## Also does some basic sampler and render state init                     ##
//##                                                                        ##
//############################################################################

S32 Create_Bink_textures( LPDIRECT3DDEVICE9 d3d_device,
                          BINKTEXTURESET * set_textures )
{
  BINKFRAMEBUFFERS * bb;
  BINKFRAMETEXTURES * bt;
  int i;

  //
  // Create our system decompress textures
  //

  bb = &set_textures->bink_buffers;

  for ( i = 0; i < set_textures->bink_buffers.TotalFrames; ++i )
  {
    bt = &set_textures->textures[ i ];
    bt->Ytexture = 0;
    bt->cBtexture = 0;
    bt->cRtexture = 0;
    bt->Atexture = 0;

    #ifdef USE_SEPARATE_BLIT_TEXTURE
      #define POOL_TYPE D3DPOOL_SYSTEMMEM
    #else
      #define POOL_TYPE D3DPOOL_MANAGED
    #endif

    // Create Y plane
    if ( bb->Frames[ i ].YPlane.Allocate )
    {
      if ( !make_texture( d3d_device,
                          bb->YABufferWidth, bb->YABufferHeight,
                          0, D3DFMT_A8, POOL_TYPE, 1,
                          &bt->Ytexture,
                          &bb->Frames[ i ].YPlane.Buffer,
                          &bb->Frames[ i ].YPlane.BufferPitch,
                          &bt->Ysize ) )
        goto fail;
    }

    // Create cR plane
    if ( bb->Frames[ i ].cRPlane.Allocate )
    {
      if ( !make_texture( d3d_device,
                          bb->cRcBBufferWidth, bb->cRcBBufferHeight,
                          0, D3DFMT_A8, POOL_TYPE, 1,
                          &bt->cRtexture,
                          &bb->Frames[ i ].cRPlane.Buffer,
                          &bb->Frames[ i ].cRPlane.BufferPitch,
                          &bt->cRsize ) )
        goto fail;
    }

    // Create cB plane
    if ( bb->Frames[ i ].cBPlane.Allocate )
    {
      if ( !make_texture( d3d_device,
                          bb->cRcBBufferWidth, bb->cRcBBufferHeight,
                          0, D3DFMT_A8, POOL_TYPE, 1,
                          &bt->cBtexture,
                          &bb->Frames[ i ].cBPlane.Buffer,
                          &bb->Frames[ i ].cBPlane.BufferPitch,
                          &bt->cBsize ) )
        goto fail;
    }

    // Create alpha plane
    if ( bb->Frames[ i ].APlane.Allocate )
    {
      if ( !make_texture( d3d_device,
                          bb->YABufferWidth, bb->YABufferHeight,
                          0, D3DFMT_A8, POOL_TYPE, 1,
                          &bt->Atexture,
                          &bb->Frames[ i ].APlane.Buffer,
                          &bb->Frames[ i ].APlane.BufferPitch,
                          &bt->Asize ) )
        goto fail;
    }

    #undef POOL_TYPE
  }

#ifdef USE_SEPARATE_BLIT_TEXTURE

  //
  // Create our output draw texture (this should be in video card memory)
  //

  bt = &set_textures->tex_draw;
  bt->Ytexture = 0;
  bt->cBtexture = 0;
  bt->cRtexture = 0;
  bt->Atexture = 0;

  // Create Y plane
  if ( bb->Frames[ 0 ].YPlane.Allocate )
  {
    if ( !make_texture( d3d_device,
                        bb->YABufferWidth, bb->YABufferHeight,
                        D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, 1,
                        &bt->Ytexture,
                        0, 0,
                        &bt->Ysize ) )
      goto fail;
  }

  // Create cR plane
  if ( bb->Frames[ 0 ].cRPlane.Allocate )
  {
    if ( !make_texture( d3d_device,
                        bb->cRcBBufferWidth, bb->cRcBBufferHeight,
                        D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, 1,
                        &bt->cRtexture,
                        0, 0,
                        &bt->cRsize ) )
      goto fail;
  }

  // Create cB plane
  if ( bb->Frames[ 0 ].cBPlane.Allocate )
  {
    if ( !make_texture( d3d_device,
                        bb->cRcBBufferWidth, bb->cRcBBufferHeight,
                        D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, 1,
                        &bt->cBtexture,
                        0, 0,
                        &bt->cBsize ) )
      goto fail;
  }

  // Create alpha plane
  if ( bb->Frames[ 0 ].APlane.Allocate )
  {
    if ( !make_texture( d3d_device,
                        bb->YABufferWidth, bb->YABufferHeight,
                        D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, 1,
                        &bt->Atexture,
                        0, 0,
                        &bt->Asize ) )
      goto fail;
  }

#endif

  return( 1 );

fail:

  Free_Bink_textures( d3d_device, set_textures );
  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Lock Bink textures for use by D3D.                                     ##
//##                                                                        ##
//############################################################################

void Lock_Bink_textures( BINKTEXTURESET * set_textures )
{
  BINKFRAMETEXTURES * bt;
  BINKFRAMEPLANESET * bp;
  D3DLOCKED_RECT lr;
  int frame_cur;
  int i;

  //
  // Lock the frame textures
  //

  bt = set_textures->textures;
  bp = set_textures->bink_buffers.Frames;

  frame_cur = set_textures->bink_buffers.FrameNum;

  for ( i = 0; i < set_textures->bink_buffers.TotalFrames; ++i, ++bt, ++bp )
  {
    U32 lock_flags = ( i == frame_cur ) ? 0 : D3DLOCK_READONLY;

    if ( SUCCEEDED( bt->Ytexture->LockRect( 0, &lr, NULL, lock_flags ) ) )
    {
      bp->YPlane.Buffer = lr.pBits;
      bp->YPlane.BufferPitch = lr.Pitch;
    }

    if ( SUCCEEDED( bt->cRtexture->LockRect( 0, &lr, NULL, lock_flags ) ) )
    {
      bp->cRPlane.Buffer = lr.pBits;
      bp->cRPlane.BufferPitch = lr.Pitch;
    }

    if ( SUCCEEDED( bt->cBtexture->LockRect( 0, &lr, NULL, lock_flags ) ) )
    {
      bp->cBPlane.Buffer = lr.pBits;
      bp->cBPlane.BufferPitch = lr.Pitch;
    }

    if ( bt->Atexture )
    {
      //
      // Lock the alpha texture
      //

      if ( SUCCEEDED( bt->Atexture->LockRect( 0, &lr, NULL, lock_flags ) ) )
      {
        bp->APlane.Buffer = lr.pBits;
        bp->APlane.BufferPitch = lr.Pitch;
      }
    }
  }
}


//############################################################################
//##                                                                        ##
//## Unlock Bink textures for use by D3D.                                   ##
//##                                                                        ##
//############################################################################

void Unlock_Bink_textures( LPDIRECT3DDEVICE9 d3d_device, BINKTEXTURESET * set_textures, HBINK Bink )
{
  BINKFRAMETEXTURES * bt;
  BINKFRAMEPLANESET * bp;
  int i;

  //
  // Unlock the frame textures
  //

  bt = set_textures->textures;
  bp = set_textures->bink_buffers.Frames;

  for ( i = 0; i < set_textures->bink_buffers.TotalFrames; ++i, ++bt, ++bp )
  {
    bt->Ytexture->UnlockRect( 0 );
    bp->YPlane.Buffer = NULL;

    bt->cRtexture->UnlockRect( 0 );
    bp->cRPlane.Buffer = NULL;

    bt->cBtexture->UnlockRect( 0 );
    bp->cBPlane.Buffer = NULL;

    if ( bt->Atexture )
    {
      //
      // Unlock the alpha texture
      //

      bt->Atexture->UnlockRect( 0 );
      bp->APlane.Buffer = NULL;
    }
  }

#ifdef USE_SEPARATE_BLIT_TEXTURE

  //
  // Update the pixels on the drawing texture
  //

  S32 num_rects;
  BINKFRAMETEXTURES * bt_dst;
  BINKFRAMETEXTURES * bt_src;

  bt_src = &set_textures->textures[ set_textures->bink_buffers.FrameNum ];
  bt_dst = &set_textures->tex_draw;

  num_rects = BinkGetRects( Bink, BINKSURFACEFAST );
  if ( num_rects > 0 )
  {
    BINKRECT * brc;
    RECT rc;
    S32 i;

    for ( i = 0; i < num_rects; ++i )
    {
      brc = &Bink->FrameRects[ i ];

      rc.left = brc->Left;
      rc.top = brc->Top;
      rc.right = rc.left + brc->Width;
      rc.bottom = rc.top + brc->Height;
      bt_src->Ytexture->AddDirtyRect( &rc );
      if ( bt_src->Atexture )
      {
        bt_src->Atexture->AddDirtyRect( &rc );
      }

      rc.left >>= 1;
      rc.top >>= 1;
      rc.right >>= 1;
      rc.bottom >>= 1;
      bt_src->cRtexture->AddDirtyRect( &rc );
      bt_src->cBtexture->AddDirtyRect( &rc );
    }

    d3d_device->UpdateTexture( bt_src->Ytexture, bt_dst->Ytexture);
    d3d_device->UpdateTexture( bt_src->cRtexture, bt_dst->cRtexture);
    d3d_device->UpdateTexture( bt_src->cBtexture, bt_dst->cBtexture);

    if ( bt_src->Atexture )
    {
      d3d_device->UpdateTexture( bt_src->Atexture, bt_dst->Atexture);
    }
  }

#endif
}


//############################################################################
//##                                                                        ##
//## Draw our textures onto the screen with our vertex and pixel shaders.   ##
//##                                                                        ##
//############################################################################

void Draw_Bink_textures( LPDIRECT3DDEVICE9 d3d_device,
                         BINKTEXTURESET * set_textures,
                         U32 width,
                         U32 height,
                         F32 x_offset,
                         F32 y_offset,
                         F32 x_scale,
                         F32 y_scale,
                         F32 alpha_level,
                         S32 is_premultiplied_alpha )
{
  POS_TC_VERTEX vertices[ 4 ];
  BINKFRAMEPLANESET * bp;
  BINKFRAMETEXTURES * bt_draw;
  BINKFRAMETEXTURES * bt_src;
  int i;
  F32 ac[ 4 ];

  ac[ 0 ] = ( is_premultiplied_alpha ) ? alpha_level : 1.0f;
  ac[ 1 ] = ac [ 0 ];
  ac[ 2 ] = ac [ 0 ];
  ac[ 3 ] = alpha_level;

  bp = &set_textures->bink_buffers.Frames[ set_textures->bink_buffers.FrameNum ];
  bt_src = &set_textures->textures[ set_textures->bink_buffers.FrameNum ];

#ifdef USE_SEPARATE_BLIT_TEXTURE
  bt_draw = &set_textures->tex_draw;
#else
  bt_draw = bt_src;
#endif

  //
  // Turn on texture filtering and texture clamping
  //

  for( i = 0 ; i < 4 ; i++ )
  {
    d3d_device->SetSamplerState( i, D3DSAMP_MAGFILTER, SAMPLING_TYPE );
    d3d_device->SetSamplerState( i, D3DSAMP_MINFILTER, SAMPLING_TYPE );
    d3d_device->SetSamplerState( i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    d3d_device->SetSamplerState( i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    d3d_device->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
  }

  //
  // turn off Z buffering, culling, and projection (since we are drawing orthographically)
  //

  d3d_device->SetRenderState( D3DRS_ZENABLE, FALSE );
  d3d_device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
  d3d_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  //
  // Set the textures.
  //

  d3d_device->SetTexture( 0, bt_draw->Ytexture );
  d3d_device->SetTexture( 1, bt_draw->cRtexture );
  d3d_device->SetTexture( 2, bt_draw->cBtexture );

  //
  // upload the fixed alpha amount
  //
  
  d3d_device->SetPixelShaderConstantF( 0, ac, 1 );

  //
  // are we using an alpha plane? if so, turn on the 4th texture and set our pixel shader
  //

  if ( bt_src->Atexture )
  {
    //
    // Update and set the alpha texture
    //

    d3d_device->SetTexture( 3, bt_draw->Atexture );

    //
    // turn on our pixel shader
    //

    d3d_device->SetPixelShader( YCrCbAToRGBA );

    goto do_alpha;
  }
  else
  {
    //
    // turn on our pixel shader
    //

    d3d_device->SetPixelShader( YCrCbToRGBNoPixelAlpha );
  }

  //
  // are we completely opaque or somewhat transparent?
  //

  if ( alpha_level >= 0.999f )
  {
    d3d_device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
  }
  else
  {
  do_alpha:
    d3d_device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
    d3d_device->SetRenderState( D3DRS_SRCBLEND, ( is_premultiplied_alpha ) ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
    d3d_device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  }

  //
  // Setup up the vertices.
  //

  d3d_device->SetVertexDeclaration( vertex_decl );

  vertices[ 0 ].sx = x_offset;
  vertices[ 0 ].sy = y_offset;
  vertices[ 0 ].sz = 0.0f;
  vertices[ 0 ].rhw = 1.0f;
  vertices[ 0 ].tu = 0.0f;
  vertices[ 0 ].tv = 0.0f;
#ifdef USE_PS_1_x
  vertices[ 0 ].tu1 = 0.0f;
  vertices[ 0 ].tv1 = 0.0f;
  vertices[ 0 ].tu2 = 0.0f;
  vertices[ 0 ].tv2 = 0.0f;
  vertices[ 0 ].tu3 = 0.0f;
  vertices[ 0 ].tv3 = 0.0f;
#endif
  vertices[ 1 ] = vertices[ 0 ];
  vertices[ 1 ].sx = x_offset + ( ( (F32)(S32) width ) * x_scale );
  vertices[ 1 ].tu = 1.0f;
#ifdef USE_PS_1_x
  vertices[ 1 ].tu1 = 1.0f;
  vertices[ 1 ].tu2 = 1.0f;
  vertices[ 1 ].tu3 = 1.0f;
#endif
  vertices[ 2 ] = vertices[0];
  vertices[ 2 ].sy = y_offset + ( ( (F32)(S32) height ) * y_scale );
  vertices[ 2 ].tv = 1.0f;
#ifdef USE_PS_1_x
  vertices[ 2 ].tv1 = 1.0f;
  vertices[ 2 ].tv2 = 1.0f;
  vertices[ 2 ].tv3 = 1.0f;
#endif
  vertices[ 3 ] = vertices[ 1 ];
  vertices[ 3 ].sy = vertices[ 2 ].sy;
  vertices[ 3 ].tv = 1.0f;
#ifdef USE_PS_1_x
  vertices[ 3 ].tv1 = 1.0f;
  vertices[ 3 ].tv2 = 1.0f;
  vertices[ 3 ].tv3 = 1.0f;
#endif

  //
  // Draw the vertices.
  //

  d3d_device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof( vertices[ 0 ] ) );

  //
  // unselect the textures
  //

  d3d_device->SetTexture( 0, 0 );
  d3d_device->SetTexture( 1, 0 );
  d3d_device->SetTexture( 2, 0 );
  d3d_device->SetTexture( 3, 0 );
}

