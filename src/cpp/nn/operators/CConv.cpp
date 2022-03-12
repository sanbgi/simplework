
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CConv : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.nn.Conv"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "Conv.cl"
    };
    
    static void floatEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(CKernelWraper::PConvParameter*,0), 
                _KArg(int,1),
                _KArg(int,2), _KArg(float*,3),
                _KArg(int,4), _KArg(float*,5),
                _KArg(int,6), _KArg(float*,7));
    }

    static void floatDevia(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatDevia(
                _KArg(CKernelWraper::PConvParameter*,0), 
                _KArg(int,1),
                _KArg(int,2), _KArg(float*,3), _KArg(float*,4),
                _KArg(int,5), _KArg(float*,6), _KArg(float*,7),
                _KArg(int,8), _KArg(float*,9));
    }

public://IKernalOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "floatDevia") == 0 ) return floatDevia;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CConv, CConv::__getClassKey())