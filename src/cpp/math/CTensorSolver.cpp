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
                static PRuntimeKey sKernelKey(y".floatEval");\
                return solveEleWise_Two_One(sKernelKey, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static PRuntimeKey sKernelKey(y".doubleEval");\
                return solveEleWise_Two_One(sKernelKey, nVars, pVars);\
            }\
        }\
    }\
    break;

#define ONE_ONE_ELEWISE(x,y) case PTensorOperator::x:{\
        switch(nVars>0?pVars[0].type():0) {\
        case PDATATYPE_FLOAT:\
            {\
                static PRuntimeKey sKernelKey(y".floatEval");\
                return solveEleWise_One_One(sKernelKey, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static PRuntimeKey sKernelKey(y".doubleEval");\
                return solveEleWise_One_One(sKernelKey, nVars, pVars);\
            }\
        }\
    }\
    break;

#define ONE_ONE_TOVALUE(x,y) case PTensorOperator::x:{\
        switch(nVars>0?pVars[0].type():0) {\
        case PDATATYPE_FLOAT:\
            {\
                static PRuntimeKey sKernelKey(y".floatEval");\
                return solveToValue_One_One(sKernelKey, nVars, pVars);\
            }\
        case PDATATYPE_DOUBLE:\
            {\
                static PRuntimeKey sKernelKey(y".doubleEval");\
                return solveToValue_One_One(sKernelKey, nVars, pVars);\
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
                static PRuntimeKey sKernelKey("sw.math.TensorConvert.uchar2floatEval");
                return solveEleWise_One_One_Type(sKernelKey, nVars, pVars, PDATATYPE_FLOAT);
            }

        case PTensorOperator::toDouble:
            {
                static PRuntimeKey sKernelKey("sw.math.TensorConvert.uchar2doubleEval");
                return solveEleWise_One_One_Type(sKernelKey, nVars, pVars, PDATATYPE_DOUBLE);
            }
        }

        return sCtx.error("????????????????????????");
    }

    int solveEleWise_Two_One(const PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 3) {
            return sCtx.error("?????????????????????????????????");
        }

        STensor sp1 = pVars[0];
        STensor sp2 = pVars[1];
        PDATATYPE type1 = sp1.type();
        PDATATYPE type2 = sp2.type();
        if(type1 != type2) {
            return sCtx.error("???????????????????????????????????????");
        }
        if(sp2.size() < 1) {
            return sCtx.error("????????????");
        }
        int nSizeIn = sp1.size();
        if( STensor::createTensor(pVars[2], sp1.dimension(), type1, nSizeIn) != sCtx.success() ) {
            return sCtx.error("??????????????????");
        }

        int ret = solve(opKey, {0}, 1, &nSizeIn, 3, pVars);
        if(ret != sCtx.success()) {
            pVars[2].release();
        }
        return ret;
    }

    int solveEleWise_One_One(const PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 2) {
            return sCtx.error("?????????????????????????????????");
        }

        STensor spIn = pVars[0];
        PDATATYPE type = spIn.type();
        int nSizeIn = spIn.size();
        if( STensor::createTensor(pVars[1], spIn.dimension(), type, nSizeIn) != sCtx.success() ) {
            return sCtx.error("??????????????????");
        }

        int ret = solve(opKey, {0}, 1, &nSizeIn, 2, pVars);
        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }

    int solveEleWise_One_One_Type(const PRuntimeKey& opKey, int nVars, STensor pVars[], PDATATYPE type) {
        if(nVars != 2) {
            return sCtx.error("?????????????????????????????????");
        }

        STensor spIn = pVars[0];
        if(spIn.type() != PDATATYPE_UCHAR) {
            return sCtx.error("????????????????????????????????????????????????");
        }
        
        int nSizeIn = spIn.size();
        if( STensor::createTensor(pVars[1], spIn.dimension(), type, nSizeIn) != sCtx.success() ) {
            return sCtx.error("??????????????????");
        }

        int ret = solve(opKey, {0}, 1, &nSizeIn, 2, pVars);
        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }

    int solveToValue_One_One(const PRuntimeKey& opKey, int nVars, STensor pVars[]) {
        if(nVars != 2) {
            return sCtx.error("?????????????????????????????????");
        }

        STensor spIn = pVars[0];
        PDATATYPE type = spIn.type();
        int nSizeIn = spIn.size();
        if( STensor::createVector(pVars[1], type, 1) != sCtx.success() ) {
            return sCtx.error("??????????????????");
        }

        int nRange = 0;
        int ret = solve(opKey, {0}, 0, &nSizeIn, 2, pVars);
        if(ret != sCtx.success()) {
            pVars[1].release();
        }
        return ret;
    }
    
    int solve(  PRuntimeKey kernelKey,
                PMemory kernelParameter,
                int nRanges, int pRanges[], 
                int nVars, STensor pVars[]) {

        //????????????
        if( m_arrHookers.size() ) {
            if( (*m_arrHookers.rbegin())->onSolve(kernelKey,kernelParameter,nRanges,pRanges,nVars,pVars) == sCtx.success() ) {
                return sCtx.success();
            }
        }

        if(nVars>__MAX_VARS) {
            return sCtx.error("??????????????????????????????");
        }

        SDevice spKernelDevice = SDevice::defaultKernelDevice();

        PKernelVariable pKernelArgs[__MAX_VARS*2+1];
        int nKernelArgs = 0;
        PKernelVariable* pKernelArg = pKernelArgs;

        //
        // ????????????????????????CPU????????????????????????????????????????????????__MAX_PARAMETER_SIZE??????
        //  ???????????????????????????????????????????????????, ???????????????????????????????????????spKernelParameterInDevice
        //  ????????????????????????????????????????????????????????????????????????
        //
        SKernelMemory spKernelParameterInDevice;
        if(kernelParameter.size>0) {
            //
            // ????????????8?????????????????????CPU???????????????????????????????????????????????????????????????
            //
            if(kernelParameter.size > __MAX_PARAMETER_SIZE) {
                if( spKernelDevice->createKernelMemory(spKernelParameterInDevice, kernelParameter.size, kernelParameter.data) != sCtx.success() ) {
                    return sCtx.error("????????????????????????");
                }
                *pKernelArg = spKernelParameterInDevice.data(spKernelDevice);
            }else{
                for(int i=0; i<kernelParameter.size; i++) {
                    pKernelArg->data[i] = kernelParameter.pByteArray[i];
                    pKernelArg->size = kernelParameter.size;
                }
            }
            nKernelArgs += 1, pKernelArg++;
        }

        for(int i=0; i<nVars; i++) {
            SDeviceMemory spDataBuffer = pVars[i].dataBuffer();
            if( !spDataBuffer ) {
                return sCtx.error("????????????????????????");
            }
            pKernelArg[0] = pVars[i].size();
            pKernelArg[1] = spDataBuffer.data(spKernelDevice);
            nKernelArgs += 2, pKernelArg += 2;
        }

        //???????????????????????????????????????????????????????????????????????????????????????????????????????????????
        return spKernelDevice->runKernel(kernelKey, nKernelArgs, pKernelArgs, nRanges, pRanges);
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
    int runKernel(const SDevice& spKernelDevice,
                        PRuntimeKey kernelKey,
                        PVector kernelRange,
                        PMemory kernelParameter,
                        int nVars,
                        const PVector pVars[] ) {
        if(nVars>__MAX_VARS) {
            return sCtx.error("??????????????????????????????");
        }

        int nArgs = nVars*2;
        PKernelVariable pArgs[__MAX_VARS*2+1];
        PKernelVariable* pMemory = pArgs;
        if(kernelParameter.size>0) {
            nArgs += 1;
            *pMemory = kernelParameter;
            pMemory++;
        }
        for(int i=0; i<nVars; i++, pMemory+=2) {
            pMemory[0] = PKernelVariable(pVars[i].size);
            pMemory[1] = PKernelVariable(pVars[i].data);
        }

        //???????????????????????????????????????????????????????????????????????????????????????????????????????????????
        SDeviceEvent sEvent;
        int ret = spKernelDevice->runKernel(kernelKey, nArgs, pArgs, kernelRange.size, kernelRange.pIntArray, &sEvent);
        if(sEvent) {
            sEvent->wait();
        }
        return ret;
    }
            
    int runDeviaKernel(
                        const SDevice& spKernelDevice,
                        PRuntimeKey kernelKey,
                        PVector kernelRange,
                        PMemory kernelParameter,
                        int nVars,
                        const PDeviaVector pVars[] ) {
                if(nVars>__MAX_VARS) {
            return sCtx.error("??????????????????????????????");
        }

        int nArgs = nVars*3;
        PMemory pArgs[__MAX_VARS*3+1];
        PMemory* pMemory = pArgs;
        if(kernelParameter.size>0) {
            nArgs += 1;
            pMemory->size = kernelParameter.size;
            pMemory->data = kernelParameter.data;
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

        //???????????????????????????????????????????????????????????????????????????????????????????????????????????????
        SDeviceEvent sEvent;
        int ret = spKernelDevice->runKernel(kernelKey, nArgs, pArgs, kernelRange.size, kernelRange.pIntArray, &sEvent);
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