#include "CNnNetwork.h"
#include "CNnInputVariable.h"
#include "CNnOperator.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"

#include <map>

static SCtx sCtx("CNnNetwork");
int CNnNetwork::saveNetwork(const char* szFileName, const SNnNetwork& spNet){
    return SIoFactory::getFactory()->saveArchive(szFileName, spNet);
}

int CNnNetwork::loadNetwork(const char* szFileName, SNnNetwork& spNet){
    SIoArchivable spObj; 
    if( SIoFactory::getFactory()->loadArchive(szFileName, spObj) != sCtx.success() ) {
        return sCtx.error("读取网络文件错误");
    }
    spNet = spObj;
    if( !spNet ) {
        return sCtx.error("读取的文件并非神经网络文件");
    }
    return sCtx.success();
}

int CNnNetwork::createNetwork(const SNnUnit& spUnit, const SDimension& spInDimVector, SNnNetwork& spNet) {
    CPointer<CNnNetwork> spNetwork;
    CObject::createObject(spNetwork);
    spNetwork->m_spUnit = spUnit;
    spNetwork->m_spInDimVector = spInDimVector;
    if(spNetwork->initNetwork() != sCtx.success()) {
        return sCtx.error("初始化网络失败");
    }
    spNet.setPtr(spNetwork.getPtr());
    return sCtx.success();
}

int CNnNetwork::initNetwork() {
    if(m_bInitialized) {
        return sCtx.success();
    }

    m_spSolveCtx.take(new PSolveContext(), [](PSolveContext* pCtx){
        delete pCtx;
    });

    //
    // 解算计算单元
    //
    PSolveContext& solveCtx = *m_spSolveCtx;
    if( CNnVariableSolver::solveUnit(m_spInDimVector, m_spUnit, &solveCtx) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }

    //
    // 更新计算变量数组
    //
    int iInputVarIndex = -1;
    vector<PSolveVar> arrVars;
    vector<SNnVariable>::iterator itVar = solveCtx.arrVars.begin();
    while(itVar != solveCtx.arrVars.end()) {
        SNnInternalVariable spToSolveVar = *itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveVar solveVar;
        solveVar.size = spToSolveVar->getSize();
        solveVar.data = nullptr;
        solveVar.type = spToSolveVar->getVariableType();
        solveVar.pVar = spToSolveVar->getVariablePtr();
        if(solveVar.type == ENnVariableType::EVInput) {
            iInputVarIndex = arrVars.size();
        }
        arrVars.push_back(solveVar);
        itVar++;
    }

    vector<PSolveInstruct> arrSolvers;
    vector<PSolveContext::PSolveOperator>::iterator itOp = solveCtx.arrOperators.begin();
    while(itOp != solveCtx.arrOperators.end()) {
        PSolveContext::PSolveOperator spOp = *itOp;
        PSolveInstruct solveParameter;
        solveParameter.nInVar = spOp.nInVars;
        solveParameter.iOutVarIndex = spOp.iOutVar;
        for(int i=0; i<spOp.nInVars; i++) {
            solveParameter.pInVarIndex[i] = spOp.pInVars[i];
        }
        solveParameter.pOperator = spOp.spOperator;
        arrSolvers.push_back(solveParameter);
        itOp++;
    }

    //
    // 计算网络变量大小，用于后续快速生成计算缓冲区
    //
    int nStateSize = 0;
    int nWeightSize = 0;
    int nOpSize = 0;
    vector<PSolveVar>::iterator it = arrVars.begin(); 
    for(;it != arrVars.end(); it++) {
        switch(it->type) {
        case ENnVariableType::EVInput:
            break;

        case ENnVariableType::EVState:
            nStateSize += it->size;
            break;

        case ENnVariableType::EVWeight:
            nWeightSize += it->size;
            break;

        case ENnVariableType::EVOperator:
            nOpSize += it->size;
            break;
        }
    }

    SNnInternalVariable spOutVar = solveCtx.arrVars[solveCtx.iOutVar];
    
    m_nOpSize = nOpSize;
    m_nStateSize = nStateSize;
    m_nWeightSize = nWeightSize;
    m_iInputVarIndex = iInputVarIndex;
    m_nInputTensorSize = m_spInDimVector.dataSize();
    m_nOutputTensorSize = spOutVar->getSize();
    m_arrSolvers = arrSolvers;
    m_arrVars = arrVars;
    m_bInitialized = true;
    return sCtx.success();
}

