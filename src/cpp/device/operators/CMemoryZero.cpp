
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CMemoryZero : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.device.MemoryZero"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "MemoryZero.cl"
    };
    
    static void ucharEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.ucharEval(
                _KArg(unsigned char*,0), _KArg(int,1));
    }

public://IKernelOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "ucharEval") == 0 ) return ucharEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMemoryZero, CMemoryZero::__getClassKey())
