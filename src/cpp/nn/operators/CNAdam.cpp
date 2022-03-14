
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CNAdam : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.nn.NAdam"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;

#include "PrepareKernel.hpp"
#include "NAdam.cl"
    };
    
    static void floatEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(int,0), _KArg(float,1),  _KArg(float,2),
                _KArg(float*,3), _KArg(float*,4), _KArg(float*,5));
    }
    static void doubleEval(const PKernalCtx* pCtx, int nArgs, PKernalVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.doubleEval(
                _KArg(int,0), _KArg(double,1),  _KArg(double,2),
                _KArg(double*,3), _KArg(double*,4), _KArg(double*,5));
    }

public://IKernalOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
            if( strcmp(szName, "doubleEval") == 0 ) return doubleEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNAdam, CNAdam::__getClassKey())
