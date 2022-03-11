
#include "operator.h"
#include "math.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CTensorOneHot");
class CTensorOneHot : public CObject, IKernalOperator, ITensorOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernalOperator)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.TensorOneHot"; }

private://IKernalOperator
    FKernalFunc getKernalFunc(const char* szName) {
        if(szName != nullptr) {
            if( strcmp(szName, "itofEval") == 0 ) return itofEval;
            if( strcmp(szName, "itodEval") == 0 ) return itodEval;
            if( strcmp(szName, "uctofEval") == 0 ) return uctofEval;
            if( strcmp(szName, "uctodEval") == 0 ) return uctodEval;
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
                static int s_kernelId = 0;
                ret = spSolver->solve({&s_kernelId, "sw.math.TensorOneHot", "uctofEval"}, {1, &nSizeOut}, {sizeof(nClassify), &nClassify}, 2, pVars);
            }else{
                static int s_kernelId = 0;
                ret = spSolver->solve({&s_kernelId, "sw.math.TensorOneHot", "itofEval"}, {1, &nSizeOut}, {sizeof(nClassify), &nClassify}, 2, pVars);
            }
        }else if( pOneHot->idType == PDATATYPE_DOUBLE) {
            if( type == PDATATYPE_UCHAR ) {
                static int s_kernelId = 0;
                ret = spSolver->solve({&s_kernelId, "sw.math.TensorOneHot", "uctodEval"}, {1, &nSizeOut}, {sizeof(nClassify), &nClassify}, 2, pVars);
            }else{
                static int s_kernelId = 0;
                ret = spSolver->solve({&s_kernelId, "sw.math.TensorOneHot", "itodEval"}, {1, &nSizeOut}, {sizeof(nClassify), &nClassify}, 2, pVars);
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
        const PKernalCtx* pCtx;
        int get_global_id(int i) {
            return pCtx->pRanges[i];
        }

#include "TensorOneHot.cl"
    };

    static void itofEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.itofEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void itodEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.itodEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(int*,2),
                _KArg(int,3), _KArg(double*,4));
    }

    static void uctofEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uctofEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(float*,4));
    }

    static void uctodEval(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) {
        CKernelWraper sKernel = {pCtx};
        sKernel.uctodEval(
                _KArg(int,0),
                _KArg(int,1), _KArg(unsigned char*,2),
                _KArg(int,3), _KArg(double*,4));
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorOneHot, CTensorOneHot::__getClassKey())
