
kernel void intEval(global int * pIn)
{
    int gid = get_global_id(0);
    pIn[gid] = gid;
}
