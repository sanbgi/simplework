#ifndef __SimpleWork_Math_Cpu_CCpuFactory_h__
#define __SimpleWork_Math_Cpu_CCpuFactory_h__

#include "operator.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CCpuFactory {
public:
    static SOperator createOperator(PID idType, const char* szOp);

};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_Cpu_CCpuFactory_h__