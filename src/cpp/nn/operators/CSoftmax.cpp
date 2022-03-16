
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CSoftmax : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.nn.Softmax"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;

#include "PrepareKernel.hpp"
#include "Softmax.cl"
    };
    
    static void floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(CKernelWraper::PSoftmaxParameter*,0), 
                _KArg(int,1),
                _KArg(int,2), _KArg(float*,3),
                _KArg(int,4), _KArg(float*,5));
    }

    static void floatDevia(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatDevia(
                _KArg(CKernelWraper::PSoftmaxParameter*,0), 
                _KArg(int,1),
                _KArg(int,2), _KArg(float*,3), _KArg(float*,4),
                _KArg(int,5), _KArg(float*,6));
    }

public://IKernelOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "floatDevia") == 0 ) return floatDevia;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CSoftmax, CSoftmax::__getClassKey())
