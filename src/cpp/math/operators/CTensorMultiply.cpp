
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorMultiply");
class CTensorMultiply : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorMultiply"; }

public://IMathOperator
    FKernalFunc getKernalFunc(const char* szName) {
        return process;
    }

    static void process(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        struct CKernelWraper {
        public:
            const PKernalCtx* pCtx;
            int get_global_id(int i) {
                return pCtx->pRanges[i];
            }

#include "TensorMultiply.cl"
        }sKernel = {pCtx};
        sKernel.eval(
                _KArg(int,0), _KArg(float*,1),
                _KArg(int,2), _KArg(float*,3),
                _KArg(int,4), _KArg(float*,5));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorMultiply, CTensorMultiply::__getClassKey())
