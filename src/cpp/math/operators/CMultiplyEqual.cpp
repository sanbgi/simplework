
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CMultiplyEqual");
class CMultiplyEqual : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.MultiplyEqual"; }

public://IMathOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "intEval") == 0 ) return intEval;
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "doubleEval") == 0 ) return doubleEval;
            if( strcmp(szName, "intEvalV") == 0 ) return intEvalV;
            if( strcmp(szName, "floatEvalV") == 0 ) return floatEvalV;
            if( strcmp(szName, "doubleEvalV") == 0 ) return doubleEvalV;
        }
        return nullptr;
    }

public:
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;

#include "PrepareKernel.hpp"
#include "MultiplyEqual.cl"
    };

    static void intEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.intEval(
                _KArg(int*,0), _KArg(int,1),
                _KArg(int*,2), _KArg(int,3));
    }

    static void floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(float*,0), _KArg(int,1),
                _KArg(float*,2), _KArg(int,3));
    }

    static void doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.doubleEval(
                _KArg(double*,0), _KArg(int,1),
                _KArg(double*,2), _KArg(int,3));
    }

    static void intEvalV(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.intEvalV(
                _KArg(int*,0), _KArg(int,1), _KArg(int,2));
    }

    static void floatEvalV(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEvalV(
                _KArg(float*,0), _KArg(int,1), _KArg(float,2));
    }

    static void doubleEvalV(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.doubleEvalV(
                _KArg(double*,0), _KArg(int,1), _KArg(double,2));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMultiplyEqual, CMultiplyEqual::__getClassKey())
