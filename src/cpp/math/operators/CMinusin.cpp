
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CMinusin");
class CMinusin : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.Minusin"; }

public://IMathOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "intEval") == 0 ) return intEval;
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "doubleEval") == 0 ) return doubleEval;
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

#include "Minusin.cl"
    };

    static void intEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.intEval(
                _KArg(int*,0), _KArg(int,1),
                _KArg(int*,2), _KArg(int,3));
    }

    static void floatEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(float*,0), _KArg(int,1),
                _KArg(float*,2), _KArg(int,3));
    }

    static void doubleEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.doubleEval(
                _KArg(double*,0), _KArg(int,1),
                _KArg(double*,2), _KArg(int,3));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMinusin, CMinusin::__getClassKey())
