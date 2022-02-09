#include "CNnLayerNetwork.h"
#include "CNnInputVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "SNnAtomSolver.h"

#include <map>
#include <iostream>
using namespace std;

static SCtx sCtx("CNnLayerNetwork");

//
// 计算变量
//
struct PSolveVar {
    ENnVariableType eVarType;//ENnVariableType
    int nVarSize;

    int size;
    void* data; //存储状态及权重值的指针
    void* devia;//存储偏倒数的指针
    CNnVariable* pVar;
};

struct PSolveInstruct {
    //
    // 输入参数及输出参数
    //
    int nInVar;
    PSolveVar *ppInVars[4];
    PSolveVar *pOutVar;

    //
    // 求解参数
    //
    PSolveParameter solver;

    //
    // 求解器
    //
    INnAtomSolver* pAtomSolver;
};

struct PSolveLayer {
    int nVars;
    PSolveVar* pVars;

    int nSolvers;
    PSolveInstruct* pSolvers;

    int nStateSize;
    int nWeightSize;
    int nOpSize;
    int iInVar;
    int iOutVar;
};

struct PLayerContext {
    //解算参数列表
    vector<PSolveVar> arrVars;
    //解算步骤列表
    vector<PSolveInstruct> arrSolvers;
    //解算参数数据
    PSolveLayer solveLayer;
};

CNnLayerNetwork::CNnLayerNetwork() {
    m_bInitialized = false;
    m_nBatchInSize = 0;
    m_idType = 0;
    m_spSolver.take(new PNnSolver, [](PNnSolver* pSolver) {
        delete pSolver;
    });
}

int CNnLayerNetwork::createNetwork(const SNnUnit& spUnit, const SDimension& spInDimVector, SNnNetwork& spNet) {
    CPointer<CNnLayerNetwork> spNetwork;
    CObject::createObject(spNetwork);
    if( CNnVariableSolver::solveUnit(spInDimVector, spUnit, spNetwork->m_spSolver) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }
    spNetwork->m_spInDimension = spInDimVector;
    spNet.setPtr(spNetwork.getPtr());
    return sCtx.success();
}

int CNnLayerNetwork::initNetwork() {
    if(m_bInitialized) {
        return sCtx.success();
    }

    SDimension spDimension = m_spInDimension;
    m_spContext.take(new PLayerContext, [](PLayerContext* pCtx){
        delete pCtx;
    });
    PLayerContext& layerCtx = *m_spContext;

    //
    // 更新计算变量数组
    //
    vector<PSolveVar>& arrVars = layerCtx.arrVars;
    vector<SNnVariable>::iterator itVar = m_spSolver->arrVars.begin();
    while(itVar != m_spSolver->arrVars.end()) {
        SNnInternalVariable spToSolveVar = *itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveVar solveVar;
        solveVar.nVarSize = spToSolveVar->getSize();
        solveVar.data = nullptr;
        solveVar.eVarType = spToSolveVar->getVariableType();
        solveVar.pVar = spToSolveVar->getVariablePtr();
        arrVars.push_back(solveVar);
        itVar++;
    }

    vector<PSolveInstruct>& arrSolvers = layerCtx.arrSolvers;
    vector<PNnSolver::PSolveParameter>::iterator itParameter = m_spSolver->arrParameters.begin();
    vector<SNnAtomSolver>::iterator itOp = m_spSolver->arrOperators.begin();
    while(itParameter != m_spSolver->arrParameters.end()) {
        PNnSolver::PSolveParameter spOp = *itParameter;
        PSolveInstruct solveParameter;
        solveParameter.nInVar = spOp.nInVars;
        if(spOp.iOutVar>=0)
            solveParameter.pOutVar = arrVars.data() + spOp.iOutVar;
        else
            solveParameter.pOutVar = nullptr;
        for(int i=0; i<spOp.nInVars; i++) {
            solveParameter.ppInVars[i] = arrVars.data() + spOp.pInVars[i];
        }
        solveParameter.pAtomSolver = (*itOp).getPtr();
        arrSolvers.push_back(solveParameter);
        itParameter++, itOp++;
    }

    //
    // 计算网络变量大小，用于后续快速生成计算缓冲区
    //
    int nStateSize = 0;
    int nWeightSize = 0;
    int nOpSize = 0;
    vector<PSolveVar>::iterator it = arrVars.begin(); 
    for(;it != arrVars.end(); it++) {
        switch(it->eVarType) {
        case ENnVariableType::EVState:
            nStateSize += it->nVarSize;
            break;

        case ENnVariableType::EVWeight:
            nWeightSize += it->nVarSize;
            break;

        case ENnVariableType::EVOperator:
            nOpSize += it->nVarSize;
            break;
        }
    }
    spDimension = m_spSolver->arrVars[m_spSolver->iOutVar].dimension();

    layerCtx.solveLayer.nOpSize = nOpSize;
    layerCtx.solveLayer.nStateSize = nStateSize;
    layerCtx.solveLayer.nWeightSize = nWeightSize;
    layerCtx.solveLayer.iInVar = m_spSolver->iInVar;
    layerCtx.solveLayer.iOutVar = m_spSolver->iOutVar;
    layerCtx.solveLayer.nVars = layerCtx.arrVars.size();
    layerCtx.solveLayer.pVars = layerCtx.arrVars.data();
    layerCtx.solveLayer.nSolvers = layerCtx.arrSolvers.size();
    layerCtx.solveLayer.pSolvers = layerCtx.arrSolvers.data();

    m_nInputTensorSize = m_spInDimension.dataSize();
    m_nOutputTensorSize = spDimension.dataSize();
    m_spOutDimension = spDimension;
    m_bInitialized = true;
    return sCtx.success();
}

int CNnLayerNetwork::prepareNetwork(const STensor& spBatchIn) {
    unsigned int idType = spBatchIn.type();
    if(spBatchIn.size() == m_nBatchInSize && spBatchIn.type() == m_idType) {
        return sCtx.success();
    }

    if(initNetwork() != sCtx.success()) {
        return sCtx.error("网络初始化失败");
    }

    //
    // 检查输入维度
    //
    SDimension spInDimVector = spBatchIn.dimension();
    int size = spInDimVector.size() - 1;
    if( size != m_spInDimension.size()) {
        return sCtx.error("输入张量的维度不符合一批输入数据的维度要求");
    }

    const int* pInDimSize = spInDimVector.data();
    {
        const int* pDim1 = spInDimVector.data() + 1;
        const int* pDim2 = m_spInDimension.data();
        while(size-->0) {
            if(*pDim1 != *pDim2) {
                return sCtx.error("输入张量的维度不符合一批输入数据的维度要求");
            }
            pDim1++, pDim2++;
        }
    }

    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    int nBatchIns = pInDimSize[0];
    PLayerContext& layerCtx = *m_spContext;

    //
    // 初始化求值函数指针
    //
    vector<PSolveInstruct>::iterator it = layerCtx.arrSolvers.begin(); 
    for(;it != layerCtx.arrSolvers.end(); it++) {
        it->pAtomSolver->initSolveParameter(idType, it->solver);
    }

    //
    // 初始化权重和状态值的指针
    //
    vector<PSolveVar>::iterator itVar = layerCtx.arrVars.begin();
    for(; itVar != layerCtx.arrVars.end(); itVar++) {
        switch(itVar->eVarType) {
        case ENnVariableType::EVState:
        case ENnVariableType::EVWeight:
            itVar->size = itVar->nVarSize;
            itVar->data = itVar->pVar->getData(idType);
            break;

        default:
            itVar->size = itVar->nVarSize * nBatchIns;
            break;
        }
    }


    m_nBatchs = nBatchIns;
    m_nBatchInSize = spBatchIn.size();
    m_idType = spBatchIn.type();
    return sCtx.success();
}

int CNnLayerNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( prepareNetwork(spBatchIn) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(m_idType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(m_idType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }

    return sCtx.error("不支持的数据类型"); 
}

template<typename Q>
int CNnLayerNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {

    int nBatchs = m_nBatchs;
    PSolveLayer& solveLayer = m_spContext->solveLayer;

    //
    // 准备计算缓冲区
    //
    if(!m_spOpSolveBuffer || m_spOpSolveBuffer.size() != solveLayer.nOpSize * nBatchs) {
        if( STensor::createVector<Q>(m_spOpSolveBuffer, solveLayer.nOpSize * nBatchs) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }
    }

    //
    // 准备输出张量
    //
    if(!m_spBatchOut || m_spBatchOut.size() != m_nOutputTensorSize * nBatchs) {
        SDimension spOutDim = m_spOutDimension.upHighDimension(nBatchs);
        if( STensor::createTensor<Q>(m_spBatchOut, spOutDim, m_nOutputTensorSize * nBatchs) != sCtx.success()) {
            return sCtx.error("创建输出张量失败");
        }
    }

    //  
    // 准备计算指针
    //  1，输入变量的数据指针指向输入数据
    //  2，运算变量指针指向缓冲区
    //
    PSolveVar *pVar, *pItVar, *pItVarEnd;
    pItVar = solveLayer.pVars;
    pItVarEnd = solveLayer.pVars+solveLayer.nVars;
    Q* pOpSolvedBuffer = m_spOpSolveBuffer.data<Q>();
    while(pItVar<pItVarEnd) {
        pVar = pItVar++;
        switch(pVar->eVarType) {
        case ENnVariableType::EVOperator:
            pVar->data = pOpSolvedBuffer;
            pOpSolvedBuffer += pVar->size;
            break;
        }
    }
    PSolveVar* pInVar = solveLayer.pVars + solveLayer.iInVar;
    PSolveVar* pOutVar = solveLayer.pVars + solveLayer.iOutVar;
    pInVar->data = spBatchIn.data<Q>();
    pOutVar->data = m_spBatchOut.data<Q>();

    //
    // 遍历计算序列并执行
    //
    PSolveInstruct *pSolver, *pItSolver, *pItSolverEnd;
    PVector evalIn[4], evalOut;
    pItSolver = solveLayer.pSolvers;
    pItSolverEnd = pItSolver + solveLayer.nSolvers;
    while(pItSolver < pItSolverEnd) {
        pSolver = pItSolver++;

        //准备输入输出计算参数
        PSolveInstruct instruct = *pSolver;
        for(int j=0; j<instruct.nInVar; j++) {
            pVar = instruct.ppInVars[j];
            evalIn[j].size = pVar->size;
            evalIn[j].data = pVar->data;
        }

        if(instruct.pOutVar!=nullptr) {
            evalOut.size = instruct.pOutVar->size;
            evalOut.data = instruct.pOutVar->data;
        }

        //实际计算函数调用
        (*instruct.solver.pEvalFun)(instruct.solver.pParameter, nBatchs, instruct.nInVar, evalIn, evalOut);
    }


    m_spBatchOut.updateVer();
    m_nBatchOutVer = m_spBatchOut.ver();
    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CNnLayerNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr() || spBatchOut.ver() != m_nBatchOutVer) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    if(spBatchOutDeviation.type() != m_idType || spBatchOutDeviation.size() != spBatchOut.size() ) {
        return sCtx.error("偏差数据类型或尺寸错误");
    }

    if(m_idType == CBasicData<double>::getStaticType()) {
        return learnT<double>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }else
    if(m_idType == CBasicData<float>::getStaticType()) {
        return learnT<float>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

template<typename Q>
int CNnLayerNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    int nBatchs = m_nBatchs;
    PSolveLayer& solveLayer = m_spContext->solveLayer;

    //
    // 准备计算缓冲区
    //
    int nDeviaBuffer = solveLayer.nOpSize * nBatchs + solveLayer.nStateSize;
    CTaker<Q*> spDeviaBuffer(new Q[nDeviaBuffer], [](Q* ptr){
        delete[] ptr;
    });
    Q* pOpDeviaBuffer = spDeviaBuffer;
    Q* pStateDeviaBuffer = pOpDeviaBuffer + solveLayer.nOpSize * nBatchs;
    memset(spDeviaBuffer, 0, sizeof(Q)*nDeviaBuffer);

    //
    // 准备权重缓冲区
    //
    int nWeights = solveLayer.nWeightSize;
    Q* pWeightDeviaBuffer = (Q*)m_spOptimizer->getDeviationPtr(nWeights);
    memset(pWeightDeviaBuffer, 0, sizeof(Q)*nWeights);

    if( int errCode = STensor::createTensor(spBatchInDeviation, m_spBatchIn.dimension(), m_idType, m_spBatchIn->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    memset(spBatchInDeviation.data<Q>(), 0, sizeof(Q)*m_nBatchInSize);
    spBatchIn = m_spBatchIn;

    PSolveVar *pItVar, *pItVarEnd;
    PSolveInstruct *pSolver, *pItSolver, *pItSolverEnd;
 
    //
    // 设置所有的权重、运算、状态的偏导指针，注意：
    //
    Q* pItOpDevia = pOpDeviaBuffer;
    Q* pItStateDevia = pStateDeviaBuffer;
    Q* pItWeightDevia = pWeightDeviaBuffer;
    Q* pOpVarBuf = m_spOpSolveBuffer.data<Q>();
    pItVar = solveLayer.pVars;
    pItVarEnd = pItVar + solveLayer.nVars;
    while(pItVar < pItVarEnd) {
        switch(pItVar->eVarType) {
        case ENnVariableType::EVWeight:
            pItVar->devia = pItWeightDevia;
            pItWeightDevia += pItVar->size;
            break;

        case ENnVariableType::EVState:
            pItVar->devia = pItStateDevia;
            pItStateDevia += pItVar->size;
            break;

        case ENnVariableType::EVOperator:
            pItVar->devia = pItOpDevia;
            pItOpDevia += pItVar->size;
            pItVar->data = pOpVarBuf;
            pOpVarBuf += pItVar->size;
            break;
        }
        pItVar++;
    }

    //
    // 准备输入数据指针
    //
    PSolveVar* pInVar = solveLayer.pVars + solveLayer.iInVar;
    pInVar->data = m_spBatchIn.data<Q>();
    pInVar->devia = spBatchInDeviation.data<Q>();

    //
    // 拷贝输出偏差
    //
    PSolveVar* pOutVar = solveLayer.pVars + solveLayer.iOutVar;
    pOutVar->devia = spBatchOutDeviation.data<Q>();

    //
    // 遍历计算序列并执行
    //
    PDeviaVector evalIn[4], evalOut;
    PSolveVar* pVar;
    pItSolver = solveLayer.pSolvers + solveLayer.nSolvers - 1;
    pItSolverEnd = solveLayer.pSolvers;
    while(pItSolver>=pItSolverEnd) {
        pSolver = pItSolver--;

        //准备输入输出计算参数
        int nInVars = pSolver->nInVar;
        for(int j=0; j<nInVars; j++) {
            pVar = pSolver->ppInVars[j];
            evalIn[j].size = pVar->size;
            evalIn[j].data = pVar->data;
            evalIn[j].devia = pVar->devia;
        }

        if(pSolver->pOutVar!=nullptr){
            pVar = pSolver->pOutVar;
            evalOut.size = pVar->size;
            evalOut.data = pVar->data;
            evalOut.devia = pVar->devia;
        }

        //实际计算函数调用
        (*pSolver->solver.pDeviaFun)(pSolver->solver.pParameter, nBatchs, nInVars, evalIn, evalOut);
    }

    m_spOptimizer->updateDeviation(nBatchs);

    //
    // 更新权重值
    //
    pItWeightDevia = pWeightDeviaBuffer;
    pItVar = solveLayer.pVars;
    pItVarEnd = pItVar+solveLayer.nVars;
    while(pItVar < pItVarEnd) {
        switch(pItVar->eVarType) {
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

int CNnLayerNetwork::toArchive(const SArchive& ar) {
    PNnSolver& spSolver = *m_spSolver;
    ar.arBlock("iinvar", spSolver.iInVar);
    ar.arBlock("ioutvar", spSolver.iOutVar);
    ar.arObjectArray("operators", spSolver.arrOperators);
    ar.arObjectArray("vars", spSolver.arrVars);
    ar.arBlockArray<PNnSolver::PSolveParameter, vector<PNnSolver::PSolveParameter>>("parameters", spSolver.arrParameters);
    ar.arObject("inputDimension", m_spInDimension);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnLayerNetwork, CNnLayerNetwork::__getClassKey())