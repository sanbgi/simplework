#include "./math.h"
#include <map>
#include <vector>
#include "CTensor.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensorSolver");
class CTensorSolver : public CObject, ITensorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int solve(const POperator& sOp, int nVars, STensor pVars[]) {
        switch(sOp.id) {

            #define TWO_ONE_ELEWISE(x,y) case POperator::x:{\
                    static PRuntimeKey sKey(y);\
                    return solveEleWise_Two_One(sKey, nVars, pVars);\
                }\
                break;
            TWO_ONE_ELEWISE(plus,"sw.math.TensorPlus")
            TWO_ONE_ELEWISE(minus,"sw.math.TensorMinus")
            TWO_ONE_ELEWISE(multiply,"sw.math.TensorMultiply")
            TWO_ONE_ELEWISE(divide,"sw.math.TensorDivide")

            #define ONE_ONE_ELEWISE(x,y) case POperator::x:{\
                    static PRuntimeKey sKey(y);\
                    return solveEleWise_One_One(sKey, nVars, pVars);\
                }\
                break;
            ONE_ONE_ELEWISE(square,"sw.math.TensorSquare")
            ONE_ONE_ELEWISE(sqrt,"sw.math.TensorSqrt")

            #define ONE_ONE_TOVALUE(x,y) case POperator::x:{\
                    static PRuntimeKey sKey(y);\
                    return solveToValue_One_One(sKey, nVars, pVars);\
                }\
                break;
            ONE_ONE_TOVALUE(sum,"sw.math.TensorSum")
            ONE_ONE_TOVALUE(avg,"sw.math.TensorAvg")

        case POperator::product:{
                static PRuntimeKey sKey("sw.math.TensorDivide");
                return solveEleWise_Two_One(sKey, nVars, pVars);
            }
            break;
        }
        return sCtx.error("不支持的张量运算");
    }

    int solveEleWise_Two_One(PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 3) {
            return sCtx.error("双元操作的参数个数错误");
        }

        STensor sp1 = pVars[0];
        STensor sp2 = pVars[1];
        PDATATYPE type1 = sp1.type();
        PDATATYPE type2 = sp2.type();
        if(type1 != type2) {
            return sCtx.error("两个类型不同的张量不能相加");
        }
        if(sp2.size() < 1) {
            return sCtx.error("无效张量");
        }
        int nSizeIn = sp1.size();
        if( STensor::createTensor(pVars[2], sp1.dimension(), type1, nSizeIn) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }

        int ret = solve(opKey, {1, &nSizeIn}, {0}, 3, pVars);
        if(ret != sCtx.success()) {
            pVars[2].release();
        }
        return ret;
    }

    int solveEleWise_One_One(PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 2) {
            return sCtx.error("单元操作的参数个数错误");
        }

        STensor spIn = pVars[0];
        PDATATYPE type = spIn.type();
        int nSizeIn = spIn.size();
        if( STensor::createTensor(pVars[1], spIn.dimension(), type, nSizeIn) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }

        int ret = solve(opKey, {1, &nSizeIn}, {0}, 2, pVars);
        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }

    int solveToValue_One_One(PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 2) {
            return sCtx.error("单元操作的参数个数错误");
        }

        STensor spIn = pVars[0];
        PDATATYPE type = spIn.type();
        int nSizeIn = spIn.size();
        if( STensor::createVector(pVars[1], type, 1) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }

        int nRange = 0;
        int ret = solve(opKey, {0, &nSizeIn}, {0}, 2, pVars);
        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }

    int solve(  PRuntimeKey kernelKey,
                PVector kernalRange,
                PMemory kernalParameter,
                int nVars, STensor pVars[]) {
        #define __MAX_VARS 8
        if(nVars>__MAX_VARS) {
            return sCtx.error("超过最大求解变量数值");
        }

        int nArgs = nVars*2;
        PVector pData[__MAX_VARS];
        PMemory pArgs[__MAX_VARS*2];
        PMemory* pMemory = pArgs;
        SDevice device = SDevice::defaultDevice();
        for(int i=0; i<nVars; i++, pMemory+=2) {
            if( pVars[i]->getDataInDevice(device, pData[i]) != sCtx.success() ) {
                return sCtx.error("读取张量数据错误");
            }
            pMemory[0].size = sizeof(int);
            pMemory[0].pIntArray = &pData[i].size;
            pMemory[1].size = sizeof(void*);
            pMemory[1].data = &pData[i].pIntArray;
        }
        return device->runKernel(kernelKey, nArgs, pArgs, kernalRange.size, kernalRange.pIntArray);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE