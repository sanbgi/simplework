
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
    template<typename Q>
    int evalT(const STensor& spBatchIn, STensor& spBatchOut);

    template<typename Q>
    int deviaT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

    template<typename Q>
    int updateT(const STensor& spBatchInDeviation);

private:
    PNnSolveGraph m_sSolveGraph;
    string m_strOptimizer;

private:
    //从计算图中获取的求解信息
    struct PSolveGraphInfos {
        PDATATYPE idType;
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

            int evalKernelId;
            int deviaKernelId;
        };

        //解算步骤列表
        vector<PSolveInstruct> arrInstructs;

        int iInVar;
        int iOutVar;
        int nSumSize[EVMax];
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

    CTaker<PSolveGraphInfos*> taker(new PSolveGraphInfos, [](PSolveGraphInfos* pCtx){
        delete pCtx;
    });
    PSolveGraphInfos& solveCtx = *taker;
    for(int i=0; i<EVMax; i++) {
        solveCtx.nSumSize[i] = 0;
    }

    //
    // 更新计算变量数组
    //
    vector<PSolveGraphInfos::PSolveVar>& arrVars = solveCtx.arrVars;
    for(auto itVar : m_sSolveGraph.arrVars) {
        SNnInternalVariable spToSolveVar = itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveGraphInfos::PSolveVar solveVar;
        solveVar.size = spToSolveVar->getSize();
        solveVar.type = spToSolveVar->getVariableType();
        solveVar.data = spToSolveVar->getData(idType);
        solveCtx.nSumSize[solveVar.type] += solveVar.size;
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
        PNnAtomOperatorArgs spOp = *itParameter;
        PSolveGraphInfos::PSolveInstruct solveParameter;
        PSolveFunc solveFunc;
        (*itOp)->prepareSolver({idType,PSolveCtx::CPU}, solveFunc);
        if(solveFunc.nParamterSize > 0) {
            solveParameter.spParameters = SDeviceMemory::createMemory({solveFunc.nParamterSize, solveFunc.pParameterData});
        }
        solveParameter.nInVars = spOp.nInVars;
        solveParameter.iOutVar = spOp.iOutVar;
        memcpy(solveParameter.pInVars, spOp.pInVars, sizeof(int)*spOp.nInVars);
        solveParameter.evalKernelId = solveParameter.deviaKernelId = 0;
        solveParameter.programName = string("sw.nn.")+(*itOp)->getName();
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

    if(idType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }
    return sCtx.error("不支持的数据类型"); 
}

template<typename Q>
int CDeviceNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {
    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;

    SDimension spInDimension = spBatchIn.dimension();
    int nBatchs = *spInDimension.data();
    int nInputSize = spInDimension.dataSize();
    if(nInputSize != nBatchs * solveCtx.spInDimension.dataSize()) {
        return sCtx.error("输入张量尺寸和网络需要的尺寸不匹配");
    }
    
    SDevice spDevice = SDevice::defaultDevice();

    //
    // 准备计算缓冲区
    //
    STensor spOut;
    SDimension spOutDim = solveCtx.spOutDimension.upHighDimension(nBatchs);
    if( STensor::createTensor<Q>(spOut, spOutDim, spOutDim.dataSize()) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }

    std::vector<SObject> arrExtras = {spBatchIn};
    struct PSolveVector {
        int size;
        void* data;
        SDeviceMemory buffer;
    };
    vector<PSolveVector> solveVars(solveCtx.arrVars.size());
    auto pOutVar = solveVars.data() + solveCtx.iOutVar;
    pOutVar->data = spOut.data(spDevice);
    auto pInVar = solveVars.data() + solveCtx.iInVar;
    pInVar->data = spBatchIn.data(spDevice);
    {
        PSolveVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size * nBatchs;
                if( pItVec != pOutVar) {
                    pItVec->buffer = SDeviceMemory::createDeviceMemory({spDevice, {pItVec->size * (int)sizeof(Q), nullptr}});
                    pItVec->data = pItVec->buffer.data(spDevice);
                    arrExtras.push_back(pItVec->buffer);
                }
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->data = pItVar->data.data(spDevice);
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveVector* pVec;
    PKernalVariable pKernelArgs[12];
    for(auto& instruct : solveCtx.arrInstructs ) {
        int nKernalArgs = instruct.nInVars*2 + 4;
        PKernalVariable* pMemory = pKernelArgs;
        if(instruct.spParameters) {
            *pMemory = PKernalVariable(instruct.spParameters.data(spDevice));
        }else{
            *pMemory = PKernalVariable((void*)nullptr);
        }
        pMemory++;

        *pMemory = PKernalVariable(nBatchs);
        pMemory++;

        for(int j=0; j<instruct.nInVars; j++) {
            pVec = &solveVars[instruct.pInVars[j]];
            pMemory[0] = PKernalVariable(pVec->size);
            pMemory[1] = PKernalVariable(pVec->data);
            pMemory+=2;
        }

        pVec = &solveVars[instruct.iOutVar];
        pMemory[0] = PKernalVariable(pVec->size);
        pMemory[1] = PKernalVariable(pVec->data);

        int nRanges = instruct.nRanges;
        int pRanges[3] = { instruct.pRanges[0], instruct.pRanges[1], instruct.pRanges[2] };
        pRanges[0] *= pRanges[0] < 0 ? -nBatchs : 1;
        pRanges[1] *= pRanges[1] < 0 ? -nBatchs : 1;
        pRanges[2] *= pRanges[2] < 0 ? -nBatchs : 1;
        if( spDevice->runKernel(
                {&instruct.evalKernelId, instruct.programName.c_str(), "floatEval"}, 
                nKernalArgs, pKernelArgs, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }
    }
    return CNnResizeTensor::createResizeTensor({spOut, (int)arrExtras.size(), arrExtras.data()}, spBatchOut);
}

int CDeviceNetwork::devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    int idType = spBatchOut.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(spBatchOutDeviation.type() != idType || spBatchOutDeviation.size() != spBatchOut.size() ) {
        return sCtx.error("偏差数据类型或尺寸错误");
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return deviaT<double>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return deviaT<float>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}


static int zeroBuffer(const SDevice& spDevice, SDeviceMemory spMemory) {
    static int kernelId = 0;
    PKernalVariable sMemory(spMemory.data(spDevice));
    int size = spMemory.size();
    if( spDevice->runKernel(
        {&kernelId, "sw.nn.Zero", "ucharEval"},
        1, &sMemory, 1, &size) != sCtx.success() ){
        return sCtx.error("设备运行内核Zero错误");
    }
    return sCtx.success();
}


template<typename Q>
int CDeviceNetwork::deviaT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;

    SDimension spBatchOutDimension = spBatchOut.dimension();
    int nBatchs = *spBatchOutDimension.data();
    int nOutputSize = spBatchOut.size();
    if(nOutputSize != nBatchs * solveCtx.spOutDimension.dataSize()) {
        return sCtx.error("输出张量尺寸和网络需要的尺寸不匹配");
    }

    SDevice spDevice = SDevice::defaultDevice();

    SNnResizeTensor spResizeOut = spBatchOut;
    if( !spResizeOut ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    if( spResizeOut->getResizeData(sResizeTensor) != sCtx.success() || sResizeTensor.nExtras < 1){
        return sCtx.error("数据错误");
    }
    int nExtras = sResizeTensor.nExtras;
    SObject* pExtras = sResizeTensor.pExtras;
    spBatchIn = *pExtras;
    pExtras++;
    nExtras--;

    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, spBatchIn.size()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    std::vector<SObject> arrWeightDeviaExtras;
    struct PSolveDeviaVector {
        int size;
        void* data;
        void* devia;
        SDeviceMemory dataBuffer;
        SDeviceMemory deviaBuffer;
    };
    vector<PSolveDeviaVector> solveVars(solveCtx.arrVars.size());
    {
        PSolveDeviaVector* pItVec = solveVars.data();
        PSolveDeviaVector* pOutVar = solveVars.data() + solveCtx.iOutVar;
        PSolveDeviaVector* pInVar = solveVars.data() + solveCtx.iInVar;
        pInVar->size = solveCtx.arrVars[solveCtx.iInVar].size*nBatchs;
        pInVar->data = spBatchIn.data(spDevice);
        pInVar->devia = spBatchInDeviation.data(spDevice);
        pOutVar->size = solveCtx.arrVars[solveCtx.iOutVar].size*nBatchs;
        pOutVar->data = spBatchOut.data(spDevice);
        pOutVar->devia = spBatchOutDeviation.data(spDevice);
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                if(pItVec != pOutVar) {
                    pItVec->size = pItVar->size*nBatchs;
                    if(nExtras < 1 ) {
                        return sCtx.error("计算数据错误，无法用于传递偏导数");
                    }
                    SDeviceMemory spOp = *pExtras;
                    pExtras++;
                    nExtras--;

                    if(!spOp || spOp.size() != pItVec->size*sizeof(Q) ) {
                        return sCtx.error("计算数据错误，无法用于传递偏导数");
                    }
                    pItVec->deviaBuffer = SDeviceMemory::createDeviceMemory({spDevice, {pItVec->size * (int)sizeof(Q), nullptr}});
                    zeroBuffer(spDevice,pItVec->deviaBuffer);
                    pItVec->data = spOp.data(spDevice);
                    pItVec->devia = pItVec->deviaBuffer.data(spDevice);
                }
                break;

            case ENnVariableType::EVState:
                {
                    pItVec->size = pItVar->size;
                    pItVec->deviaBuffer = SDeviceMemory::createDeviceMemory({spDevice, {pItVec->size * (int)sizeof(Q), nullptr}});
                    zeroBuffer(spDevice,pItVec->deviaBuffer);
                    pItVec->data = pItVar->data.data(spDevice);
                    pItVec->devia = pItVec->deviaBuffer.data(spDevice);
                }
                break;

            case ENnVariableType::EVWeight:
                {
                    pItVec->size = pItVar->size;
                    pItVec->deviaBuffer = SDeviceMemory::createDeviceMemory({spDevice,{pItVec->size * (int)sizeof(Q), nullptr}});
                    zeroBuffer(spDevice,pItVec->deviaBuffer);
                    pItVec->data = pItVar->data.data(spDevice);
                    pItVec->devia = pItVec->deviaBuffer.data(spDevice);
                    arrWeightDeviaExtras.push_back(pItVec->deviaBuffer);
                }
                break;
            }
        }
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
        if(instruct.spParameters) {
            *pMemory = PKernalVariable(instruct.spParameters.data(spDevice));
        }else{
            *pMemory = PKernalVariable((void*)nullptr);
        }
        pMemory++;

        *pMemory = PKernalVariable(nBatchs);
        pMemory++;

        for(int j=0; j<instruct.nInVars; j++) {
            pVec = &solveVars[instruct.pInVars[j]];
            pMemory[0] = PKernalVariable(pVec->size);
            pMemory[1] = PKernalVariable(pVec->data);
            pMemory[2] = PKernalVariable(pVec->devia);
            pMemory+=3;
        }
        pVec = &solveVars[instruct.iOutVar];
        pMemory[0] = PKernalVariable(pVec->size);
        pMemory[1] = PKernalVariable(pVec->devia);

        int nRanges = instruct.nRanges;
        int pRanges[3] = { instruct.pRanges[0], instruct.pRanges[1], instruct.pRanges[2] };
        pRanges[0] *= pRanges[0] < 0 ? -nBatchs : 1;
        pRanges[1] *= pRanges[1] < 0 ? -nBatchs : 1;
        pRanges[2] *= pRanges[2] < 0 ? -nBatchs : 1;
        if( spDevice->runKernel(
                {&instruct.deviaKernelId, instruct.programName.c_str(), "floatDevia"}, 
                nKernalArgs, pKernelArgs, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }
    }

    //
    // 拷贝权重核输入偏导数
    //
    int nWeights = solveCtx.nSumSize[EVWeight];
    Q* pWeightDeviaBuffer = (Q*)solveCtx.spOptimizer->getDeviationPtr(nWeights);
    {
        Q* pItWeightBuffer = pWeightDeviaBuffer;
        PSolveDeviaVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                pItVec->deviaBuffer->getCpuMemory({pItVec->size*(int)sizeof(Q),pItWeightBuffer});
                pItWeightBuffer += pItVec->size;
                break;
            }
        }
    }

    solveCtx.spOptimizer->updateDeviation(nBatchs);
    SObject pExtras2[] = { STensor::createVector<Q>(nWeights, pWeightDeviaBuffer) };
    return CNnResizeTensor::createResizeTensor({spBatchInDeviation, 1, pExtras2}, spBatchInDeviation);
}

int CDeviceNetwork::update(const STensor& spBatchInDeviation) {
    int idType = spBatchInDeviation.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return updateT<double>(spBatchInDeviation);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return updateT<float>(spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

template<typename Q>
int CDeviceNetwork::updateT(const STensor& spBatchInDeviation) {
    SNnResizeTensor spResizeDevia = spBatchInDeviation;
    if( !spResizeDevia ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    spResizeDevia->getResizeData(sResizeTensor);
    STensor spWeightDevia = sResizeTensor.pExtras[0];

    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;
    int nWeights = solveCtx.nSumSize[EVWeight];
    if(spWeightDevia.size() != nWeights) {
        return sCtx.error("数据错误，无法用于更新权重");
    }

    Q* pItWeightDevia = spWeightDevia.data<Q>();
    vector<PSolveGraphInfos::PSolveVar>::iterator itVar = solveCtx.arrVars.begin();
    while(itVar != solveCtx.arrVars.end() ) {
        switch(itVar->type) {
        case ENnVariableType::EVWeight:
            {
                Q* pItDevia = pItWeightDevia;
                Q* pItData = (Q*)itVar->data.data();
                Q* pDataEnd = pItData + itVar->size;
                while(pItData < pDataEnd) {
                    *pItData -= *pItDevia;
                    if(*pItData > 1) {
                        *pItData = 1;
                    }else if( *pItData < -1){
                        *pItData = -1;
                    }
                    pItData++, pItDevia++;
                }
                pItWeightDevia += itVar->size;
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