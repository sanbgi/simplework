
#include "operator.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorConvert");
class CTensorConvert : public CObject, IKernalOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorConvert"; }

public://IMathOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "int2floatEval") == 0 ) return int2floatEval;
            if( strcmp(szName, "int2doubleEval") == 0 ) return int2doubleEval;
            if( strcmp(szName, "uc2floatEval") == 0 ) return uc2floatEval;
            if( strcmp(szName, "uc2doubleEval") == 0 ) return uc2doubleEval;
        }
        return nullptr;
    }

public:
    struct CKernelWraper {
    public:
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "TensorConvert.cl"
    };

    static void int2floatEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2floatEval(
                _KArg(int,0), _KArg(int*,1),
                _KArg(int,2), _KArg(float*,3));
    }

    static void int2doubleEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2doubleEval(
                _KArg(int,0), _KArg(int*,1),
                _KArg(int,2), _KArg(double*,3));
    }

    static void uc2floatEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uc2floatEval(
                _KArg(int,0), _KArg(unsigned char*,1),
                _KArg(int,2), _KArg(float*,3));
    }

    static void uc2doubleEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uc2doubleEval(
                _KArg(int,0), _KArg(unsigned char*,1),
                _KArg(int,2), _KArg(double*,3));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorConvert, CTensorConvert::__getClassKey())
