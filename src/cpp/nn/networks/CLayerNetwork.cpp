
#include "network.h"

#include <map>
#include <string>
#include <iostream>
using namespace sw;
using namespace std;

static SCtx sCtx("CLayerNetwork");
class CLayerNetwork : public CObject, public INnNetwork, public IArchivable{
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
    static const char* __getClassKey() { return "sw.nn.LayerNetwork"; }

public://INnLayerNetwork
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
        };

        //解算参数列表
        vector<PSolveVar> arrVars;
        //解算步骤列表
        vector<PSolveInstruct> arrInstructs;

        int iInVar;
        int iOutVar;
        int nSumSize[EVMax];
    
        int nInputTensorSize;
        int nOutputTensorSize;
        SOptimizer spOptimizer;
    };
    CTaker<PSolveGraphInfos*> m_spSolveGraphInfos;

public:
    int initNetwork(PDATATYPE idType);
};

int CLayerNetwork::__initialize(const PData* pData){
    const PNnNetwork* pNet = CData<PNnNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    if( CNnVariableSolver::solveNetwork(pNet, &m_sSolveGraph) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }
    return sCtx.success();
}

int CLayerNetwork::initNetwork(PDATATYPE idType) {
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
    vector<PSolveGraphInfos::PSolveInstruct>& arrInstructs = solveCtx.arrInstructs;
    auto itParameter = m_sSolveGraph.arrOperatorArgs.begin();
    auto itOp = m_sSolveGraph.arrOperators.begin();
    while(itParameter != m_sSolveGraph.arrOperatorArgs.end()) {
        PNnAtomOperatorArgs spOp = *itParameter;
        PSolveGraphInfos::PSolveInstruct solveParameter;
        solveParameter.args = spOp;
        (*itOp)->prepareSolver({idType,PSolveCtx::CPU}, solveParameter.solver);
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
    solveCtx.nInputTensorSize = spInDimension.dataSize();
    solveCtx.nOutputTensorSize = spOutDimension.dataSize();
    solveCtx.spInDimension = spInDimension;
    solveCtx.spOutDimension = spOutDimension;
    m_spSolveGraphInfos.take(taker);
    return sCtx.success();
}

int CLayerNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
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
int CLayerNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {
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

    //  
    // 准备计算指针
    //  1，输入变量的数据指针指向输入数据
    //  2，运算变量指针指向缓冲区
    //
    Q* pOpSolvedBuffer = spOpSolveBuffer.data<Q>();
    vector<PVector> solveVars(solveCtx.arrVars.size());
    {
        Q* pItOpBuffer = pOpSolvedBuffer;
        PVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size * nBatchs;
                pItVec->data = pItOpBuffer;
                pItOpBuffer += pItVec->size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->data = pItVar->data;
                break;

            case ENnVariableType::EVInput:
                pItVec->size = pItVar->size * nBatchs;
                pItVec->data = spBatchIn.data<Q>();
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PVector evalIn[4], evalOut;
    for(auto& instruct : solveCtx.arrInstructs ) {
        //准备输入输出计算参数
        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pEvalFun)(instruct.solver.pParameterData, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    memcpy(spOut.data(), solveVars[solveCtx.iOutVar].data, solveVars[solveCtx.iOutVar].size*sizeof(Q) );
    return CNnResizeTensor::createResizeTensor({spOut, spOpSolveBuffer, spBatchIn}, spBatchOut);
}

int CLayerNetwork::devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
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

template<typename Q>
int CLayerNetwork::deviaT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {

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

    PNnResizeTensor sResizeTensor;
    spResizeOut->getResizeData(sResizeTensor);
    STensor spOpTensor = sResizeTensor.spExtra1;
    spBatchIn = sResizeTensor.spExtra2;

    //
    // 准备计算缓冲区
    //
    int nDeviaBuffer = solveCtx.nSumSize[EVOperator] * nBatchs + solveCtx.nSumSize[EVState];
    CTaker<Q*> spDeviaBuffer(new Q[nDeviaBuffer], [](Q* ptr){
        delete[] ptr;
    });
    Q* pOpDeviaBuffer = spDeviaBuffer;
    Q* pStateDeviaBuffer = pOpDeviaBuffer + solveCtx.nSumSize[EVOperator] * nBatchs;
    memset(spDeviaBuffer, 0, sizeof(Q)*nDeviaBuffer);

    //
    // 准备权重缓冲区
    //
    int nWeights = solveCtx.nSumSize[EVWeight];
    Q* pWeightDeviaBuffer = (Q*)solveCtx.spOptimizer->getDeviationPtr(nWeights);
    memset(pWeightDeviaBuffer, 0, sizeof(Q)*nWeights);

    int nBatchInSize = spBatchIn.size();
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, nBatchInSize) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    memset(spBatchInDeviation.data<Q>(), 0, sizeof(Q)*nBatchInSize);

    vector<PDeviaVector> solveVars(solveCtx.arrVars.size());
    {
        Q* pItOpDevia = pOpDeviaBuffer;
        Q* pItStateDevia = pStateDeviaBuffer;
        Q* pItWeightDevia = pWeightDeviaBuffer;
        Q* pItOpVar = spOpTensor.data<Q>();
        PDeviaVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->data = pItVar->data;
                pItVec->devia = pItWeightDevia;
                pItWeightDevia += pItVec->size;
                break;

            case ENnVariableType::EVState:
                pItVec->size = pItVar->size;
                pItVec->data = pItVar->data;
                pItVec->devia = pItStateDevia;
                pItStateDevia += pItVec->size;
                break;

            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size*nBatchs;
                pItVec->data = pItOpVar;
                pItVec->devia = pItOpDevia;
                pItOpDevia += pItVec->size;
                pItOpVar += pItVec->size;
                break;

            case ENnVariableType::EVInput:
                pItVec->size = pItVar->size*nBatchs;
                pItVec->data = spBatchIn.data<Q>();
                pItVec->devia = spBatchInDeviation.data<Q>();
                break;
            }
        }
    }

    //
    // 拷贝输出偏差
    //
    PDeviaVector* pOutVar = solveVars.data() + solveCtx.iOutVar;
    pOutVar->devia = spBatchOutDeviation.data<Q>();

    //
    // 遍历计算序列并执行
    //
    PDeviaVector evalIn[4], evalOut;
    for(auto itSolver=solveCtx.arrInstructs.rbegin(); itSolver != solveCtx.arrInstructs.rend(); itSolver++) {

        //准备输入输出计算参数
        PSolveGraphInfos::PSolveInstruct& instruct = *itSolver;

        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pDeviaFun)(instruct.solver.pParameterData, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    solveCtx.spOptimizer->updateDeviation(nBatchs);
    STensor spWeightDevia = STensor::createVector<Q>(nWeights, pWeightDeviaBuffer);
    return CNnResizeTensor::createResizeTensor({spBatchInDeviation, spWeightDevia}, spBatchInDeviation);
}

int CLayerNetwork::update(const STensor& spBatchInDeviation) {
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
int CLayerNetwork::updateT(const STensor& spBatchInDeviation) {
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

int CLayerNetwork::toArchive(const SArchive& ar) {
    m_sSolveGraph.toArchive(ar);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLayerNetwork, CLayerNetwork::__getClassKey())