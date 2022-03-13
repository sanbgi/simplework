
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CMemoryCopy : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.device.MemoryCopy"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "MemoryCopy.cl"
    };
    
    static void ucharEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.ucharEval(
                _KArg(unsigned char*,0), _KArg(int,1),
                _KArg(unsigned char*,2), _KArg(int,3));
    }

public://IKernalOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "ucharEval") == 0 ) return ucharEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMemoryCopy, CMemoryCopy::__getClassKey())
