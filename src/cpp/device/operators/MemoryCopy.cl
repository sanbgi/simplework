kernel void ucharEval(
    global unsigned char* pDest, int iOffsetDest,
    global unsigned char* pSrc, int iOffsetSrc )
{
    int gid = get_global_id(0);
    pDest[gid+iOffsetDest] = pSrc[gid+iOffsetSrc];
}

