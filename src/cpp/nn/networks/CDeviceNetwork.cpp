
#include "network.h"

#include <map>
#include <vector>
#include <string>
#include <fstream> 
#include <iostream>
using namespace sw;
using namespace std;

static SCtx sCtx("CDeviceNetwork");

class CDeviceNetwork : public CObject, public INnNetwork, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DeviceNetwork"; }

public://INnDeviceNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);
    int update(const STensor& spBatchInDeviation);

private:
    PNnSolveGraph m_sSolveGraph;
    string m_strOptimizer;

private:
    //从计算图中获取的求解信息
    struct PSolveGraphInfos {
        PDATATYPE idType;
        int nElementSize;
        SDimension spInDimension;
        SDimension spOutDimension;

        // 计算变量
        struct PSolveVar {
            ENnVariableType type;
            int size;
            STensor data;
        };
        //解算参数列表
        vector<PSolveVar> arrVars;


        // 计算指令
        struct PSolveInstruct {
            string programName;
            SDeviceMemory spParameters;
            
            int nInVars;
            int pInVars[4];
            int iOutVar;
            
            int nRanges;
            int pRanges[3];

            string evalKernameName;
            string deviaKernameName;
            PID evalKernelId;
            PID deviaKernelId;
        };

        //解算步骤列表
        vector<PSolveInstruct> arrInstructs;

        int iInVar;
        int iOutVar;
        int nWeights;
        SOptimizer spOptimizer;
    };
    CTaker<PSolveGraphInfos*> m_spSolveGraphInfos;

public:
    int initNetwork(PDATATYPE idType);
};

int CDeviceNetwork::__initialize(const PData* pData){
    const PNnNetwork* pNet = CData<PNnNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    if( CNnVariableSolver::solveNetwork(pNet, &m_sSolveGraph) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }
    return sCtx.success();
}

int CDeviceNetwork::initNetwork(PDATATYPE idType) {
    if(m_spSolveGraphInfos) {
        if(idType == m_spSolveGraphInfos->idType) {
            return sCtx.success();
        }else{
            return sCtx.error("数据类型与初始化时不一致，暂不支持");
        }
    }

    string strType;
    int nElementSize;
    switch(idType) {
        case PDATATYPE_FLOAT:
            strType = "float";
            nElementSize = sizeof(float);
            break;

        case PDATATYPE_DOUBLE:
            strType = "double";
            nElementSize = sizeof(double);
            break;

        default:
            return sCtx.error("数据类型还不支持");
    }

    CTaker<PSolveGraphInfos*> taker(new PSolveGraphInfos, [](PSolveGraphInfos* pCtx){
        delete pCtx;
    });
    PSolveGraphInfos& solveCtx = *taker;
    //
    // 更新计算变量数组
    //
    int nWeights = 0;
    vector<PSolveGraphInfos::PSolveVar>& arrVars = solveCtx.arrVars;
    for(auto itVar : m_sSolveGraph.arrVars) {
        SNnInternalVariable spToSolveVar = itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveGraphInfos::PSolveVar solveVar;
        solveVar.size = spToSolveVar->getSize();
        solveVar.type = spToSolveVar->getVariableType();
        if(solveVar.type==ENnVariableType::EVWeight) {
            solveVar.data = spToSolveVar->getData(idType);
            nWeights += solveVar.size;
        }
        arrVars.push_back(solveVar);
    }

    //
    // 更新计算步骤数组
    //
    int nMaxParameterSize = 0;
    vector<PSolveGraphInfos::PSolveInstruct>& arrInstructs = solveCtx.arrInstructs;
    auto itParameter = m_sSolveGraph.arrOperatorArgs.begin();
    auto itOp = m_sSolveGraph.arrOperators.begin();
    while(itParameter != m_sSolveGraph.arrOperatorArgs.end()) {
        PNnOperator spOp = *itParameter;
        PSolveGraphInfos::PSolveInstruct solveParameter;
        PSolveFunc solveFunc;
        (*itOp)->prepareSolver({idType,PSolveCtx::CPU}, solveFunc);
        if(solveFunc.nParamterSize > 0) {
            solveParameter.spParameters = SDeviceMemory::createDeviceMemory(SDevice::cpu(),solveFunc.nParamterSize, solveFunc.pParameterData);
        }
        solveParameter.nInVars = spOp.nInVars;
        solveParameter.iOutVar = spOp.iOutVar;
        memcpy(solveParameter.pInVars, spOp.pInVars, sizeof(int)*spOp.nInVars);
        solveParameter.programName = string("sw.nn.")+(*itOp)->getName();
        solveParameter.evalKernameName = solveParameter.programName+"."+strType+"Eval";
        solveParameter.deviaKernameName = solveParameter.programName+"."+strType+"Devia";
        solveParameter.evalKernelId = PRuntimeKey(solveParameter.evalKernameName.c_str()).runtimeId;
        solveParameter.deviaKernelId = PRuntimeKey(solveParameter.deviaKernameName.c_str()).runtimeId;
        solveParameter.nRanges = 1;
        solveParameter.pRanges[0] = solveParameter.pRanges[1] = solveParameter.pRanges[2] = 0;
        switch(solveFunc.eClRange) {
            case PSolveFunc::PBatch:
                solveParameter.pRanges[0] = -1;
                break;

            case PSolveFunc::PCustomer:
                solveParameter.pRanges[0] = solveFunc.nCustomerRange;
                break;

            case PSolveFunc::POut:
                solveParameter.pRanges[0] = -arrVars[spOp.iOutVar].size;
                break;

            default:
                solveParameter.pRanges[0] = 1;
                break;
        }
        arrInstructs.push_back(solveParameter);
        itParameter++, itOp++;
    }

    //
    // 创建优化器
    //
    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, solveCtx.spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    SDimension spInDimension = m_sSolveGraph.arrVars[m_sSolveGraph.iInVar].dimension();
    SDimension spOutDimension = m_sSolveGraph.arrVars[m_sSolveGraph.iOutVar].dimension();
    solveCtx.iInVar = m_sSolveGraph.iInVar;
    solveCtx.iOutVar = m_sSolveGraph.iOutVar;
    solveCtx.idType = idType;
    solveCtx.nWeights = nWeights;
    solveCtx.nElementSize = nElementSize;
    solveCtx.spInDimension = spInDimension;
    solveCtx.spOutDimension = spOutDimension;
    m_spSolveGraphInfos.take(taker);
    return sCtx.success();
}

int CDeviceNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    PDATATYPE idType = spBatchIn.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;
    int nElementSize = solveCtx.nElementSize;

    SDimension spInDimension = spBatchIn.dimension();
    int nBatchs = *spInDimension.data();
    int nInputSize = spInDimension.dataSize();
    if(nInputSize != nBatchs * solveCtx.spInDimension.dataSize()) {
        return sCtx.error("输入张量尺寸和网络需要的尺寸不匹配");
    }
    
    //
    // 准备求解参数
    //
    struct PSolveVector {
        int size;
        SDeviceMemory dataBuffer;
    };
    vector<PSolveVector> solveVars(solveCtx.arrVars.size());
    {
        PSolveVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
            case ENnVariableType::EVInput:
                pItVec->size = pItVar->size*nBatchs;
                break;
            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->dataBuffer = pItVar->data.dataBuffer();
                break;
            }
        }
    }
    auto pOutVar = solveVars.data() + solveCtx.iOutVar;
    auto pInVar = solveVars.data() + solveCtx.iInVar;
    pInVar->dataBuffer = spBatchIn.dataBuffer();

    SDevice spDevice = SDevice::defaultKernelDevice();
    SDevice spKernelDevice = SDevice::defaultKernelDevice();

    //
    // 遍历计算序列并执行
    //
    PSolveVector* pVec;
    PKernalVariable pKernelArgs[12];
    for(auto& instruct : solveCtx.arrInstructs ) {
        int nKernalArgs = instruct.nInVars*2 + 4;
        PKernalVariable* pMemory = pKernelArgs;
        pMemory[0] = (instruct.spParameters)?instruct.spParameters.data(spKernelDevice):nullptr;
        pMemory[1] = nBatchs;
        pMemory += 2;

        //输入参数
        SDeviceMemory pKernelMemory[4];
        for(int j=0; j<instruct.nInVars; j++) {
            pVec = &solveVars[instruct.pInVars[j]];
            if( spKernelDevice->createKernelMemory(pKernelMemory[j], pVec->dataBuffer) != sCtx.success() ) {
                return sCtx.error("无法创建计算内核需要的参数");
            }
            pMemory[0] = pVec->size;
            pMemory[1] = pKernelMemory[j].data(spKernelDevice);
            pMemory+=2;
        }

        //输出参数
        pVec = &solveVars[instruct.iOutVar];
        SDeviceMemory sKernelOut;
        if( spKernelDevice->createKernelMemory(sKernelOut, pVec->size*nElementSize) != sCtx.success() ) {
            return sCtx.error("创建内核内存错误");
        }
        pMemory[0] = pVec->size;
        pMemory[1] = sKernelOut.data(spKernelDevice);

        //内核计算
        int nRanges = instruct.nRanges;
        int pRanges[3] = { instruct.pRanges[0], instruct.pRanges[1], instruct.pRanges[2] };
        pRanges[0] *= pRanges[0] < 0 ? -nBatchs : 1;
        pRanges[1] *= pRanges[1] < 0 ? -nBatchs : 1;
        pRanges[2] *= pRanges[2] < 0 ? -nBatchs : 1;
        if( spKernelDevice->runKernel(
                {instruct.evalKernelId, instruct.evalKernameName.c_str()}, 
                nKernalArgs, pKernelArgs, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }

        //
        // 这里存在性能问题，因为内核内存刚拷贝回主内存，下一次内核计算，又很快需要这个，所以还要拷贝到设备中
        //
        pVec->dataBuffer = SDeviceMemory::createDeviceMemory(spDevice, sKernelOut);
    }

    std::vector<SObject> arrExtras = {spBatchIn};
    PSolveVector* pItVec = solveVars.data();
    for(auto &itVar : solveCtx.arrVars ){
        if(itVar.type == ENnVariableType::EVOperator && pItVec != pOutVar) {
            arrExtras.push_back(pItVec->dataBuffer);
        }
        pItVec++;
    }
    
    //
    // 准备计算缓冲区
    //
    STensor spOut;
    SDimension spOutDim = solveCtx.spOutDimension.upHighDimension(nBatchs);
    if( STensor::createTensor(spOut, idType, pOutVar->size, spOutDim, pOutVar->dataBuffer ) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }
    return CNnExtraTensor::createResizeTensor({spOut, (int)arrExtras.size(), arrExtras.data()}, spBatchOut);
}

