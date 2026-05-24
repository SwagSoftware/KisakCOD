#include "md4.h"

#include <cstring>
#include <universal/q_shared.h>

uint8_t PADDING[64] =
{
  128u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u
}; // idb

void __cdecl Encode(uint8_t *output, uint32_t *input, uint32_t len)
{
    uint32_t j; // [esp+0h] [ebp-8h]
    uint32_t i; // [esp+4h] [ebp-4h]

    i = 0;
    for (j = 0; j < len; j += 4)
    {
        *(_WORD *)&output[j] = input[i];
        output[j + 2] = BYTE2(input[i]);
        output[j + 3] = HIBYTE(input[i++]);
    }
}

void __cdecl Decode(uint32_t *output, uint8_t *input, uint32_t len)
{
    uint32_t j; // [esp+0h] [ebp-8h]
    uint32_t i; // [esp+4h] [ebp-4h]

    i = 0;
    for (j = 0; j < len; j += 4)
        output[i++] = (input[j + 3] << 24) | (input[j + 2] << 16) | *(uint16_t *)&input[j];
}

void __cdecl MD4Init(MD4_CTX *context)
{
    context->count[1] = 0;
    context->count[0] = 0;
    context->state[0] = 1732584193;
    context->state[1] = -271733879;
    context->state[2] = -1732584194;
    context->state[3] = 271733878;
}

void __cdecl MD4Update(MD4_CTX *context, uint8_t *input, uint32_t inputLen)
{
    uint32_t index; // [esp+4h] [ebp-8h]
    uint32_t i; // [esp+8h] [ebp-4h]

    index = (context->count[0] >> 3) & 0x3F;
    context->count[0] += 8 * inputLen;
    if (context->count[0] < 8 * inputLen)
        ++context->count[1];
    context->count[1] += inputLen >> 29;
    if (inputLen < 64 - index)
    {
        i = 0;
    }
    else
    {
        memcpy(&context->buffer[index], input, 64 - index);
        MD4Transform(context->state, context->buffer);
        for (i = 64 - index; i + 63 < inputLen; i += 64)
            MD4Transform(context->state, &input[i]);
        index = 0;
    }
    memcpy(&context->buffer[index], &input[i], inputLen - i);
}

void __cdecl MD4Final(uint8_t *digest, MD4_CTX *context)
{
    uint32_t v2; // [esp+0h] [ebp-18h]
    uint8_t bits[8]; // [esp+8h] [ebp-10h] BYREF
    uint32_t index; // [esp+14h] [ebp-4h]

    Encode(bits, context->count, 8u);
    index = (context->count[0] >> 3) & 0x3F;
    if (index >= 0x38)
        v2 = 120 - index;
    else
        v2 = 56 - index;
    MD4Update(context, PADDING, v2);
    MD4Update(context, bits, 8u);
    Encode(digest, context->state, 0x10u);
    memset((uint8_t *)context, 0, sizeof(MD4_CTX));
}

