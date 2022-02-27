
#include "network.h"

#include <map>
#include <string>
#include <iostream>
using namespace sw;
using namespace std;

static SCtx sCtx("CLayerNetwork");

struct PSolveContext {
    //
    // 计算变量
    //
    struct PSolveVar {
        ENnVariableType type;//ENnVariableType
        int size;
        void* data; //存储状态及权重值的指针
    };

    //
    // 计算指令
    //
    struct PSolveInstruct {
        //
        // 求解器信息
        //
        PSolveParameter solver;

        //
        // 计算参数
        //
        PNnAtomOperatorArgs args;
    };

    //解算参数列表
    vector<PSolveVar> arrVars;
    //解算步骤列表
    vector<PSolveInstruct> arrSolvers;

    //解算参数数据
    int nVars;
    PSolveVar* pVars;

    int nSolvers;
    PSolveInstruct* pSolvers;

    int nStateSize;
    int nWeightSize;
    int nOpSize;
    int iOutVar;;
};

class CLayerNetwork : public CObject, public INnNetwork, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    //
    // 计算函数
    //
    typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

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
    CTaker<PNnSolver*> m_spSolver;
    SDimension m_spInDimension;
    string m_strOptimizer;

    bool m_bInitialized;
    SDimension m_spOutDimension;
    int m_nInputTensorSize;
    int m_nOutputTensorSize;
    CTaker<PSolveContext*> m_spContext;
    unsigned int m_idType;
    SOptimizer m_spOptimizer;

public:
    int initNetwork(unsigned int idType);
    CLayerNetwork();
};

CLayerNetwork::CLayerNetwork() {
    m_bInitialized = false;
    m_spSolver.take(new PNnSolver, [](PNnSolver* pSolver) {
        delete pSolver;
    });
}

int CLayerNetwork::__initialize(const PData* pData){
    const PNnNetwork* pNet = CData<PNnNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    if( CNnVariableSolver::solveNetwork(pNet, m_spSolver) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }
    m_spInDimension = pNet->spInDimension;
    return sCtx.success();
}

