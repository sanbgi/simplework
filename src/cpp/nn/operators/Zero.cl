kernel void ucharEval(global unsigned char* pIn)
{
    pIn[get_global_id(0)] = 0;
}

kernel void intEval(global int* pIn)
{
    pIn[get_global_id(0)] = 0;
}

kernel void floatEval(global float* pIn)
{
    pIn[get_global_id(0)] = 0;
}