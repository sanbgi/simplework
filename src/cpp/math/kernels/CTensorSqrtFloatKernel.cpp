
#include "kernel.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorSqrtFloatKernel");
class CTensorSqrtFloatKernel : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorSqrtFloatKernel"; }

public://IMathOperator
    int process(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        struct CKernelWraper {
        public:
            const PKernalCtx* pCtx;
            int get_global_id(int i) {
                return pCtx->pRanges[i];
            }

#include "CTensorSqrtFloatKernel.cl"
        }sKernel = {pCtx};
        sKernel.eval(
                _KArg(int)(pArgs[0]), _KArg(float*)(pArgs[1]),
                _KArg(int)(pArgs[2]), _KArg(float*)(pArgs[3]));
        return sCtx.success();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSqrtFloatKernel, CTensorSqrtFloatKernel::__getClassKey())
