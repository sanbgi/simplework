
#include "network.h"

#include <map>
#include <string>
#include <iostream>
using namespace sw;
using namespace std;

static SCtx sCtx("CLayerNetwork");

struct PSolveInfos {
    // 计算变量
    struct PSolveVar {
        ENnVariableType type;//ENnVariableType
        int size;
        void* data; //存储状态及权重值的指针
    };

    // 计算指令
    struct PSolveInstruct {
        PSolveParameter solver;
        PNnAtomOperatorArgs args;
    };

    //解算参数列表
    vector<PSolveVar> arrVars;
    //解算步骤列表
    vector<PSolveInstruct> arrSolvers;

    int nStateSize;
    int nWeightSize;
    int nOpSize;
    int iOutVar;

    SDimension spOutDimension;
    int nInputTensorSize;
    int nOutputTensorSize;
    unsigned int idType;
    SOptimizer spOptimizer;
};

class CLayerNetwork : public CObject, public INnNetwork, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "LayerNetwork"; } 
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
    PNnSolver m_sSolver;
    SDimension m_spInDimension;
    string m_strOptimizer;
    CTaker<PSolveInfos*> m_spSolveInfos;

public:
    int initNetwork(unsigned int idType);
};

int CLayerNetwork::__initialize(const PData* pData){
    const PNnNetwork* pNet = CData<PNnNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    if( CNnVariableSolver::solveNetwork(pNet, &m_sSolver) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }
    m_spInDimension = pNet->spInDimension;
    return sCtx.success();
}

int CLayerNetwork::initNetwork(unsigned int idType) {
    if(m_spSolveInfos) {
        if(idType == m_spSolveInfos->idType) {
            return sCtx.success();
        }else{
            return sCtx.error("数据类型与初始化时不一致，暂不支持");
        }
    }

    CTaker<PSolveInfos*> taker(new PSolveInfos, [](PSolveInfos* pCtx){
        delete pCtx;
    });
    PSolveInfos& solveCtx = *taker;

    //
    // 更新计算变量数组
    //
    int nStateSize = 0;
    int nWeightSize = 0;
    int nOpSize = 0;
    vector<PSolveInfos::PSolveVar>& arrVars = solveCtx.arrVars;
    vector<SNnVariable>::iterator itVar = m_sSolver.arrVars.begin();
    while(itVar != m_sSolver.arrVars.end()) {
        SNnInternalVariable spToSolveVar = *itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveInfos::PSolveVar solveVar;
        solveVar.size = spToSolveVar->getSize();
        solveVar.type = spToSolveVar->getVariableType();
        solveVar.data = spToSolveVar->getData(idType);
        switch(solveVar.type) {
        case ENnVariableType::EVState:
            nStateSize += solveVar.size;
            break;

        case ENnVariableType::EVWeight:
            nWeightSize += solveVar.size;
            break;

        case ENnVariableType::EVOperator:
            nOpSize += solveVar.size;
            break;
        }
        arrVars.push_back(solveVar);
        itVar++;
    }

    vector<PSolveInfos::PSolveInstruct>& arrSolvers = solveCtx.arrSolvers;
    vector<PNnAtomOperatorArgs>::iterator itParameter = m_sSolver.arrOperatorArgs.begin();
    vector<SNnAtomOperator>::iterator itOp = m_sSolver.arrOperators.begin();
    while(itParameter != m_sSolver.arrOperatorArgs.end()) {
        PNnAtomOperatorArgs spOp = *itParameter;
        PSolveInfos::PSolveInstruct solveParameter;
        solveParameter.args = spOp;
        (*itOp)->prepareSolver(idType, solveParameter.solver);
        arrSolvers.push_back(solveParameter);
        itParameter++, itOp++;
    }
    SDimension spOutDimension = m_sSolver.arrVars[m_sSolver.iOutVar].dimension();

    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, solveCtx.spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }
    solveCtx.nOpSize = nOpSize;
    solveCtx.nStateSize = nStateSize;
    solveCtx.nWeightSize = nWeightSize;
    solveCtx.iOutVar = m_sSolver.iOutVar;
    solveCtx.idType = idType;
    solveCtx.nInputTensorSize = m_spInDimension.dataSize();
    solveCtx.nOutputTensorSize = spOutDimension.dataSize();
    solveCtx.spOutDimension = spOutDimension;

    m_spSolveInfos.take(taker);
    return sCtx.success();
}

int CLayerNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    unsigned int idType = spBatchIn.type();
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
    PSolveInfos& solveCtx = *m_spSolveInfos;

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
    if( STensor::createVector<Q>(spOpSolveBuffer, solveCtx.nOpSize * nBatchs) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }

    //  
    // 准备计算指针
    //  1，输入变量的数据指针指向输入数据
    //  2，运算变量指针指向缓冲区
    //
    Q* pOpSolvedBuffer = spOpSolveBuffer.data<Q>();
    PVector solveVars[solveCtx.arrVars.size()];
    {
        Q* pItOpBuffer = pOpSolvedBuffer;
        vector<PSolveInfos::PSolveVar>::iterator pVar = solveCtx.arrVars.begin();
        PVector* pVec = solveVars;
        while(pVar != solveCtx.arrVars.end() ) {
            switch(pVar->type) {
            case ENnVariableType::EVOperator:
                pVec->size = pVar->size * nBatchs;
                pVec->data = pItOpBuffer;
                pItOpBuffer += pVec->size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pVec->size = pVar->size;
                pVec->data = pVar->data;
                break;

            case ENnVariableType::EVInput:
                pVec->size = pVar->size * nBatchs;
                pVec->data = spBatchIn.data<Q>();
                break;
            }
            pVar++, pVec++;
        }
    }

    //
    // 遍历计算序列并执行
    //
    PVector evalIn[4], evalOut;
    vector<PSolveInfos::PSolveInstruct>::iterator itSolver = solveCtx.arrSolvers.begin();
    while(itSolver != solveCtx.arrSolvers.end() ) {
        //准备输入输出计算参数
        PSolveInfos::PSolveInstruct instruct = *itSolver;
        itSolver++;

        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pEvalFun)(instruct.solver.pParameter, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    SDimension spOutDim = solveCtx.spOutDimension.upHighDimension(nBatchs);
    PVector* pOutVar = solveVars + solveCtx.iOutVar;
    int iOffset = ((Q*)pOutVar->data)-pOpSolvedBuffer;
    return CNnResizeTensor::createResizeTensor({spOpSolveBuffer, spOutDim, iOffset, spBatchIn}, spBatchOut);
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

    PSolveInfos& solveCtx = *m_spSolveInfos;

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
    spBatchIn = sResizeTensor.spExtra;


    //
    // 准备计算缓冲区
    //
    int nDeviaBuffer = solveCtx.nOpSize * nBatchs + solveCtx.nStateSize;
    CTaker<Q*> spDeviaBuffer(new Q[nDeviaBuffer], [](Q* ptr){
        delete[] ptr;
    });
    Q* pOpDeviaBuffer = spDeviaBuffer;
    Q* pStateDeviaBuffer = pOpDeviaBuffer + solveCtx.nOpSize * nBatchs;
    memset(spDeviaBuffer, 0, sizeof(Q)*nDeviaBuffer);

    //
    // 准备权重缓冲区
    //
    int nWeights = solveCtx.nWeightSize;
    Q* pWeightDeviaBuffer = (Q*)solveCtx.spOptimizer->getDeviationPtr(nWeights);
    memset(pWeightDeviaBuffer, 0, sizeof(Q)*nWeights);

    int nBatchInSize = spBatchIn.size();
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, nBatchInSize) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    memset(spBatchInDeviation.data<Q>(), 0, sizeof(Q)*nBatchInSize);

    PDeviaVector solveVars[solveCtx.arrVars.size()];
    {
        Q* pItOpDevia = pOpDeviaBuffer;
        Q* pItStateDevia = pStateDeviaBuffer;
        Q* pItWeightDevia = pWeightDeviaBuffer;
        Q* pItOpVar = sResizeTensor.spSrc.data<Q>();
        vector<PSolveInfos::PSolveVar>::iterator pItVar = solveCtx.arrVars.begin();
        PDeviaVector* pVec = solveVars;
        while(pItVar != solveCtx.arrVars.end() ) {
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                pVec->size = pItVar->size;
                pVec->data = pItVar->data;
                pVec->devia = pItWeightDevia;
                pItWeightDevia += pVec->size;
                break;

            case ENnVariableType::EVState:
                pVec->size = pItVar->size;
                pVec->data = pItVar->data;
                pVec->devia = pItStateDevia;
                pItStateDevia += pVec->size;
                break;

            case ENnVariableType::EVOperator:
                pVec->size = pItVar->size*nBatchs;
                pVec->data = pItOpVar;
                pVec->devia = pItOpDevia;
                pItOpDevia += pVec->size;
                pItOpVar += pVec->size;
                break;

            case ENnVariableType::EVInput:
                pVec->size = pItVar->size*nBatchs;
                pVec->data = spBatchIn.data<Q>();
                pVec->devia = spBatchInDeviation.data<Q>();
                break;
            }

            pItVar++, pVec++;
        }
    }

    //
    // 拷贝输出偏差
    //
    PDeviaVector* pOutVar = solveVars + solveCtx.iOutVar;
    pOutVar->devia = spBatchOutDeviation.data<Q>();

    //
    // 遍历计算序列并执行
    //
    PDeviaVector evalIn[4], evalOut;
    vector<PSolveInfos::PSolveInstruct>::reverse_iterator itSolver = solveCtx.arrSolvers.rbegin();
    while(itSolver != solveCtx.arrSolvers.rend() ) {

        //准备输入输出计算参数
        PSolveInfos::PSolveInstruct instruct = *itSolver;
        itSolver++;

        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pDeviaFun)(instruct.solver.pParameter, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    solveCtx.spOptimizer->updateDeviation(nBatchs);
    STensor spWeightDevia = STensor::createVector<Q>(nWeights, pWeightDeviaBuffer);
    return CNnResizeTensor::createResizeTensor({spBatchInDeviation, spBatchInDeviation.dimension(), 0, spWeightDevia}, spBatchInDeviation);
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
    STensor spWeightDevia = sResizeTensor.spExtra;

    PSolveInfos& solveCtx = *m_spSolveInfos;
    int nWeights = solveCtx.nWeightSize;
    if(spWeightDevia.size() != nWeights) {
        return sCtx.error("数据错误，无法用于更新权重");
    }

    Q* pItWeightDevia = spWeightDevia.data<Q>();
    vector<PSolveInfos::PSolveVar>::iterator itVar = solveCtx.arrVars.begin();
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
    m_sSolver.toArchive(ar);
    ar.arObject("inputDimension", m_spInDimension);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLayerNetwork, CLayerNetwork::__getClassKey())