int CNnNetwork::prepareNetwork(const STensor& spBatchIn) {
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
    if( size != m_spInDimVector.size()) {
        return sCtx.error("输入张量的维度不符合一批输入数据的维度要求");
    }

    const int* pInDimSize = spInDimVector.data();
    {
        const int* pDim1 = spInDimVector.data() + 1;
        const int* pDim2 = m_spInDimVector.data();
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

    //
    // 初始化求值函数指针
    //
    vector<PSolveInstruct>::iterator it = m_arrSolvers.begin(); 
    for(;it != m_arrSolvers.end(); it++) {
        it->pOperator->getSolveParameter(idType, it->solver);
    }

    //
    // 初始化权重和状态值的指针
    //
    vector<PSolveVar>::iterator itVar = m_arrVars.begin();
    for(; itVar != m_arrVars.end(); itVar++) {
        switch(itVar->type) {
            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
            itVar->data = itVar->pVar->getData(idType);
            break;
        }
    }
    
    m_nBatchIns = pInDimSize[0];
    m_nBatchInSize = spBatchIn.size();
    m_idType = spBatchIn.type();
    return sCtx.success();
}

int CNnNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
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
int CNnNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {
    //
    // 准备计算缓冲区
    //
    if(!m_spOpSolveBuffer || m_spOpSolveBuffer.size() != m_nOpSize * m_nBatchIns) {
        if( STensor::createVector<Q>(m_spOpSolveBuffer, m_nOpSize * m_nBatchIns) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }
    }

    //
    // 准备输出张量
    //
    if(!m_spBatchOut || m_spBatchOut.size() != m_nOutputTensorSize * m_nBatchIns) {

        SNnInternalVariable spOutVar = m_spSolveCtx->arrVars[m_spSolveCtx->iOutVar];
        SDimension spOutDimVector = spOutVar.dimension();
        int nDims = spOutDimVector.size();
        const int* pDimSizes = spOutDimVector.data();

        int pOutDimSize[nDims+1];
        pOutDimSize[0] = m_nBatchIns;
        for( int i=0; i<nDims; i++ ){
            pOutDimSize[i+1] = pDimSizes[i];
        }
        
        SDimension spOutDim;
        if( SDimension::createDimension(spOutDim, nDims+1, pOutDimSize) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }

        if( STensor::createTensor<Q>(m_spBatchOut, spOutDim, m_nOutputTensorSize * m_nBatchIns) != sCtx.success()) {
            return sCtx.error("创建输出张量失败");
        }
    }
    
    PDeviaVector evalIn[4], evalOut;

    int nVars = m_arrVars.size();
    PSolveVar* pVars = m_arrVars.data();
    Q* pOpSolvedBuffer = m_spOpSolveBuffer.data<Q>();

    int nSolvers = m_arrSolvers.size();
    PSolveInstruct* pSolver = m_arrSolvers.data();

    Q* pInData = spBatchIn.data<Q>();
    Q* pOutData = m_spBatchOut.data<Q>();

    Q* pInDataEnd = pInData + spBatchIn.size();
    while(pInData < pInDataEnd) {

        //
        // 准备计算指针
        //  1，输入变量的数据指针指向输入数据
        //  2，运算变量指针指向缓冲区
        //  3，输出变量指针指向输出张量
        //
        Q* pOpVarBuf = pOpSolvedBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVOperator:
                pVars[i].data = pOpVarBuf;
                pOpVarBuf += pVars[i].size;
                break;
            }
        }
        pVars[m_iInputVarIndex].data = pInData;
        pVars[nVars-1].data = pOutData;

        //
        // 遍历计算序列并执行
        //
        for(int i=0; i<nSolvers; i++) {
            //准备输入输出计算参数
            PSolveInstruct instruct = pSolver[i];
            int nInVars = instruct.nInVar;
            for(int j=0; j<nInVars; j++) {
                int iVar = instruct.pInVarIndex[j];
                evalIn[j].size = pVars[iVar].size;
                evalIn[j].data = pVars[iVar].data;
            }
            if(instruct.iOutVarIndex>=0) {
                int iOutVarIndex = instruct.iOutVarIndex;
                evalOut.size = pVars[iOutVarIndex].size;
                evalOut.data = pVars[iOutVarIndex].data;
            }

            //实际计算函数调用
            (*instruct.solver.pEvalFun)(instruct.solver.pParameter, instruct.nInVar, evalIn, evalOut);
        }

        pInData += m_nInputTensorSize;
        pOutData += m_nOutputTensorSize;
        pOpSolvedBuffer += m_nOpSize;
    }

    m_spBatchOut.updateVer();
    m_nBatchOutVer = m_spBatchOut.ver();
    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CNnNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
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
int CNnNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {

    //
    // 准备计算缓冲区
    //
    int nDeviaBuffer = m_nOpSize + m_nStateSize;
    if(!m_spDeviaBuffer || m_spDeviaBuffer.size() != nDeviaBuffer) {
        if( STensor::createVector<Q>(m_spDeviaBuffer,nDeviaBuffer) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }
    }
    Q* pDeviaBuffer = m_spDeviaBuffer.data<Q>();

    //
    // 准备权重缓冲区
    //
    Q* pWeightDeviaBuffer = (Q*)m_spOptimizer->getDeviationPtr(m_nWeightSize);
    memset(pWeightDeviaBuffer, 0, sizeof(Q)*m_nWeightSize);

    //
    // 准备输入缓冲区
    //
    if( !m_spBatchInDeviation ) {
        if( int errCode = STensor::createTensor(m_spBatchInDeviation, m_spBatchIn.dimension(), m_idType, m_spBatchIn->getDataSize()) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输入偏差张量失败");
        }
    }else{
        m_spBatchInDeviation.updateVer();
    }
    spBatchIn = m_spBatchIn;
    spBatchInDeviation = m_spBatchInDeviation;

    int nVars = m_arrVars.size();
    PSolveVar* pVars = m_arrVars.data();

    //
    // 设置所有的权重、运算、状态的偏导指针
    //
    {
        Q* pItWeightDevia = pWeightDeviaBuffer;
        Q* pItDevia = pDeviaBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVWeight:
                pVars[i].devia = pItWeightDevia;
                pItWeightDevia += pVars[i].size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVOperator:
                pVars[i].devia = pItDevia;
                pItDevia += pVars[i].size;
                break;
            }
        }
    }

    //
    // 准备计算参数
    //
    PDeviaVector evalIn[4], evalOut;
    Q* pOpSolvedBuffer = m_spOpSolveBuffer.data<Q>();

    int nSolvers = m_arrSolvers.size();
    PSolveInstruct* pSolver = m_arrSolvers.data();

    Q* pInData = m_spBatchIn.data<Q>();
    Q* pInDevial = m_spBatchInDeviation.data<Q>();
    Q* pOutData = spBatchOut.data<Q>();
    Q* pOutDevia = spBatchOutDeviation.data<Q>();
    Q* pInDataEnd = pInData + spBatchIn.size();

    memset(pInDevial, 0, sizeof(Q)*m_nBatchInSize);
    while(pInData < pInDataEnd) {

        //重置Op和State的偏导缓冲
        memset(pDeviaBuffer, 0, sizeof(Q)*nDeviaBuffer);
        
        //
        // 准备计算指针
        //  1，输入变量的数据指针指向输入数据
        //  2，运算变量指针指向缓冲区
        //  3，输出变量指针指向输出张量
        //
        Q* pOpVarBuf = pOpSolvedBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVOperator:
                pVars[i].data = pOpVarBuf;
                pOpVarBuf += pVars[i].size;
                break;
            }
        }
        pVars[m_iInputVarIndex].data = pInData;
        pVars[m_iInputVarIndex].devia = pInDevial;
        pVars[nVars-1].data = pOutData;
        pVars[nVars-1].devia = pOutDevia;

        //
        // 遍历计算序列并执行
        //
        for(int i=nSolvers-1; i>=0; i--) {
            //准备输入输出计算参数
            PSolveInstruct instruct = pSolver[i];
            int nInVars = instruct.nInVar;
            for(int j=0; j<nInVars; j++) {
                int iVar = instruct.pInVarIndex[j];
                evalIn[j].size = pVars[iVar].size;
                evalIn[j].data = pVars[iVar].data;
                evalIn[j].devia = pVars[iVar].devia;
            }
            if(instruct.iOutVarIndex>=0){
                int iOutVarIndex = instruct.iOutVarIndex;
                evalOut.size = pVars[iOutVarIndex].size;
                evalOut.data = pVars[iOutVarIndex].data;
                evalOut.devia = pVars[iOutVarIndex].devia;
            }

            //实际计算函数调用
            (*instruct.solver.pDeviaFun)(instruct.solver.pParameter, nInVars, evalIn, evalOut);
        }

        pInData += m_nInputTensorSize;
        pInDevial += m_nInputTensorSize;
        pOutData += m_nOutputTensorSize;
        pOutDevia += m_nOutputTensorSize;
        pOpSolvedBuffer += m_nOpSize;
    }

    m_spOptimizer->updateDeviation(m_nBatchIns);

    //更新权重值
    {
        Q* pItWeightDevia = pWeightDeviaBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVWeight:
                {
                    int nWeights = pVars[i].size;
                    Q* pItDevia = pItWeightDevia;
                    Q* pItData = (Q*)(pVars[i].data);
                    Q* pDataEnd = pItData + nWeights;
                    while(pItData < pDataEnd) {
                        *pItData -= *pItDevia;
                        pItData++, pItDevia++;
                    }
                    pItWeightDevia += nWeights;
                }
                break;
            }
        }
    }

    m_spBatchInDeviation.updateVer();
    spBatchInDeviation = m_spBatchInDeviation;
    return sCtx.success();
}