int CDeviceNetwork::devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    int idType = spBatchOut.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(spBatchOutDeviation.type() != idType || spBatchOutDeviation.size() != spBatchOut.size() ) {
        return sCtx.error("偏差数据类型或尺寸错误");
    }

    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;
    int nElementSize = solveCtx.nElementSize;

    SDimension spBatchOutDimension = spBatchOut.dimension();
    int nBatchs = *spBatchOutDimension.data();
    int nOutputSize = spBatchOut.size();
    if(nOutputSize != nBatchs * solveCtx.spOutDimension.dataSize()) {
        return sCtx.error("输出张量尺寸和网络需要的尺寸不匹配");
    }

    SNnExtraTensor spResizeOut = spBatchOut;
    if( !spResizeOut ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    SDevice spDevice = SDevice::defaultKernelDevice();
    SDevice spKernelDevice = SDevice::defaultKernelDevice();

    PNnExtraTensor sResizeTensor;
    if( spResizeOut->getResizeData(sResizeTensor) != sCtx.success() || sResizeTensor.nExtras < 1){
        return sCtx.error("数据错误");
    }
    int nExtras = sResizeTensor.nExtras;
    SObject* pExtras = sResizeTensor.pExtras;
    spBatchIn = *pExtras;
    pExtras++ , nExtras--;
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, spBatchIn.size()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    
    struct PSolveDeviaVector {
        int size;
        SDeviceMemory dataBuffer;
        SDeviceMemory deviaBuffer;
    };

    int nWeights = 0;
    vector<PSolveDeviaVector> solveVars(solveCtx.arrVars.size());
    vector<PSolveDeviaVector*> weightVars;
    {
        PSolveDeviaVector* pItVec = solveVars.data();
        PSolveDeviaVector* pOutVar = solveVars.data() + solveCtx.iOutVar;
        PSolveDeviaVector* pInVar = solveVars.data() + solveCtx.iInVar;
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVInput:
                pItVec->size = pItVar->size*nBatchs;
                break;

            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size*nBatchs;
                if(pItVec != pOutVar) {
                    pItVec->size = pItVar->size*nBatchs;
                    if(nExtras < 1 ) {
                        return sCtx.error("计算数据错误，无法用于传递偏导数");
                    }
                    pItVec->dataBuffer = *pExtras;
                    pExtras++, nExtras--;
                    if(!pItVec->dataBuffer || pItVec->dataBuffer.size() != pItVec->size*nElementSize ) {
                        return sCtx.error("计算数据错误，无法用于传递偏导数");
                    }
                }
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                {
                    pItVec->size = pItVar->size;
                    pItVec->dataBuffer = pItVar->data.dataBuffer();
                    if(pItVar->type == ENnVariableType::EVWeight) {
                        nWeights += pItVec->size;
                        weightVars.push_back(pItVec);
                    }
                }
                break;
            }
        }
        pInVar->dataBuffer = spBatchIn.dataBuffer();
        pOutVar->dataBuffer = spBatchOut.dataBuffer();
        pOutVar->deviaBuffer = spBatchOutDeviation.dataBuffer();
    }
    if(nExtras != 0) {
        return sCtx.error("输入的数据并非合法的计算结果数据，偏导数计算失败");
    }

    //
    // 遍历计算序列并执行
    //
    PSolveDeviaVector* pVec;
    PKernalVariable pKernelArgs[17];
    for(auto itSolver=solveCtx.arrInstructs.rbegin(); itSolver != solveCtx.arrInstructs.rend(); itSolver++) {

        //准备输入输出计算参数
        PSolveGraphInfos::PSolveInstruct& instruct = *itSolver;

        int nKernalArgs = instruct.nInVars*3 + 4;
        PKernalVariable* pMemory = pKernelArgs;
        pMemory[0] = (instruct.spParameters)?instruct.spParameters.data(spKernelDevice):nullptr;
        pMemory[1] = nBatchs;
        pMemory += 2;

        //
        // 创建内核参数
        //
        SDeviceMemory pKernelMemory[4], pKernelDeviaMemory[4];
        for(int j=0; j<instruct.nInVars; j++) {
            pVec = &solveVars[instruct.pInVars[j]];

            void* pDevia;
            if(!pVec->deviaBuffer) {
                pVec->deviaBuffer = SDeviceMemory::createDeviceMemory(spDevice, pVec->size*nElementSize);
                if( !pVec->deviaBuffer ) {
                    return sCtx.error("创建偏导数失败");
                }

                if( spKernelDevice->createKernelMemory(pKernelDeviaMemory[j], pVec->size*nElementSize) != sCtx.success()) {
                    return sCtx.error("创建内核内存失败");
                }
                pDevia = pKernelDeviaMemory[j].data(spKernelDevice);
                spKernelDevice.memoryZero(pDevia, 0, pVec->size* nElementSize);
            }else{
                if( spKernelDevice->createKernelMemory(pKernelDeviaMemory[j], pVec->deviaBuffer) != sCtx.success()) {
                    return sCtx.error("创建内核内存失败");
                }
                pDevia = pKernelDeviaMemory[j].data(spKernelDevice);
            }

            if( spKernelDevice->createKernelMemory(pKernelMemory[j], pVec->dataBuffer) != sCtx.success() ) {
                return sCtx.error("创建内核内存失败");
            }

            pMemory[0] = pVec->size;
            pMemory[1] = pKernelMemory[j].data(spKernelDevice);
            pMemory[2] = pDevia;
            pMemory+=3;
        }
        pVec = &solveVars[instruct.iOutVar];
        SDeviceMemory sKernelOutDevia;
        if( spKernelDevice->createKernelMemory(sKernelOutDevia, pVec->deviaBuffer) != sCtx.success() ) {
            return sCtx.error("创建内核内存失败");
        }
        pMemory[0] = pVec->size;
        pMemory[1] = sKernelOutDevia.data(spKernelDevice);

        int nRanges = instruct.nRanges;
        int pRanges[3] = { instruct.pRanges[0], instruct.pRanges[1], instruct.pRanges[2] };
        pRanges[0] *= pRanges[0] < 0 ? -nBatchs : 1;
        pRanges[1] *= pRanges[1] < 0 ? -nBatchs : 1;
        pRanges[2] *= pRanges[2] < 0 ? -nBatchs : 1;
        if( spKernelDevice->runKernel(
                {instruct.deviaKernelId, instruct.deviaKernameName.c_str()}, 
                nKernalArgs, pKernelArgs, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }

        //
        // 从内核计算结果拷贝回主机设备
        //
        for(int j=0; j<instruct.nInVars; j++) {
            pVec = &solveVars[instruct.pInVars[j]];
            pVec->deviaBuffer->writeMemory(pKernelDeviaMemory[j]);
        }
    }

    //
    // 创建偏导数
    //
    SDeviceMemory spKernelDeviations;
    if( spKernelDevice->createKernelMemory(spKernelDeviations, nWeights*nElementSize) != sCtx.success() ){
        return sCtx.error("创建权重张量失败");
    }

    //
    // 拷贝权重偏导数
    //
    int iDeviationOffset = 0;
    void* pDeviations = spKernelDeviations.data(spKernelDevice);
    for(auto pItVec : weightVars) {

        SMathKernal::equal(spKernelDevice, idType, pDeviations, iDeviationOffset, pItVec->deviaBuffer.data(spKernelDevice), 0, pItVec->size );

        iDeviationOffset += pItVec->size;

        //在将偏导数拷贝以后，可以释放了
        pItVec->deviaBuffer.release();
    }
    if(iDeviationOffset != nWeights) {
        return sCtx.error("未知错误");
    }

    if( solveCtx.spOptimizer->updateDeviation(idType, nBatchs, spKernelDevice, nWeights, pDeviations) != sCtx.success() ){
        return sCtx.error("权重计算异常");
    }

    SObject pExtras2[] = { SDeviceMemory::createDeviceMemory(spDevice, spKernelDeviations) };
    return CNnExtraTensor::createResizeTensor({spBatchInDeviation, 1, pExtras2}, spBatchInDeviation);
}

int CDeviceNetwork::update(const STensor& spBatchInDeviation) {
    int idType = spBatchInDeviation.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    SNnExtraTensor spResizeDevia = spBatchInDeviation;
    if( !spResizeDevia ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnExtraTensor sResizeTensor;
    spResizeDevia->getResizeData(sResizeTensor);
    SDeviceMemory spWeightDevia = sResizeTensor.pExtras[0];

    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;
    if(spWeightDevia.size() != solveCtx.nWeights * solveCtx.nElementSize) {
        return sCtx.error("数据错误，无法用于更新权重");
    }

    SDevice spKernelDevice = SDevice::defaultKernelDevice();
    SDeviceMemory spKernelDeiva;
    if( spKernelDevice->createKernelMemory(spKernelDeiva, spWeightDevia) != sCtx.success() ) {
        return sCtx.error("创建内核计算对象失败");
    }

    int iDeviationOffset = 0;
    void* pWeightDevia = spKernelDeiva.data(spKernelDevice);
    vector<PSolveGraphInfos::PSolveVar>::iterator itVar = solveCtx.arrVars.begin();
    while(itVar != solveCtx.arrVars.end() ) {
        switch(itVar->type) {
        case ENnVariableType::EVWeight:
            {
                SDeviceMemory spWeightBuffer = itVar->data.dataBuffer();

                SDeviceMemory spKernelWeights;
                if( spKernelDevice->createKernelMemory(spKernelWeights, spWeightBuffer) != sCtx.success() ) {
                    return sCtx.error("创建内核计算对象失败");
                }
                
                void* pWeightData = spKernelWeights.data(spKernelDevice);
                switch(solveCtx.idType) {
                    case PDATATYPE_FLOAT:{
                        static PRuntimeKey sKernelKey("sw.nn.UpdateWeight.floatEval");
                        PKernalVariable pArgs[] = {
                            0,
                            iDeviationOffset,
                            -1.0f,
                            1.0f,
                            pWeightData,
                            pWeightDevia
                        };
                        spKernelDevice->runKernel(
                            sKernelKey,
                            6, pArgs,
                            1, &itVar->size);
                    }break;

                    case PDATATYPE_DOUBLE:{
                        static PRuntimeKey sKernelKey("sw.nn.UpdateWeight.doubleEval");
                        PKernalVariable pArgs[] = {
                            0,
                            iDeviationOffset,
                            -1.0f,
                            1.0f,
                            pWeightData,
                            pWeightDevia
                        };
                        spKernelDevice->runKernel(
                            sKernelKey,
                            6, pArgs,
                            1, &itVar->size);

                    }break;
                }
                
                if( spWeightBuffer->writeMemory(spKernelWeights) != sCtx.success() ) {
                    return sCtx.error("拷贝内核权重结果异常");
                }

                iDeviationOffset += itVar->size;
            }
            break;
        }
        itVar++;
    }
    return sCtx.success();
}

int CDeviceNetwork::toArchive(const SArchive& ar) {
    m_sSolveGraph.toArchive(ar);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDeviceNetwork, CDeviceNetwork::__getClassKey())