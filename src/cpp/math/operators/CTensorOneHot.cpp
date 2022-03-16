
#include "operator.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorOneHot");
class CTensorOneHot : public CObject, IKernelOperator, ITensorOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelOperator)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorOneHot"; }

private://IKernelOperator
    FKernelFunc getKernelFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "int2floatEval") == 0 ) return int2floatEval;
            if( strcmp(szName, "int2doubleEval") == 0 ) return int2doubleEval;
            if( strcmp(szName, "uchar2floatEval") == 0 ) return uchar2floatEval;
            if( strcmp(szName, "uchar2doubleEval") == 0 ) return uchar2doubleEval;
        }
        return nullptr;
    }

private://ITensorOperator
    int solve(const PData* pData, int nVars, STensor pVars[]) {
        if(nVars != 2) {
            return sCtx.error("单元操作的参数个数错误");
        }

        const POneHot* pOneHot = CData<POneHot>(pData);
        if(pOneHot == nullptr) {
            return sCtx.error("缺少必要的参数");
        }

        int nClassify = pOneHot->nClassify;
        if(nClassify > 1000000 || nClassify <= 0) {
            return sCtx.error("分类数不合法，无法生成OneHot张量");
        }

        STensor spIn = pVars[0];
        PDATATYPE type = spIn.type();
        if(type != PDATATYPE_UCHAR && type != PDATATYPE_INT) {
            return sCtx.error("OneHot张量只支持整数类型");
        }

        int nSizeOut = spIn.size()*nClassify;
        if( STensor::createTensor(pVars[1], spIn.dimension().upLowDimension(nClassify), pOneHot->idType, nSizeOut) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }

        int ret = -1;
        STensorSolver spSolver = STensorSolver::getSolver();
        if( pOneHot->idType == PDATATYPE_FLOAT ) {
            if( type == PDATATYPE_UCHAR ) {
                static PRuntimeKey sKernelKey("sw.math.TensorOneHot.uchar2floatEval");
                ret = spSolver->solve(sKernelKey, {sizeof(nClassify), &nClassify}, 1, &nSizeOut, 2, pVars);
            }else{
                static PRuntimeKey sKernelKey("sw.math.TensorOneHot.int2floatEval");
                ret = spSolver->solve(sKernelKey, {sizeof(nClassify), &nClassify}, 1, &nSizeOut, 2, pVars);
            }
        }else if( pOneHot->idType == PDATATYPE_DOUBLE) {
            if( type == PDATATYPE_UCHAR ) {
                static PRuntimeKey sKernelKey("sw.math.TensorOneHot.uchar2doubleEval");
                ret = spSolver->solve(sKernelKey, {sizeof(nClassify), &nClassify}, 1, &nSizeOut, 2, pVars);
            }else{
                static PRuntimeKey sKernelKey("sw.math.TensorOneHot.int2doubleEval");
                ret = spSolver->solve(sKernelKey, {sizeof(nClassify), &nClassify}, 1, &nSizeOut, 2, pVars);
            }
        }

        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }

public:
    struct CKernelWraper {
    public:
        const PKernelCtx* pCtx;

#include "PrepareKernel.hpp"
#include "TensorOneHot.cl"
    };

    static void int2floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2floatEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void int2doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.int2doubleEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(double*,4));
    }

    static void uchar2floatEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uchar2floatEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void uchar2doubleEval(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uchar2doubleEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(double*,4));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorOneHot, CTensorOneHot::__getClassKey())
