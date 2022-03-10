
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorAvg");
class CTensorAvg : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorAvg"; }

public://Kernel
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "TensorAvg.cl"
    };
    
public://IKernalOperator
    static void process(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.eval(
                _KArg(int,0), _KArg(float*,1),
                _KArg(int,2), _KArg(float*,3));
    }

    FKernalFunc getKernalFunc(const char* szName) {
        return process;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorAvg, CTensorAvg::__getClassKey())
