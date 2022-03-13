kernel void ucharEval(global unsigned char* pDest, int iOffsetDest)
{
    int gid = get_global_id(0);
    pDest[gid+iOffsetDest] = 0;
}