void __cdecl MD4Transform(uint32_t *state, uint8_t *block)
{
    uint32_t c; // [esp+0h] [ebp-50h]
    uint32_t ca; // [esp+0h] [ebp-50h]
    uint32_t cb; // [esp+0h] [ebp-50h]
    uint32_t cc; // [esp+0h] [ebp-50h]
    uint32_t cd; // [esp+0h] [ebp-50h]
    uint32_t ce; // [esp+0h] [ebp-50h]
    uint32_t cf; // [esp+0h] [ebp-50h]
    uint32_t cg; // [esp+0h] [ebp-50h]
    uint32_t ch; // [esp+0h] [ebp-50h]
    uint32_t ci; // [esp+0h] [ebp-50h]
    uint32_t cj; // [esp+0h] [ebp-50h]
    uint32_t ck; // [esp+0h] [ebp-50h]
    uint32_t cl; // [esp+0h] [ebp-50h]
    uint32_t cm; // [esp+0h] [ebp-50h]
    uint32_t cn; // [esp+0h] [ebp-50h]
    uint32_t co; // [esp+0h] [ebp-50h]
    uint32_t cp; // [esp+0h] [ebp-50h]
    uint32_t cq; // [esp+0h] [ebp-50h]
    uint32_t cr; // [esp+0h] [ebp-50h]
    uint32_t cs; // [esp+0h] [ebp-50h]
    uint32_t ct; // [esp+0h] [ebp-50h]
    uint32_t cu; // [esp+0h] [ebp-50h]
    uint32_t cv; // [esp+0h] [ebp-50h]
    uint32_t cw; // [esp+0h] [ebp-50h]
    uint32_t cx; // [esp+0h] [ebp-50h]
    uint32_t d; // [esp+4h] [ebp-4Ch]
    uint32_t da; // [esp+4h] [ebp-4Ch]
    uint32_t db; // [esp+4h] [ebp-4Ch]
    uint32_t dc; // [esp+4h] [ebp-4Ch]
    uint32_t dd; // [esp+4h] [ebp-4Ch]
    uint32_t de; // [esp+4h] [ebp-4Ch]
    uint32_t df; // [esp+4h] [ebp-4Ch]
    uint32_t dg; // [esp+4h] [ebp-4Ch]
    uint32_t dh; // [esp+4h] [ebp-4Ch]
    uint32_t di; // [esp+4h] [ebp-4Ch]
    uint32_t dj; // [esp+4h] [ebp-4Ch]
    uint32_t dk; // [esp+4h] [ebp-4Ch]
    uint32_t dl; // [esp+4h] [ebp-4Ch]
    uint32_t dm; // [esp+4h] [ebp-4Ch]
    uint32_t dn; // [esp+4h] [ebp-4Ch]
    uint32_t dp; // [esp+4h] [ebp-4Ch]
    uint32_t dq; // [esp+4h] [ebp-4Ch]
    uint32_t dr; // [esp+4h] [ebp-4Ch]
    uint32_t ds; // [esp+4h] [ebp-4Ch]
    uint32_t dt; // [esp+4h] [ebp-4Ch]
    uint32_t du; // [esp+4h] [ebp-4Ch]
    uint32_t dv; // [esp+4h] [ebp-4Ch]
    uint32_t dw; // [esp+4h] [ebp-4Ch]
    uint32_t dx; // [esp+4h] [ebp-4Ch]
    uint32_t dy; // [esp+4h] [ebp-4Ch]
    uint32_t b; // [esp+8h] [ebp-48h]
    uint32_t ba; // [esp+8h] [ebp-48h]
    uint32_t bb; // [esp+8h] [ebp-48h]
    uint32_t bc; // [esp+8h] [ebp-48h]
    uint32_t bd; // [esp+8h] [ebp-48h]
    uint32_t be; // [esp+8h] [ebp-48h]
    uint32_t bf; // [esp+8h] [ebp-48h]
    uint32_t bg; // [esp+8h] [ebp-48h]
    uint32_t bh; // [esp+8h] [ebp-48h]
    uint32_t bi; // [esp+8h] [ebp-48h]
    uint32_t bj; // [esp+8h] [ebp-48h]
    uint32_t bk; // [esp+8h] [ebp-48h]
    uint32_t bl; // [esp+8h] [ebp-48h]
    uint32_t bm; // [esp+8h] [ebp-48h]
    uint32_t bn; // [esp+8h] [ebp-48h]
    uint32_t bo; // [esp+8h] [ebp-48h]
    uint32_t bp; // [esp+8h] [ebp-48h]
    uint32_t bq; // [esp+8h] [ebp-48h]
    uint32_t br; // [esp+8h] [ebp-48h]
    uint32_t bs; // [esp+8h] [ebp-48h]
    uint32_t bt; // [esp+8h] [ebp-48h]
    uint32_t bu; // [esp+8h] [ebp-48h]
    uint32_t bv; // [esp+8h] [ebp-48h]
    uint32_t bw; // [esp+8h] [ebp-48h]
    uint32_t a; // [esp+Ch] [ebp-44h]
    uint32_t aa; // [esp+Ch] [ebp-44h]
    uint32_t ab; // [esp+Ch] [ebp-44h]
    uint32_t ac; // [esp+Ch] [ebp-44h]
    uint32_t ad; // [esp+Ch] [ebp-44h]
    uint32_t ae; // [esp+Ch] [ebp-44h]
    uint32_t af; // [esp+Ch] [ebp-44h]
    uint32_t ag; // [esp+Ch] [ebp-44h]
    uint32_t ah; // [esp+Ch] [ebp-44h]
    uint32_t ai; // [esp+Ch] [ebp-44h]
    uint32_t aj; // [esp+Ch] [ebp-44h]
    uint32_t ak; // [esp+Ch] [ebp-44h]
    uint32_t al; // [esp+Ch] [ebp-44h]
    uint32_t am; // [esp+Ch] [ebp-44h]
    uint32_t an; // [esp+Ch] [ebp-44h]
    uint32_t ao; // [esp+Ch] [ebp-44h]
    uint32_t ap; // [esp+Ch] [ebp-44h]
    uint32_t aq; // [esp+Ch] [ebp-44h]
    uint32_t ar; // [esp+Ch] [ebp-44h]
    uint32_t as; // [esp+Ch] [ebp-44h]
    uint32_t at; // [esp+Ch] [ebp-44h]
    uint32_t au; // [esp+Ch] [ebp-44h]
    uint32_t av; // [esp+Ch] [ebp-44h]
    uint32_t aw; // [esp+Ch] [ebp-44h]
    uint32_t ax; // [esp+Ch] [ebp-44h]
    uint32_t x[16]; // [esp+10h] [ebp-40h] BYREF

    a = *state;
    b = state[1];
    c = state[2];
    d = state[3];
    Decode(x, block, 0x40u);
    aa = a + x[0] + (d & ~b | c & b);
    ab = (aa >> 29) | (8 * aa);
    da = d + x[1] + (c & ~ab | b & ab);
    db = (da >> 25) | (da << 7);
    ca = c + x[2] + (b & ~db | ab & db);
    cb = (ca >> 21) | (ca << 11);
    ba = b + x[3] + (ab & ~cb | db & cb);
    bb = (ba >> 13) | (ba << 19);
    ac = ab + x[4] + (db & ~bb | cb & bb);
    ad = (ac >> 29) | (8 * ac);
    dc = db + x[5] + (cb & ~ad | bb & ad);
    dd = (dc >> 25) | (dc << 7);
    cc = cb + x[6] + (bb & ~dd | ad & dd);
    cd = (cc >> 21) | (cc << 11);
    bc = bb + x[7] + (ad & ~cd | dd & cd);
    bd = (bc >> 13) | (bc << 19);
    ae = ad + x[8] + (dd & ~bd | cd & bd);
    af = (ae >> 29) | (8 * ae);
    de = dd + x[9] + (cd & ~af | bd & af);
    df = (de >> 25) | (de << 7);
    ce = cd + x[10] + (bd & ~df | af & df);
    cf = (ce >> 21) | (ce << 11);
    be = bd + x[11] + (af & ~cf | df & cf);
    bf = (be >> 13) | (be << 19);
    ag = af + x[12] + (df & ~bf | cf & bf);
    ah = (ag >> 29) | (8 * ag);
    dg = df + x[13] + (cf & ~ah | bf & ah);
    dh = (dg >> 25) | (dg << 7);
    cg = cf + x[14] + (bf & ~dh | ah & dh);
    ch = (cg >> 21) | (cg << 11);
    bg = bf + x[15] + (ah & ~ch | dh & ch);
    bh = (bg >> 13) | (bg << 19);
    ai = ah + x[0] + (dh & ch | dh & bh | ch & bh) + 1518500249;
    aj = (ai >> 29) | (8 * ai);
    di = dh + x[4] + (ch & bh | ch & aj | bh & aj) + 1518500249;
    dj = (di >> 27) | (32 * di);
    ci = ch + x[8] + (bh & aj | bh & dj | aj & dj) + 1518500249;
    cj = (ci >> 23) | (ci << 9);
    bi = bh + x[12] + (aj & dj | aj & cj | dj & cj) + 1518500249;
    bj = (bi >> 19) | (bi << 13);
    ak = aj + x[1] + (dj & cj | dj & bj | cj & bj) + 1518500249;
    al = (ak >> 29) | (8 * ak);
    dk = dj + x[5] + (cj & bj | cj & al | bj & al) + 1518500249;
    dl = (dk >> 27) | (32 * dk);
    ck = cj + x[9] + (bj & al | bj & dl | al & dl) + 1518500249;
    cl = (ck >> 23) | (ck << 9);
    bk = bj + x[13] + (al & dl | al & cl | dl & cl) + 1518500249;
    bl = (bk >> 19) | (bk << 13);
    am = al + x[2] + (dl & cl | dl & bl | cl & bl) + 1518500249;
    an = (am >> 29) | (8 * am);
    dm = dl + x[6] + (cl & bl | cl & an | bl & an) + 1518500249;
    dn = (dm >> 27) | (32 * dm);
    cm = cl + x[10] + (bl & an | bl & dn | an & dn) + 1518500249;
    cn = (cm >> 23) | (cm << 9);
    bm = bl + x[14] + (an & dn | an & cn | dn & cn) + 1518500249;
    bn = (bm >> 19) | (bm << 13);
    ao = an + x[3] + (dn & cn | dn & bn | cn & bn) + 1518500249;
    ap = (ao >> 29) | (8 * ao);
    dp = dn + x[7] + (cn & bn | cn & ap | bn & ap) + 1518500249;
    dq = (dp >> 27) | (32 * dp);
    co = cn + x[11] + (bn & ap | bn & dq | ap & dq) + 1518500249;
    cp = (co >> 23) | (co << 9);
    bo = bn + x[15] + (ap & dq | ap & cp | dq & cp) + 1518500249;
    bp = (bo >> 19) | (bo << 13);
    aq = ap + x[0] + (dq ^ cp ^ bp) + 1859775393;
    ar = (aq >> 29) | (8 * aq);
    dr = dq + x[8] + (cp ^ bp ^ ar) + 1859775393;
    ds = (dr >> 23) | (dr << 9);
    cq = cp + x[4] + (bp ^ ar ^ ds) + 1859775393;
    cr = (cq >> 21) | (cq << 11);
    bq = bp + x[12] + (ar ^ ds ^ cr) + 1859775393;
    br = (bq >> 17) | (bq << 15);
    as = ar + x[2] + (ds ^ cr ^ br) + 1859775393;
    at = (as >> 29) | (8 * as);
    dt = ds + x[10] + (cr ^ br ^ at) + 1859775393;
    du = (dt >> 23) | (dt << 9);
    cs = cr + x[6] + (br ^ at ^ du) + 1859775393;
    ct = (cs >> 21) | (cs << 11);
    bs = br + x[14] + (at ^ du ^ ct) + 1859775393;
    bt = (bs >> 17) | (bs << 15);
    au = at + x[1] + (du ^ ct ^ bt) + 1859775393;
    av = (au >> 29) | (8 * au);
    dv = du + x[9] + (ct ^ bt ^ av) + 1859775393;
    dw = (dv >> 23) | (dv << 9);
    cu = ct + x[5] + (bt ^ av ^ dw) + 1859775393;
    cv = (cu >> 21) | (cu << 11);
    bu = bt + x[13] + (av ^ dw ^ cv) + 1859775393;
    bv = (bu >> 17) | (bu << 15);
    aw = av + x[3] + (dw ^ cv ^ bv) + 1859775393;
    ax = (aw >> 29) | (8 * aw);
    dx = dw + x[11] + (cv ^ bv ^ ax) + 1859775393;
    dy = (dx >> 23) | (dx << 9);
    cw = cv + x[7] + (bv ^ ax ^ dy) + 1859775393;
    cx = (cw >> 21) | (cw << 11);
    bw = bv + x[15] + (ax ^ dy ^ cx) + 1859775393;
    *state += ax;
    state[1] += (bw >> 17) | (bw << 15);
    state[2] += cx;
    state[3] += dy;
    memset((uint8_t *)x, 0, sizeof(x));
}



void __cdecl Com_BlockChecksum128Cat(
    uint8_t *buffer0,
    uint32_t length0,
    uint8_t *buffer1,
    uint32_t length1,
    uint8_t *outChecksum)
{
    MD4_CTX ctx; // [esp+0h] [ebp-60h] BYREF

    MD4Init(&ctx);
    MD4Update(&ctx, buffer0, length0);
    MD4Update(&ctx, buffer1, length1);
    MD4Final(outChecksum, &ctx);
}

void __cdecl Com_BlockChecksum128(uint8_t *buffer, uint32_t length, int key, uint8_t *outChecksum)
{
    MD4_CTX ctx; // [esp+0h] [ebp-60h] BYREF

    MD4Init(&ctx);
    MD4Update(&ctx, (unsigned char*)&key, 4u);
    MD4Update(&ctx, buffer, length);
    MD4Final(outChecksum, &ctx);
}