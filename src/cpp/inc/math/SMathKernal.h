#ifndef __SimpleWork_MathKernal_h__
#define __SimpleWork_MathKernal_h__

SIMPLEWORK_MATH_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(MathKernal)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IMathKernal", 211202)
    SIMPLEWORK_INTERFACE_LEAVE
    
    static int plusEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernalVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.intEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.floatEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.doubleEval"}, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    static int minusEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernalVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.intEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.floatEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.doubleEval"}, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(MathKernal)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_MathKernal_h__