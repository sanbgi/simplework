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

    template<typename Q>
    static int plusEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.intEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_FLOAT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.floatEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_DOUBLE:
            return spDevice->runKernel({&sSetKernalId, "sw.math.PlusEqual.doubleEvalV"}, 3, pArgs, 1, &nElements);
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

    template<typename Q>
    static int minusEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.intEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_FLOAT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.floatEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_DOUBLE:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MinusEqual.doubleEvalV"}, 3, pArgs, 1, &nElements);
        };
        return -1;
    }

    static int multiplyEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernalVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.intEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.floatEval"}, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static int sSetKernalId = 0;
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.doubleEval"}, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    template<typename Q>
    static int multiplyEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.intEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_FLOAT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.floatEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_DOUBLE:
            return spDevice->runKernel({&sSetKernalId, "sw.math.MultiplyEqual.doubleEvalV"}, 3, pArgs, 1, &nElements);
        };
        return -1;
    }

    template<typename Q>
    static int equal(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {{pDest}, {iDestOffset}, {v} };
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.Equal.intEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_FLOAT:
            return spDevice->runKernel({&sSetKernalId, "sw.math.Equal.floatEvalV"}, 3, pArgs, 1, &nElements);
        case PDATATYPE_DOUBLE:
            return spDevice->runKernel({&sSetKernalId, "sw.math.Equal.doubleEvalV"}, 3, pArgs, 1, &nElements);
        };
        return -1;
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(MathKernal)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_MathKernal_h__