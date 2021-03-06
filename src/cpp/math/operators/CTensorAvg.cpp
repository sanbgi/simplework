
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CTensorAvg : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorAvg"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;
        
#include "PrepareKernel.hpp"
#include "TensorAvg.cl"
    };
    
    static void floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(int,0), _KArg(float*,1),
                _KArg(int,2), _KArg(float*,3));
    }

public://IKernelOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorAvg, CTensorAvg::__getClassKey())