int CLayerNetwork::initNetwork(unsigned int idType) {
    if(m_bInitialized) {
        return sCtx.success();
    }

    SDimension spDimension = m_spInDimension;
    m_spContext.take(new PSolveContext, [](PSolveContext* pCtx){
        delete pCtx;
    });
    PSolveContext& solveCtx = *m_spContext;

    //
    // 更新计算变量数组
    //
    int nStateSize = 0;
    int nWeightSize = 0;
    int nOpSize = 0;
    vector<PSolveContext::PSolveVar>& arrVars = solveCtx.arrVars;
    vector<SNnVariable>::iterator itVar = m_spSolver->arrVars.begin();
    while(itVar != m_spSolver->arrVars.end()) {
        SNnInternalVariable spToSolveVar = *itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveContext::PSolveVar solveVar;
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

    vector<PSolveContext::PSolveInstruct>& arrSolvers = solveCtx.arrSolvers;
    vector<PNnAtomOperatorArgs>::iterator itParameter = m_spSolver->arrOperatorArgs.begin();
    vector<SNnAtomOperator>::iterator itOp = m_spSolver->arrOperators.begin();
    while(itParameter != m_spSolver->arrOperatorArgs.end()) {
        PNnAtomOperatorArgs spOp = *itParameter;
        PSolveContext::PSolveInstruct solveParameter;
        solveParameter.args = spOp;
        (*itOp)->prepareSolver(idType, solveParameter.solver);
        arrSolvers.push_back(solveParameter);
        itParameter++, itOp++;
    }
    spDimension = m_spSolver->arrVars[m_spSolver->iOutVar].dimension();

    solveCtx.nOpSize = nOpSize;
    solveCtx.nStateSize = nStateSize;
    solveCtx.nWeightSize = nWeightSize;
    solveCtx.iOutVar = m_spSolver->iOutVar;
    solveCtx.nVars = solveCtx.arrVars.size();
    solveCtx.pVars = solveCtx.arrVars.data();
    solveCtx.nSolvers = solveCtx.arrSolvers.size();
    solveCtx.pSolvers = solveCtx.arrSolvers.data();
    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    m_idType = idType;
    m_nInputTensorSize = m_spInDimension.dataSize();
    m_nOutputTensorSize = spDimension.dataSize();
    m_spOutDimension = spDimension;
    m_bInitialized = true;
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

    SDimension spInDimension = spBatchIn.dimension();
    int nBatchs = *spInDimension.data();
    int nInputSize = spInDimension.dataSize();
    if(nInputSize != nBatchs * m_nInputTensorSize) {
        return sCtx.error("输入张量尺寸和网络需要的尺寸不匹配");
    }

    PSolveContext& solveCtx = *m_spContext;

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
    PVector solveVars[solveCtx.nVars];
    {
        PSolveContext::PSolveVar *pVar = solveCtx.pVars;
        Q* pItOpBuffer = pOpSolvedBuffer;
        for( int i=0; i<solveCtx.nVars; i++, pVar++ ) {
            switch(pVar->type) {
            case ENnVariableType::EVOperator:
                solveVars[i].size = pVar->size * nBatchs;
                solveVars[i].data = pItOpBuffer;
                pItOpBuffer += solveVars[i].size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                solveVars[i].size = pVar->size;
                solveVars[i].data = pVar->data;
                break;

            case ENnVariableType::EVInput:
                solveVars[i].size = pVar->size * nBatchs;
                solveVars[i].data = spBatchIn.data<Q>();
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveContext::PSolveInstruct *pSolver, *pItSolver, *pItSolverEnd;
    PVector evalIn[4], evalOut;
    pItSolver = solveCtx.pSolvers;
    pItSolverEnd = pItSolver + solveCtx.nSolvers;
    while(pItSolver < pItSolverEnd) {
        pSolver = pItSolver++;

        //准备输入输出计算参数
        PSolveContext::PSolveInstruct instruct = *pSolver;
        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pEvalFun)(instruct.solver.pParameter, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    SDimension spOutDim = m_spOutDimension.upHighDimension(nBatchs);
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
    SDimension spBatchOutDimension = spBatchOut.dimension();
    int nBatchs = *spBatchOutDimension.data();
    int nOutputSize = spBatchOutDimension.dataSize();
    if(nOutputSize != nBatchs * m_nOutputTensorSize) {
        return sCtx.error("输出张量尺寸和网络需要的尺寸不匹配");
    }

    SNnResizeTensor spResizeOut = spBatchOut;
    if( !spResizeOut ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    spResizeOut->getResizeData(sResizeTensor);
    spBatchIn = sResizeTensor.spExtra;

    PSolveContext& solveCtx = *m_spContext;

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
    Q* pWeightDeviaBuffer = (Q*)m_spOptimizer->getDeviationPtr(nWeights);
    memset(pWeightDeviaBuffer, 0, sizeof(Q)*nWeights);

    int nBatchInSize = spBatchIn.size();
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), m_idType, nBatchInSize) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    memset(spBatchInDeviation.data<Q>(), 0, sizeof(Q)*nBatchInSize);

    PDeviaVector solveVars[solveCtx.nVars];
    {
        Q* pItOpDevia = pOpDeviaBuffer;
        Q* pItStateDevia = pStateDeviaBuffer;
        Q* pItWeightDevia = pWeightDeviaBuffer;
        Q* pItOpVar = sResizeTensor.spSrc.data<Q>();
        PSolveContext::PSolveVar* pItVar = solveCtx.pVars;
        for( int i=0; i<solveCtx.nVars; i++, pItVar++ ) {
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                solveVars[i].size = pItVar->size;
                solveVars[i].data = pItVar->data;
                solveVars[i].devia = pItWeightDevia;
                pItWeightDevia += solveVars[i].size;
                break;

            case ENnVariableType::EVState:
                solveVars[i].size = pItVar->size;
                solveVars[i].data = pItVar->data;
                solveVars[i].devia = pItStateDevia;
                pItStateDevia += solveVars[i].size;
                break;

            case ENnVariableType::EVOperator:
                solveVars[i].size = pItVar->size*nBatchs;
                solveVars[i].data = pItOpVar;
                solveVars[i].devia = pItOpDevia;
                pItOpDevia += solveVars[i].size;
                pItOpVar += solveVars[i].size;
                break;

            case ENnVariableType::EVInput:
                solveVars[i].size = pItVar->size*nBatchs;
                solveVars[i].data = spBatchIn.data<Q>();
                solveVars[i].devia = spBatchInDeviation.data<Q>();
                break;
            }
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
    PSolveContext::PSolveInstruct* pSolver = solveCtx.pSolvers + solveCtx.nSolvers;
    while(--pSolver>=solveCtx.pSolvers) {

        //准备输入输出计算参数
        PSolveContext::PSolveInstruct instruct = *pSolver;
        for(int j=0; j<instruct.args.nInVars; j++) {
            evalIn[j] = solveVars[instruct.args.pInVars[j]];
        }
        if(instruct.args.iOutVar>=0) {
            evalOut = solveVars[instruct.args.iOutVar];
        }

        //实际计算函数调用
        (*instruct.solver.pDeviaFun)(instruct.solver.pParameter, nBatchs, instruct.args.nInVars, evalIn, evalOut);
    }

    m_spOptimizer->updateDeviation(nBatchs);
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

    PSolveContext& solveCtx = *m_spContext;
    int nWeights = solveCtx.nWeightSize;
    if(spWeightDevia.size() != nWeights) {
        return sCtx.error("数据错误，无法用于更新权重");
    }

    Q* pItWeightDevia = spWeightDevia.data<Q>();
    PSolveContext::PSolveVar* pItVar = solveCtx.pVars;
    PSolveContext::PSolveVar* pItVarEnd = pItVar+solveCtx.nVars;
    while(pItVar < pItVarEnd) {
        switch(pItVar->type) {
        case ENnVariableType::EVWeight:
            {
                Q* pItDevia = pItWeightDevia;
                Q* pItData = (Q*)(pItVar->data);
                Q* pDataEnd = pItData + pItVar->size;
                while(pItData < pDataEnd) {
                    *pItData -= *pItDevia;
                    if(*pItData > 1) {
                        *pItData = 1;
                    }else if( *pItData < -1){
                        *pItData = -1;
                    }
                    pItData++, pItDevia++;
                }
                pItWeightDevia += pItVar->size;
            }
            break;
        }
        pItVar++;
    }
    return sCtx.success();
}

int CLayerNetwork::toArchive(const SArchive& ar) {
    PNnSolver& spSolver = *m_spSolver;
    ar.arBlock("iinvar", spSolver.iInVar);
    ar.arBlock("ioutvar", spSolver.iOutVar);
    ar.arObjectArray("operators", spSolver.arrOperators);
    ar.arObjectArray("vars", spSolver.arrVars);
    ar.arBlockArray<PNnAtomOperatorArgs, vector<PNnAtomOperatorArgs>>("parameters", spSolver.arrOperatorArgs);
    ar.arObject("inputDimension", m_spInDimension);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLayerNetwork, CLayerNetwork::__getClassKey())