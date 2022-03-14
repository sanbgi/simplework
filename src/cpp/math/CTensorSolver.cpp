#include "./math.h"
#include <map>
#include <vector>
#include "CTensor.h"
#define __MAX_VARS 8
#define __MAX_PARAMETER_SIZE 8

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensorSolver");

#define TWO_ONE_ELEWISE(x,y) case PTensorOperator::x:{\
        switch(nVars>0?pVars[0].type():0) {\
        case PDATATYPE_FLOAT:\
            {\
                static int sKernelId=0;\
                return solveEleWise_Two_One({&sKernelId, y".floatEval"}, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static int sKernelId=0;\
                return solveEleWise_Two_One({&sKernelId, y".doubleEval"}, nVars, pVars);\
            }\
        }\
    }\
    break;

#define ONE_ONE_ELEWISE(x,y) case PTensorOperator::x:{\
        switch(nVars>0?pVars[0].type():0) {\
        case PDATATYPE_FLOAT:\
            {\
                static int sKernelId=0;\
                return solveEleWise_One_One({&sKernelId, y".floatEval"}, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static int sKernelId=0;\
                return solveEleWise_One_One({&sKernelId, y".doubleEval"}, nVars, pVars);\
            }\
        }\
    }\
    break;

#define ONE_ONE_TOVALUE(x,y) case PTensorOperator::x:{\
        switch(nVars>0?pVars[0].type():0) {\
        case PDATATYPE_FLOAT:\
            {\
                static int sKernelId=0;\
                return solveToValue_One_One({&sKernelId, y".floatEval"}, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static int sKernelId=0;\
                return solveToValue_One_One({&sKernelId, y".doubleEval"}, nVars, pVars);\
            }\
        }\
    }\
    break;

