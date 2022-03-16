
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CUpdateWeight : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.nn.UpdateWeight"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;

#include "PrepareKernel.hpp"
#include "UpdateWeight.cl"
    };
    
    static void floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(int,0), _KArg(int,1),
                _KArg(float,2), _KArg(float,3),
                _KArg(float*,4), _KArg(float*,5));
    }
    static void doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.doubleEval(
                _KArg(int,0), _KArg(int,1),
                _KArg(double,2), _KArg(double,3),
                _KArg(double*,4), _KArg(double*,5));
    }

public://IKernelOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "doubleEval") == 0 ) return doubleEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CUpdateWeight, CUpdateWeight::__getClassKey())
