
#include "operator.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorMinus");
class CTensorMinus : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorMinus"; }

    static void floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        struct CKernelWraper {
        public:
            const PKernelCtx* pCtx;
            int get_global_id(int i) {
                return pCtx->pRanges[i];
            }

#include "TensorMinus.cl"
        }sKernel = {pCtx};
        sKernel.floatEval(
                _KArg(int,0), _KArg(float*,1),
                _KArg(int,2), _KArg(float*,3),
                _KArg(int,4), _KArg(float*,5));
    }

public://IMathOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "floatEval") == 0 ) return floatEval;
        }
        return nullptr;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorMinus, CTensorMinus::__getClassKey())
