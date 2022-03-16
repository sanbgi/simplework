#ifndef __SimpleWork_MathKernel_h__
#define __SimpleWork_MathKernel_h__

SIMPLEWORK_MATH_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(MathKernel)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IMathKernel", 211202)
    SIMPLEWORK_INTERFACE_LEAVE
    
    static int plusEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernelVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.intEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.floatEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.doubleEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    template<typename Q>
    static int plusEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        PKernelVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.intEval");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.floatEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.PlusEqual.doubleEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    static int minusEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernelVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.intEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.floatEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.doubleEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    template<typename Q>
    static int minusEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        PKernelVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.intEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.floatEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.MinusEqual.doubleEval");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    static int multiplyEqual(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernelVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.intEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.floatEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.doubleEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    template<typename Q>
    static int multiplyEqual(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        PKernelVariable pArgs[] = {{pDest}, {iDestOffset}, {v}};
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.intEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.floatEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.MultiplyEqual.doubleEval");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    static int equal(const SDevice& spDevice, PDATATYPE idType, void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nElements) {
        PKernelVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        switch(idType) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.Equal.intEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.Equal.floatEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.Equal.doubleEval");
            return spDevice->runKernel(sKernelKey, 4, pArgs, 1, &nElements);
            }
        };
        return -1;
    }

    template<typename Q>
    static int equal(const SDevice& spDevice, void* pDest, int iDestOffset, Q v, int nElements) {
        PKernelVariable pArgs[] = {{pDest}, {iDestOffset}, {v} };
        switch(CBasicData<Q>::getStaticType()) {
        case PDATATYPE_INT:{
            static PRuntimeKey sKernelKey("sw.math.Equal.intEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_FLOAT:{
            static PRuntimeKey sKernelKey("sw.math.Equal.floatEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        case PDATATYPE_DOUBLE:{
            static PRuntimeKey sKernelKey("sw.math.Equal.doubleEvalV");
            return spDevice->runKernel(sKernelKey, 3, pArgs, 1, &nElements);
            }
        };
        return -1;
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(MathKernel)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_MathKernel_h__