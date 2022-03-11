kernel void floatEval(global unsigned char* pIn)
{
    pIn[get_global_id(0)] = 0;
}
