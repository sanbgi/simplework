
#include "operator.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorOneHot");
class CTensorOneHot : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorOneHot"; }

public://IMathOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "itofEval") == 0 ) return itofEval;
            if( strcmp(szName, "itodEval") == 0 ) return itodEval;
            if( strcmp(szName, "uctofEval") == 0 ) return uctofEval;
            if( strcmp(szName, "uctodEval") == 0 ) return uctodEval;
        }
        return nullptr;
    }

public:
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "TensorOneHot.cl"
    };

    static void itofEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.itofEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void itodEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.itodEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(double*,4));
    }

    static void uctofEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uctofEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void uctodEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uctodEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(double*,4));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorOneHot, CTensorOneHot::__getClassKey())
