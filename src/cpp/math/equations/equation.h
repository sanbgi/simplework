#ifndef __SimpleWork_Math_Equeations_equation_h__
#define __SimpleWork_Math_Equeations_equation_h__

#include "../math.h"
#include "math.h"
#include "map"

using namespace sw;
using namespace std;


typedef void (*FEval)(void* pParameter, int nInVar, const PVector inVars[], PVector outVar);
typedef void (*FDevia)(void* pParameter, int nInVar, const PDeviaVector inVars[], PDeviaVector outVar);

#define BASIC_EVAL_ENTRIES \
    {CBasicData<char>::getStaticType(), eval<char>},\
    {CBasicData<int>::getStaticType(), eval<int>},\
    {CBasicData<unsigned int>::getStaticType(), eval<unsigned int>},\
    {CBasicData<long>::getStaticType(), eval<long>},\
    {CBasicData<double>::getStaticType(), eval<double>},\
    {CBasicData<float>::getStaticType(), eval<float>},\

#endif//__SimpleWork_Math_Equeations_equation_h__