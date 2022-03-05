kernel void zero(global unsigned char* pIn)
{
    pIn[get_global_id(0)] = 0;
}
