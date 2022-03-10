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
    int solve(const PTensorOperator& sOp, int nVars, STensor pVars[]) {
        switch(sOp.id) {
            #define TWO_ONE_ELEWISE(x,y) case PTensorOperator::x:{\
                    switch(nVars>0?pVars[0].type():0) {\
                    case PDATATYPE_FLOAT:\
                        {\
                            static int sKernelId=0;\
                            return solveEleWise_Two_One({&sKernelId, y, "floatEval"}, nVars, pVars);\
                        }\
                    case PDATATYPE_DOUBLE:\
                        {\
                            static int sKernelId=0;\
                            return solveEleWise_Two_One({&sKernelId, y, "doubleEval"}, nVars, pVars);\
                        }\
                    }\
                }\
                break;
            TWO_ONE_ELEWISE(plus,"sw.math.TensorPlus")
            TWO_ONE_ELEWISE(minus,"sw.math.TensorMinus")
            TWO_ONE_ELEWISE(multiply,"sw.math.TensorMultiply")
            TWO_ONE_ELEWISE(divide,"sw.math.TensorDivide")

            #define ONE_ONE_ELEWISE(x,y) case PTensorOperator::x:{\
                    switch(nVars>0?pVars[0].type():0) {\
                    case PDATATYPE_FLOAT:\
                        {\
                            static int sKernelId=0;\
                            return solveEleWise_One_One({&sKernelId, y, "floatEval"}, nVars, pVars);\
                        }\
                    case PDATATYPE_DOUBLE:\
                        {\
                            static int sKernelId=0;\
                            return solveEleWise_One_One({&sKernelId, y, "doubleEval"}, nVars, pVars);\
                        }\
                    }\
                }\
                break;
            ONE_ONE_ELEWISE(square,"sw.math.TensorSquare")
            ONE_ONE_ELEWISE(sqrt,"sw.math.TensorSqrt")

            #define ONE_ONE_TOVALUE(x,y) case PTensorOperator::x:{\
                    switch(nVars>0?pVars[0].type():0) {\
                    case PDATATYPE_FLOAT:\
                        {\
                            static int sKernelId=0;\
                            return solveToValue_One_One({&sKernelId, y, "floatEval"}, nVars, pVars);\
                        }\
                    case PDATATYPE_DOUBLE:\
                        {\
                            static int sKernelId=0;\
                            return solveToValue_One_One({&sKernelId, y, "doubleEval"}, nVars, pVars);\
                        }\
                    }\
                }\
                break;
            ONE_ONE_TOVALUE(sum,"sw.math.TensorSum")
            ONE_ONE_TOVALUE(avg,"sw.math.TensorAvg")

        case PTensorOperator::product:
            {
                switch(nVars>0?pVars[0].type():0) {
                    case PDATATYPE_FLOAT:
                        {
                            static int sKernelId=0;
                            return solveEleWise_Two_One({&sKernelId, "sw.math.TensorDivide", "floatEval"}, nVars, pVars);
                        }
                    case PDATATYPE_DOUBLE:
                        {
                            static int sKernelId=0;
                            return solveEleWise_Two_One({&sKernelId, "sw.math.TensorDivide", "doubleEval"}, nVars, pVars);
                        }
                }
            }
            break;
        }
        return sCtx.error("不支持的张量运算");
    }

    int solveEleWise_Two_One(const PKernalKey& opKey, int nVars, STensor pVars[]) {
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

    int solveEleWise_One_One(const PKernalKey& opKey, int nVars, STensor pVars[]) {
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

    int solveToValue_One_One(const PKernalKey& opKey, int nVars, STensor pVars[]) {
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

    int solve(  PKernalKey kernelKey,
                PVector kernalRange,
                PMemory kernalParameter,
                int nVars, STensor pVars[]) {

        //钩子机制
        if( m_arrHookers.size() ) {
            if( (*m_arrHookers.rbegin())->onSolve(kernelKey,kernalRange,kernalParameter,nVars,pVars) == sCtx.success() ) {
                return sCtx.success();
            }
        }

        #define __MAX_VARS 8
        if(nVars>__MAX_VARS) {
            return sCtx.error("超过最大求解变量数值");
        }

        SDevice device = SDevice::defaultDevice();

        int nArgs = nVars*2;
        PVector pData[__MAX_VARS];
        PMemory pArgs[__MAX_VARS*2];
        PMemory* pMemory = pArgs;
        for(int i=0; i<nVars; i++, pMemory+=2) {
            if( pVars[i]->getDataInDevice(device, pData[i]) != sCtx.success() ) {
                return sCtx.error("读取张量数据错误");
            }
            pMemory[0].size = sizeof(int);
            pMemory[0].pIntArray = &pData[i].size;
            pMemory[1].size = sizeof(void*);
            pMemory[1].data = &pData[i].pIntArray;
        }

        //目前暂时不支持异步计算，因为还未设计好异步计算时，对于设备内存资源如何管理
        SDeviceEvent sEvent;
        int ret = device->runKernel(kernelKey, nArgs, pArgs, kernalRange.size, kernalRange.pIntArray, &sEvent);
        if(sEvent) {
            sEvent->wait();
        }
        return ret;
    }

    int pushHooker(const STensorHooker& spHooker){
        if(spHooker){
            m_arrHookers.push_back(spHooker);
        }
        return sCtx.success();
    }

    int getHooker(STensorHooker& spHooker) {
        if(m_arrHookers.size() > 0){
            spHooker = *m_arrHookers.rbegin();
            return sCtx.success();
        }
        return sCtx.error();
    }
        
    int popHooker(){
        if(m_arrHookers.size() > 0){
            m_arrHookers.pop_back();
            return sCtx.success();
        }
        return sCtx.error();
    }

private:
    std::vector<STensorHooker> m_arrHookers;
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE