#include "operator.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CCpuFloatMinus : public CObject, IOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int process(int nArgs, PMemory pArgs[]) {
        float *pA = pArgs[0].pFloatArray;
        float *pB = pArgs[1].pFloatArray;
        float *pC = pArgs[2].pFloatArray;
        float *pCEnd = (float*)(pArgs[2].pByteArray + pArgs[2].size);
        while(pC < pCEnd) {
            *pC = *pA - *pB;
            pC++, pA++, pB++;
        }
        return 0;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuFloatMinus, "sw.math.CpuFloatMinus")

SIMPLEWORK_MATH_NAMESPACE_LEAVE