class CTensorSolver : public CObject, ITensorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int solve(const PTensorOperator& sOp, int nVars, STensor pVars[]) {
        switch(sOp.id) {

            TWO_ONE_ELEWISE(plus,"sw.math.TensorPlus")
            TWO_ONE_ELEWISE(minus,"sw.math.TensorMinus")
            TWO_ONE_ELEWISE(multiply,"sw.math.TensorMultiply")
            TWO_ONE_ELEWISE(divide,"sw.math.TensorDivide")

            ONE_ONE_ELEWISE(square,"sw.math.TensorSquare")
            ONE_ONE_ELEWISE(sqrt,"sw.math.TensorSqrt")

            ONE_ONE_TOVALUE(sum,"sw.math.TensorSum")
            ONE_ONE_TOVALUE(avg,"sw.math.TensorAvg")

        case PTensorOperator::toFloat:
            {
                static int sKernelId=0;
                return solveEleWise_One_One_Type({&sKernelId, "sw.math.TensorConvert.uchar2floatEval"}, nVars, pVars, PDATATYPE_FLOAT);
            }

        case PTensorOperator::toDouble:
            {
                static int sKernelId=0;
                return solveEleWise_One_One_Type({&sKernelId, "sw.math.TensorConvert.uchar2doubleEval"}, nVars, pVars, PDATATYPE_DOUBLE);
            }
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

    int solveEleWise_One_One_Type(const PKernalKey& opKey, int nVars, STensor pVars[], PDATATYPE type) {
        if(nVars != 2) {
            return sCtx.error("单元操作的参数个数错误");
        }

        STensor spIn = pVars[0];
        if(spIn.type() != PDATATYPE_UCHAR) {
            return sCtx.error("目前数据格式转化，只支持整数类型");
        }
        
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

        if(nVars>__MAX_VARS) {
            return sCtx.error("超过最大求解变量数值");
        }

        SDevice spDevice = SDevice::cpu();

        PKernalVariable pKernelArgs[__MAX_VARS*2+1];
        int nKernalArgs = 0;
        PKernalVariable* pKernelArg = pKernelArgs;

        //
        // 如果运算设备不是CPU，并且当前的内核计算参数如果大于__MAX_PARAMETER_SIZE个字
        //  节，则需要将计算参数拷贝到设备内存, 注意，这个时候，设备内存由spKernalParameterInDevice
        //  持有，所以，执行必须同步执行，不能异步执行（？）
        //
        SDeviceMemory spKernalParameterInDevice;
        if(kernalParameter.size>0) {
            //
            // 如果大于8个字节，则在非CPU情况下，需要把指针转化为设备指针再传递指针
            //
            if(kernalParameter.size > __MAX_PARAMETER_SIZE) {
                if( spDevice.getPtr() != SDevice::cpu().getPtr() ) {
                    PMemory sKernalMemory;
                    spKernalParameterInDevice = SDeviceMemory::createDeviceMemory(spDevice, kernalParameter.size, kernalParameter.data);
                    if( !spKernalParameterInDevice ) {
                        return sCtx.error("创建设备内存错误");
                    }

                    if(spKernalParameterInDevice->getMemory(spDevice, kernalParameter) != sCtx.success()) {
                        return sCtx.error("获取设备内存错误");
                    }
                    *pKernelArg = PKernalVariable(kernalParameter.data);
                }else{
                    *pKernelArg = PKernalVariable(kernalParameter.data);
                }
            }else{
                for(int i=0; i<kernalParameter.size; i++) {
                    pKernelArg->data[i] = kernalParameter.pByteArray[i];
                    pKernelArg->size = kernalParameter.size;
                }
            }
            nKernalArgs += 1;
            pKernelArg++;
        }

        for(int i=0; i<nVars; i++) {
            PVector sVec;
            if(pVars[i]->getDataInDevice(spDevice,sVec) != sCtx.success()) {
                return sCtx.error("获取张量数据异常");
            }

            pKernelArg[0] = PKernalVariable(sVec.size);
            pKernelArg[1] = PKernalVariable(sVec.data);

            nKernalArgs += 2;
            pKernelArg += 2;
        }

        //目前暂时不支持异步计算，因为还未设计好异步计算时，对于设备内存资源如何管理
        return spDevice->runKernel(kernelKey, nKernalArgs, pKernelArgs, kernalRange.size, kernalRange.pIntArray);
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

    /*
    int runKernel(const SDevice& spDevice,
                        PKernalKey kernelKey,
                        PVector kernalRange,
                        PMemory kernalParameter,
                        int nVars,
                        const PVector pVars[] ) {
        if(nVars>__MAX_VARS) {
            return sCtx.error("超过最大求解变量数值");
        }

        int nArgs = nVars*2;
        PKernalVariable pArgs[__MAX_VARS*2+1];
        PKernalVariable* pMemory = pArgs;
        if(kernalParameter.size>0) {
            nArgs += 1;
            *pMemory = kernalParameter;
            pMemory++;
        }
        for(int i=0; i<nVars; i++, pMemory+=2) {
            pMemory[0] = PKernalVariable(pVars[i].size);
            pMemory[1] = PKernalVariable(pVars[i].data);
        }

        //目前暂时不支持异步计算，因为还未设计好异步计算时，对于设备内存资源如何管理
        SDeviceEvent sEvent;
        int ret = spDevice->runKernel(kernelKey, nArgs, pArgs, kernalRange.size, kernalRange.pIntArray, &sEvent);
        if(sEvent) {
            sEvent->wait();
        }
        return ret;
    }
            
    int runDeviaKernel(
                        const SDevice& spDevice,
                        PKernalKey kernelKey,
                        PVector kernalRange,
                        PMemory kernalParameter,
                        int nVars,
                        const PDeviaVector pVars[] ) {
                if(nVars>__MAX_VARS) {
            return sCtx.error("超过最大求解变量数值");
        }

        int nArgs = nVars*3;
        PMemory pArgs[__MAX_VARS*3+1];
        PMemory* pMemory = pArgs;
        if(kernalParameter.size>0) {
            nArgs += 1;
            pMemory->size = kernalParameter.size;
            pMemory->data = kernalParameter.data;
            pMemory++;
        }
        for(int i=0; i<nVars; i++, pMemory+=3) {
            pMemory[0].size = sizeof(int);
            pMemory[0].pIntArray = (int*)&pVars[i].size;
            pMemory[1].size = sizeof(void*);
            pMemory[1].data = (void*)&pVars[i].data;
            pMemory[2].size = sizeof(void*);
            pMemory[2].data = (void*)&pVars[i].devia;
        }

        //目前暂时不支持异步计算，因为还未设计好异步计算时，对于设备内存资源如何管理
        SDeviceEvent sEvent;
        int ret = spDevice->runKernel(kernelKey, nArgs, pArgs, kernalRange.size, kernalRange.pIntArray, &sEvent);
        if(sEvent) {
            sEvent->wait();
        }
        return ret;
    }*/

private:
    std::vector<STensorHooker> m_arrHookers;
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE