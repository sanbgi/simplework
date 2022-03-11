
#include "network.h"

#include <map>
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
            ENnVariableType type;//ENnVariableType
            int size;
            void* data; //存储状态及权重值的指针
        };

        // 计算指令
        struct PSolveInstruct {
            PSolveFunc solver;
            PNnAtomOperatorArgs args;
            int evalKernelId;
            int deviaKernelId;
            string programName;
        };
        int zeroKernelId;

        //解算参数列表
        vector<PSolveVar> arrVars;
        //解算步骤列表
        vector<PSolveInstruct> arrInstructs;

        int iInVar;
        int iOutVar;
        int nSumSize[EVMax];
        int nMaxParameterSize;
    
        int nInputTensorSize;
        int nOutputTensorSize;
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
        solveParameter.args = spOp;
        (*itOp)->prepareSolver({idType,PSolveCtx::CPU}, solveParameter.solver);
        if(solveParameter.solver.nParamterSize > nMaxParameterSize) {
            nMaxParameterSize = solveParameter.solver.nParamterSize;
        }
        solveParameter.evalKernelId = solveParameter.deviaKernelId = 0;
        solveParameter.programName = string("sw.nn.")+(*itOp)->getName();
        arrInstructs.push_back(solveParameter);
        itParameter++, itOp++;
    }
    solveCtx.zeroKernelId = 0;

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
    solveCtx.nMaxParameterSize = nMaxParameterSize;
    solveCtx.idType = idType;
    solveCtx.nInputTensorSize = spInDimension.dataSize();
    solveCtx.nOutputTensorSize = spOutDimension.dataSize();
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
    if(nInputSize != nBatchs * solveCtx.nInputTensorSize) {
        return sCtx.error("输入张量尺寸和网络需要的尺寸不匹配");
    }
    
    //
    // 准备计算缓冲区
    //
    STensor spOpSolveBuffer;
    if( STensor::createVector<Q>(spOpSolveBuffer, solveCtx.nSumSize[EVOperator] * nBatchs) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }
    STensor spOut;
    SDimension spOutDim = solveCtx.spOutDimension.upHighDimension(nBatchs);
    if( STensor::createTensor<Q>(spOut, spOutDim, spOutDim.dataSize()) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }

    SDevice spDevice = SDevice::defaultDevice();
    struct PSolveVector {
        int size;
        void* data;
        SDeviceMemory buffer;
    };
    vector<PSolveVector> solveVars(solveCtx.arrVars.size());
    {
        PSolveVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size * nBatchs;
                pItVec->buffer = SDeviceMemory::createMemory({pItVec->size * (int)sizeof(Q), nullptr});
                pItVec->data = pItVec->buffer.data(spDevice);
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->buffer = SDeviceMemory::createMemory({pItVec->size * (int)sizeof(Q), pItVar->data});
                pItVec->data = pItVec->buffer.data(spDevice);
                break;

            case ENnVariableType::EVInput:
                {
                    pItVec->size = pItVar->size * nBatchs;
                    Q* pIn = spBatchIn.data<Q>();
                    pItVec->buffer = SDeviceMemory::createMemory({pItVec->size * (int)sizeof(Q), pItVar->data});
                    pItVec->data = pItVec->buffer.data(spDevice);
                }
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveVector* pVec;
    PMemory pMemorys[12];
    for(auto& instruct : solveCtx.arrInstructs ) {
        int nMemory = instruct.args.nInVars*2 + 4;
        PMemory* pMemory = pMemorys;
        SDeviceMemory spParameter;
        void* pParameter=nullptr;
        if(instruct.solver.nParamterSize>0) {
            spParameter = SDeviceMemory::createMemory({instruct.solver.nParamterSize, instruct.solver.pParameterData});
            pParameter = spParameter.data(spDevice);
            pMemory->size = sizeof(void*);
            pMemory->data = &pParameter;
        }else{
            pMemory->size = sizeof(void*);
            pMemory->data = &pParameter;
        }
        pMemory++;

        pMemory->size = sizeof(int);
        pMemory->data = &nBatchs;
        pMemory++;
        for(int j=0; j<instruct.args.nInVars; j++) {
            pVec = &solveVars[instruct.args.pInVars[j]];
            pMemory[0].size = sizeof(pVec->size);
            pMemory[0].data = &pVec->size;
            pMemory[1].size = sizeof(pVec->data);
            pMemory[1].data = &pVec->data;
            pMemory+=2;
        }
        pVec = &solveVars[instruct.args.iOutVar];
        pMemory[0].size = sizeof(pVec->size);
        pMemory[0].data = &pVec->size;
        pMemory[1].size = sizeof(pVec->data);
        pMemory[1].data = &pVec->data;

        int nRanges = 1;
        int pRanges[3];
        switch(instruct.solver.eClRange) {
            case PSolveFunc::PBatchAndOut:
                nRanges = 2;
                pRanges[0] = nBatchs;
                pRanges[1] = pVec->size/nBatchs;
                break;

            case PSolveFunc::PBatch:
                pRanges[0] = nBatchs;
                break;

            case PSolveFunc::PCustomer:
                pRanges[0] = instruct.solver.nCustomerRange;
                break;

            case PSolveFunc::POut:
                pRanges[0] = pVec->size;
                break;

            default:
                pRanges[0] = 1;
                break;
        }
        if( spDevice->runKernel(
                {&instruct.evalKernelId, instruct.programName.c_str(), "floatEval"}, 
                nMemory, pMemorys, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }
    }

    //  
    // 准备计算指针
    //  1，输入变量的数据指针指向输入数据
    //  2，运算变量指针指向缓冲区
    //
    int iOffset = 0;
    {
        Q* pOpSolvedBuffer = spOpSolveBuffer.data<Q>();
        Q* pItBuffer = pOpSolvedBuffer;
        PSolveVector* pItVec = solveVars.data();
        PSolveVector* pOutVec = pItVec + solveCtx.iOutVar;
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                if( pItVec->buffer->getMemory({pItVec->size* (int)sizeof(Q), pItBuffer}) != sCtx.success()) {
                    return sCtx.error("内存拷贝错误");
                }
                if(pItVec == pOutVec) {
                    iOffset = (int)(pItBuffer - pOpSolvedBuffer);
                }
                pItBuffer+=pItVec->size;
                break;
            }
        }
    }
    memcpy(spOut.data(), spOpSolveBuffer.data<Q>()+iOffset, spOut.size()*sizeof(Q) );
    return CNnResizeTensor::createResizeTensor({spOut, spOpSolveBuffer, spBatchIn}, spBatchOut);
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


static int zeroBuffer(const SDevice& spDevice, SDeviceMemory spMemory, int size) {
    static int kernelId = 0;
    PMemory sMemory;
    void* pData = spMemory.data(spDevice);
    sMemory.size = sizeof(void*);
    sMemory.data = &pData;
    size = spMemory.size();
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
    int nOutputSize = spBatchOutDimension.dataSize();
    if(nOutputSize != nBatchs * solveCtx.nOutputTensorSize) {
        return sCtx.error("输出张量尺寸和网络需要的尺寸不匹配");
    }

    SNnResizeTensor spResizeOut = spBatchOut;
    if( !spResizeOut ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    SDevice spDevice = SDevice::defaultDevice();
    PNnResizeTensor sResizeTensor;
    spResizeOut->getResizeData(sResizeTensor);
    STensor spOpTensor = sResizeTensor.spExtra1;
    spBatchIn = sResizeTensor.spExtra2;

    struct PSolveDeviaVector {
        int size;
        void* data;
        void* devia;
        SDeviceMemory dataBuffer;
        SDeviceMemory deviaBuffer;
    };
    vector<PSolveDeviaVector> solveVars(solveCtx.arrVars.size());
    {
        Q* pItOpVar = spOpTensor.data<Q>();
        PSolveDeviaVector* pItVec = solveVars.data();
        PSolveDeviaVector* pOutVar = solveVars.data() + solveCtx.iOutVar;
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            pItVec->size = pItVar->size;
            switch(pItVar->type) {
                case ENnVariableType::EVOperator:
                case ENnVariableType::EVInput:
                    pItVec->size *= nBatchs;
                    break;
            }
            int nSize = pItVec->size * sizeof(Q);
            pItVec->dataBuffer = SDeviceMemory::createMemory({nSize, nullptr});
            pItVec->deviaBuffer = SDeviceMemory::createMemory({nSize, nullptr});
            pItVec->data = pItVec->dataBuffer.data(spDevice);
            pItVec->devia = pItVec->deviaBuffer.data(spDevice);
            if( zeroBuffer(spDevice, pItVec->deviaBuffer, nSize) != sCtx.success() ){
                return sCtx.error("OpenCL初始化数据错误");
            }
        
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                if(pItVec == pOutVar) {
                    Q* pOutDevia = spBatchOutDeviation.data<Q>();
                    if( pItVec->deviaBuffer->setMemory({pItVec->size*(int)sizeof(Q), pOutDevia}) != sCtx.success() ) {
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }else{
                    if( pItVec->dataBuffer->setMemory({pItVec->size*(int)sizeof(Q), pItOpVar}) != sCtx.success() ) {
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }
                pItOpVar += pItVec->size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                if( pItVec->dataBuffer->setMemory({pItVec->size*(int)sizeof(Q), pItVar->data}) != sCtx.success() ) {
                    return sCtx.error("OpenCL拷贝数据错误");
                }
                break;

            case ENnVariableType::EVInput:
                {
                    Q* pIn = spBatchIn.data<Q>();
                    if( pItVec->dataBuffer->setMemory({pItVec->size*(int)sizeof(Q), pIn}) != sCtx.success() ) {
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveDeviaVector* pVec;
    PMemory pMemorys[17];
    for(auto itSolver=solveCtx.arrInstructs.rbegin(); itSolver != solveCtx.arrInstructs.rend(); itSolver++) {

        //准备输入输出计算参数
        PSolveGraphInfos::PSolveInstruct& instruct = *itSolver;

        int nMemory = instruct.args.nInVars*3 + 4;
        PMemory* pMemory = pMemorys;
        SDeviceMemory spParameter;
        void* pParameter=nullptr;
        if(instruct.solver.nParamterSize>0) {
            spParameter = SDeviceMemory::createMemory({instruct.solver.nParamterSize, instruct.solver.pParameterData});
            pParameter = spParameter.data(spDevice);
            pMemory->size = sizeof(void*);
            pMemory->data = &pParameter;
        }else{
            pMemory->size = sizeof(void*);
            pMemory->data = &pParameter;
        }
        pMemory++;

        pMemory->size = sizeof(int);
        pMemory->data = &nBatchs;
        pMemory++;

        for(int j=0; j<instruct.args.nInVars; j++) {
            pVec = &solveVars[instruct.args.pInVars[j]];
            pMemory[0].size = sizeof(pVec->size);
            pMemory[0].data = &pVec->size;
            pMemory[1].size = sizeof(pVec->data);
            pMemory[1].data = &pVec->data;
            pMemory[2].size = sizeof(pVec->devia);
            pMemory[2].data = &pVec->devia;
            pMemory+=3;
        }
        pVec = &solveVars[instruct.args.iOutVar];
        pMemory[0].size = sizeof(pVec->size);
        pMemory[0].data = &pVec->size;
        pMemory[1].size = sizeof(pVec->devia);
        pMemory[1].data = &pVec->devia;

        int nRanges = 1;
        int pRanges[3];
        switch(instruct.solver.eClRange) {
            case PSolveFunc::PBatchAndOut:
                nRanges = 2;
                pRanges[0] = nBatchs;
                pRanges[1] = pVec->size/nBatchs;
                break;

            case PSolveFunc::PBatch:
                pRanges[0] = nBatchs;
                break;

            case PSolveFunc::PCustomer:
                pRanges[0] = instruct.solver.nCustomerRange;
                break;

            case PSolveFunc::POut:
                pRanges[0] = pVec->size;
                break;

            default:
                pRanges[0] = 1;
                break;
        }
        if( spDevice->runKernel(
                {&instruct.deviaKernelId, instruct.programName.c_str(), "floatDevia"}, 
                nMemory, pMemorys, 
                nRanges, pRanges) != sCtx.success() ) {
            return sCtx.error("设备计算错误");
        }
    }

    //
    // 拷贝权重核输入偏导数
    //
    int nWeights = solveCtx.nSumSize[EVWeight];
    Q* pWeightDeviaBuffer = (Q*)solveCtx.spOptimizer->getDeviationPtr(nWeights);
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, spBatchIn.size()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    {
        Q* pBatchInDevia = spBatchInDeviation.data<Q>();
        Q* pItWeightBuffer = pWeightDeviaBuffer;
        PSolveDeviaVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                pItVec->deviaBuffer->getMemory({pItVec->size*(int)sizeof(Q),pItWeightBuffer});
                pItWeightBuffer += pItVec->size;
                break;

            case ENnVariableType::EVInput:
                pItVec->deviaBuffer->getMemory({pItVec->size*(int)sizeof(Q),pBatchInDevia});
                break;
            }
        }
    }

    solveCtx.spOptimizer->updateDeviation(nBatchs);
    STensor spWeightDevia = STensor::createVector<Q>(nWeights, pWeightDeviaBuffer);
    return CNnResizeTensor::createResizeTensor({spBatchInDeviation, spWeightDevia}, spBatchInDeviation);
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
    STensor spWeightDevia = sResizeTensor.spExtra1;

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
                Q* pItData = (Q*)(itVar->data);
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