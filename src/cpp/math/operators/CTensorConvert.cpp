
#include "operator.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorConvert");
class CTensorConvert : public CObject, IKernelOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorConvert"; }

public://IMathOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "int2floatEval") == 0 ) return int2floatEval;
            if( strcmp(szName, "int2doubleEval") == 0 ) return int2doubleEval;
            if( strcmp(szName, "uchar2floatEval") == 0 ) return uchar2floatEval;
            if( strcmp(szName, "uchar2doubleEval") == 0 ) return uchar2doubleEval;
        }
        return nullptr;
    }

public:
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;
        
#include "PrepareKernel.hpp"
#include "TensorConvert.cl"
    };

    static void int2floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2floatEval(
                _KArg(int,0), _KArg(int*,1),
                _KArg(int,2), _KArg(float*,3));
    }

    static void int2doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2doubleEval(
                _KArg(int,0), _KArg(int*,1),
                _KArg(int,2), _KArg(double*,3));
    }

    static void uchar2floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uchar2floatEval(
                _KArg(int,0), _KArg(unsigned char*,1),
                _KArg(int,2), _KArg(float*,3));
    }

    static void uchar2doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uchar2doubleEval(
                _KArg(int,0), _KArg(unsigned char*,1),
                _KArg(int,2), _KArg(double*,3));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorConvert, CTensorConvert::__getClassKey())
