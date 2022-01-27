#include "CNnLayerNetwork.h"
#include "CNnInputVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "SNnOperator.h"

#include <map>
#include <iostream>
using namespace std;

static SCtx sCtx("CNnLayerNetwork");

//
// 计算变量
//
struct PSolveVar {
    int type;//ENnVariableType
    int size;
    void* data; //存储状态及权重值的指针
    void* devia;//存储偏倒数的指针
    CNnVariable* pVar;
};

struct PSolveInstruct {
    //
    // 输入参数个数及位置
    //
    int nInVar;
    int pInVarIndex[4];

    //
    // 输出参数位置
    //
    int iOutVar;

    //
    // 求解参数
    //
    PSolveParameter solver;

    //
    // 求解器
    //
    SNnOperator pOperator;
};

struct PLayerContext {
    //求解结果
    PSolveContext solveCtx;
    //解算参数列表
    vector<PSolveVar> arrVars;
    //解算步骤列表
    vector<PSolveInstruct> arrSolvers;
    //层运行模式
    ENnLayerMode eMode;
    int nStateSize;
    int nWeightSize;
    int nOpSize;
    int iInVar;
    int iOutVar;
    int nBatchs;
    void* pOutData;
    void* pOutDevia;
};


int CNnLayerNetwork::createNetwork(int nLayers, const SNnLayer pLayers[], const SDimension& spInDimension, SNnNetwork& spNet) {
    CPointer<CNnLayerNetwork> spNetwork;
    CObject::createObject(spNetwork);
    for(int i=0; i<nLayers; i++) {
        spNetwork->m_arrLayers.push_back(pLayers[i]);
    }
    spNetwork->m_spInDimension = spInDimension;
    if(spNetwork->initNetwork() != sCtx.success()) {
        return sCtx.error("初始化网络失败");
    }

    spNet.setPtr(spNetwork.getPtr());
    return sCtx.success();
}

int CNnLayerNetwork::initNetwork() {
    if(m_bInitialized) {
        return sCtx.success();
    }

    int nSumOpSize = 0;
    int nSumStateSize = 0;
    int nSumWeightSize = 0;
    SDimension spDimension = m_spInDimension;
    for(int i=0, nLayers=m_arrLayers.size(); i<nLayers; i++) {
        SNnLayer spLayer = m_arrLayers[i];
        CTaker<PLayerContext*> spCtx(new PLayerContext, [](PLayerContext* pCtx){
            delete pCtx;
        });

        int nBatchs = 1;
        ENnLayerMode eMode = spLayer->getMode();
        switch (eMode)
        {
        case SNnLayer::EMODE_BATCH:
        case SNnLayer::EMODE_SEQUENCE:
            {
                if(spDimension.size() < 2) {
                    return sCtx.error("输入张量维度小于二，不能启用BATCH和Sequence模式");
                }
                nBatchs = *spDimension.data();
                spDimension = SDimension(spDimension.size()-1, spDimension.data() + 1);
            }
            break;
        }

        //
        // 解算计算单元
        //
        PLayerContext& layerCtx = *spCtx;
        PSolveContext& solveCtx = layerCtx.solveCtx;
        if( CNnVariableSolver::solveUnit(spDimension, spLayer.getUnit(), &solveCtx) != sCtx.success()) {
            return sCtx.error("解算网络单元错误");
        }

        //
        // 更新计算变量数组
        //

        vector<PSolveVar>& arrVars = layerCtx.arrVars;
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
            arrVars.push_back(solveVar);
            itVar++;
        }

        vector<PSolveInstruct>& arrSolvers = layerCtx.arrSolvers;
        vector<PSolveContext::PSolveOperator>::iterator itOp = solveCtx.arrOperators.begin();
        while(itOp != solveCtx.arrOperators.end()) {
            PSolveContext::PSolveOperator spOp = *itOp;
            PSolveInstruct solveParameter;
            solveParameter.nInVar = spOp.nInVars;
            solveParameter.iOutVar = spOp.iOutVar;
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

        spDimension = solveCtx.arrVars[solveCtx.iOutVar].dimension();
        switch(eMode) {
        case SNnLayer::EMODE_BATCH:
            {
                int nDims = spDimension.size();
                int pDimSizes[nDims+1] = {nBatchs};
                const int* pUnitDimSizes = spDimension.data();
                for(int i=0; i<nDims; i++) {
                    pDimSizes[i+1] = pUnitDimSizes[i];
                }
                spDimension = SDimension(nDims+1, pDimSizes);
            }
        }
        layerCtx.eMode = eMode;
        layerCtx.nOpSize = nOpSize;
        layerCtx.nStateSize = nStateSize;
        layerCtx.nWeightSize = nWeightSize;
        layerCtx.nBatchs = nBatchs;
        layerCtx.iInVar = solveCtx.iInVar;
        layerCtx.iOutVar = solveCtx.iOutVar;
        nSumOpSize += nOpSize * nBatchs;
        nSumStateSize += nStateSize;
        nSumWeightSize += nWeightSize;
        m_arrLayerCtx.push_back(spCtx.untake());
    }

    m_nInputTensorSize = m_spInDimension.dataSize();
    m_nOutputTensorSize = spDimension.dataSize();
    m_spOutDimension = spDimension;
    m_nOpSize = nSumOpSize;
    m_nStateSize = nSumStateSize;
    m_nWeightSize = nSumWeightSize;
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

    vector<PLayerContext*>::iterator itLayer = m_arrLayerCtx.begin();
    while(itLayer != m_arrLayerCtx.end()) {

        PLayerContext& layerCtx = *(*itLayer);

        //
        // 初始化求值函数指针
        //
        vector<PSolveInstruct>::iterator it = layerCtx.arrSolvers.begin(); 
        for(;it != layerCtx.arrSolvers.end(); it++) {
            it->pOperator->getSolveParameter(idType, it->solver);
        }

        //
        // 初始化权重和状态值的指针
        //
        vector<PSolveVar>::iterator itVar = layerCtx.arrVars.begin();
        for(; itVar != layerCtx.arrVars.end(); itVar++) {
            switch(itVar->type) {
                case ENnVariableType::EVState:
                case ENnVariableType::EVWeight:
                itVar->data = itVar->pVar->getData(idType);
                break;
            }
        }

        itLayer++;
    }
    
    m_nBatchIns = pInDimSize[0];
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

        int nDims = m_spOutDimension.size();
        const int* pDimSizes = m_spOutDimension.data();

        int pOutDimSize[nDims+1];
        pOutDimSize[0] = m_nBatchIns;
        for( int i=0; i<nDims; i++ ){
            pOutDimSize[i+1] = pDimSizes[i];
        }
        
        SDimension spOutDim = SDimension(nDims+1, pOutDimSize);
        if( STensor::createTensor<Q>(m_spBatchOut, spOutDim, m_nOutputTensorSize * m_nBatchIns) != sCtx.success()) {
            return sCtx.error("创建输出张量失败");
        }
    }
    
    PDeviaVector evalIn[4], evalOut;
    Q* pOpSolvedBuffer = m_spOpSolveBuffer.data<Q>();

    int nLayers = m_arrLayerCtx.size();
    PLayerContext** ppLayers = m_arrLayerCtx.data();

    Q* pInData = spBatchIn.data<Q>();
    Q* pOutData = m_spBatchOut.data<Q>();
    Q* pInDataEnd = pInData + spBatchIn.size();
    while(pInData < pInDataEnd) {
        Q* pOpVarBuf = pOpSolvedBuffer;
        PLayerContext** itPpLayer = ppLayers;
        for(int iLayer=0; iLayer<nLayers; iLayer++) {
            PLayerContext* pLayer = *itPpLayer;
            
            int nVars = pLayer->arrVars.size();
            PSolveVar* pVars = pLayer->arrVars.data();

            int nSolvers = pLayer->arrSolvers.size();
            PSolveInstruct* pSolver = pLayer->arrSolvers.data();

            int nBatchs = pLayer->nBatchs;
            //  
            // 准备计算指针
            //  1，输入变量的数据指针指向输入数据
            //  2，运算变量指针指向缓冲区
            //
            for(int i=0; i<nVars; i++) {
                switch(pVars[i].type) {
                case ENnVariableType::EVOperator:
                    pVars[i].data = pOpVarBuf;

                    #ifdef _DEBUG
                    pVars[i].devia = pVars[i].data;
                    #endif//_DEBUG

                    pOpVarBuf += pVars[i].size * nBatchs;
                    break;
                }
            }

            //
            // 设置层输出指针，如果是SEQUENCE模式，则把层输出指针指向最后一个计算数据
            //
            PSolveVar* pOutVar = pVars + pLayer->iOutVar;
            pLayer->pOutData = pOutVar->data;
            if(pLayer->eMode == ENnLayerMode::EMODE_SEQUENCE) {
                int nOffset = pOutVar->size*(nBatchs-1);
                pLayer->pOutData = ((Q*)pLayer->pOutData)+nOffset;
            }

            //
            // 准备输入数据指针，逻辑是：
            //  1，如果是第一层，则指向原始输入指针
            //  2，如果不是第一层，则指向上一层输出的指针
            //
            PSolveVar* pInVar = pVars + pLayer->iInVar;
            if(iLayer == 0) {
                pInVar->data = pInData;
            }
            else {
                pInVar->data = (*(itPpLayer-1))->pOutData;
            }

            //
            // 遍历所有批次，依次运算
            //
            for(int iBatch=0; iBatch<nBatchs; iBatch++) {
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

                    if(instruct.iOutVar>=0) {
                        int iOutVarIndex = instruct.iOutVar;
                        evalOut.size = pVars[iOutVarIndex].size;
                        evalOut.data = pVars[iOutVarIndex].data;
                    }

                    //实际计算函数调用
                    (*instruct.solver.pEvalFun)(instruct.solver.pParameter, instruct.nInVar, evalIn, evalOut);
                }

                //
                //  如果不是最后一个，则所有输入和运算的指针都要向后移动步长
                //  如果是最后一个，当层模式为BATCH时，需要恢复指针位置
                //
                if(nBatchs > 1) {
                    if(iBatch < nBatchs-1) {
                        for(int i=0; i<nVars; i++) {
                            switch(pVars[i].type) {
                            case ENnVariableType::EVInput:
                            case ENnVariableType::EVOperator:
                                pVars[i].data = ((Q*)pVars[i].data) + pVars[i].size;
                                break;
                            }
                        }
                    }
                    else{
                        for(int i=0; i<nVars; i++) {
                            switch(pVars[i].type) {
                            case ENnVariableType::EVInput:
                            case ENnVariableType::EVOperator:
                                pVars[i].data = ((Q*)pVars[i].data) - pVars[i].size * (nBatchs-1);
                                break;
                            }
                        }
                    }
                }
            }
            #ifdef _DEBUG
                for(int i=0; i<nVars; i++) {
                    switch(pVars[i].type) {
                    case ENnVariableType::EVOperator:
                        VERIFY(pVars[i].data == pVars[i].devia);
                        break;
                    }
                }
            #endif//_DEBUG

            //
            // 如果是最后一层，则拷贝输出结果，注意：此时输出变量的指针指向一批次中的最后一次结果，
            //  所以，实际数据取数据结尾指针 - m_nOutputTensorSize;
            //
            if(iLayer==nLayers-1) {
                PSolveVar* pOutVar = &pVars[pLayer->iOutVar];
                memcpy(pOutData,(Q*)pOutVar->data,sizeof(Q)*m_nOutputTensorSize);
            }
            itPpLayer++;
        }
        pInData += m_nInputTensorSize;
        pOutData += m_nOutputTensorSize;
        pOpSolvedBuffer += m_nOpSize;
        VERIFY(pOpVarBuf==pOpSolvedBuffer)
    }
    VERIFY(pInData==pInDataEnd)
    VERIFY(pOutData==(m_spBatchOut.data<Q>()+m_nOutputTensorSize*m_nBatchIns))
    VERIFY((pOpSolvedBuffer-m_spOpSolveBuffer.data<Q>())==(m_nOpSize*m_nBatchIns))

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

    //
    // 准备计算缓冲区
    //
    int nDeviaBuffer = m_nOpSize + m_nStateSize;
    if(!m_spDeviaBuffer || m_spDeviaBuffer.size() != nDeviaBuffer) {
        if(STensor::createVector<Q>(m_spDeviaBuffer, nDeviaBuffer) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }
    }
    Q* pOpDeviaBuffer = m_spDeviaBuffer.data<Q>();
    Q* pStateDeviaBuffer = pOpDeviaBuffer+m_nOpSize;

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

    int nLayers = m_arrLayerCtx.size();
    PLayerContext** ppLayers = m_arrLayerCtx.data();
    PLayerContext** itPpLayer = ppLayers;
    Q* pItWeightDevia = pWeightDeviaBuffer;
    Q* pItOpDevia = pOpDeviaBuffer;
    Q* pItStateDevia = pOpDeviaBuffer+m_nOpSize;
    for(int iLayer=0; iLayer<nLayers; iLayer++) {
        PLayerContext* pLayer = *itPpLayer;

        int nBatchs = pLayer->nBatchs;
        int nVars = pLayer->arrVars.size();

        //
        // 设置所有的权重、运算、状态的偏导指针，注意：
        //
        PSolveVar* pVars = pLayer->arrVars.data();
        for(int i=0; i<nVars; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVWeight:
                pVars[i].devia = pItWeightDevia;
                pItWeightDevia += pVars[i].size;
                break;

            case ENnVariableType::EVState:
                pVars[i].devia = pItStateDevia;
                pItStateDevia += pVars[i].size;
                break;

            case ENnVariableType::EVOperator:
                pVars[i].devia = pItOpDevia;
                pItOpDevia += pVars[i].size*nBatchs;
                break;
            }
        }

        itPpLayer++;
    }
    #ifdef _DEBUG
        if(pItOpDevia - pOpDeviaBuffer != m_nOpSize || pItWeightDevia - pWeightDeviaBuffer != m_nWeightSize) {
            return sCtx.error("内部异常");
        }
    #endif//_DEBUG

    //
    // 准备计算参数
    //
    PDeviaVector evalIn[4], evalOut;
    Q* pOpSolvedBuffer = m_spOpSolveBuffer.data<Q>() + (m_nBatchIns-1)*m_nOpSize;
    Q* pInData = m_spBatchIn.data<Q>() + (m_nBatchIns-1)*m_nInputTensorSize;
    Q* pInDevial = m_spBatchInDeviation.data<Q>() + (m_nBatchIns-1)*m_nInputTensorSize;
    Q* pOutData = spBatchOut.data<Q>() + (m_nBatchIns-1)*m_nOutputTensorSize;
    Q* pOutDevia = spBatchOutDeviation.data<Q>() + (m_nBatchIns-1)*m_nOutputTensorSize;
    Q* pInDataEnd = pInData - spBatchIn.size();
    memset(m_spBatchInDeviation.data<Q>(), 0, sizeof(Q)*m_nBatchInSize);
    //重置Op和State的偏导缓冲
    memset(pOpDeviaBuffer, 0, sizeof(Q)*nDeviaBuffer);
    while(pInData > pInDataEnd) {
        memset(pOpDeviaBuffer, 0, sizeof(Q)*m_nOpSize);

        //
        // 正向设置指针运算数据指针，这一步以后，偏导数、数据指针都已经就绪
        //
        itPpLayer = ppLayers;
        Q* pOpVarBuf = pOpSolvedBuffer;
        for( int iLayer=0; iLayer<nLayers; iLayer++) {
            PLayerContext* pLayer = *itPpLayer;
            int nBatchs = pLayer->nBatchs;
            int nVars = pLayer->arrVars.size();
            PSolveVar* pVars = pLayer->arrVars.data();
            for(int i=0; i<nVars; i++) {
                switch(pVars[i].type) {
                case ENnVariableType::EVOperator:
                    {
                        pVars[i].data = pOpVarBuf;
                        pOpVarBuf += pVars[i].size * nBatchs;
                    }
                    break;
                }
            }

            PSolveVar* pOutVar = pVars + pLayer->iOutVar;
            pLayer->pOutData = pOutVar->data;
            pLayer->pOutDevia = pOutVar->devia;

            //
            // 如果是SEQUENCE模式，则把层输出指针指向最后一个计算数据
            //
            if(pLayer->eMode == ENnLayerMode::EMODE_SEQUENCE) {
                int nOffset = pOutVar->size*(nBatchs-1);
                pLayer->pOutData = ((Q*)pLayer->pOutData)+nOffset;
                pLayer->pOutDevia = ((Q*)pLayer->pOutDevia)+nOffset;
            }
            itPpLayer++;
        }
        VERIFY(pOpVarBuf-pOpSolvedBuffer==m_nOpSize)

        //
        // 反向求解
        //
        itPpLayer = ppLayers + nLayers - 1;
        for(int iLayer=nLayers-1; iLayer>=0; iLayer--){
            PLayerContext* pLayer = *itPpLayer;

            //
            // 准备层参数
            //
            int nBatchs = pLayer->nBatchs;
            int nVars = pLayer->arrVars.size();
            PSolveVar* pVars = pLayer->arrVars.data();

            int nSolvers = pLayer->arrSolvers.size();
            PSolveInstruct* pSolver = pLayer->arrSolvers.data();

            //
            // 准备输入数据指针，比较复杂，大体逻辑是：
            //  1，如果是第一层，则指向原始输入指针
            //  2，如果不是第一层，则指向上一层输出的指针
            //
            PSolveVar * pInVar = pVars + pLayer->iInVar;
            if(iLayer == 0) {
                pInVar->data = pInData;
                pInVar->devia = pInDevial;
            }
            else {
                PLayerContext* pPrevLayer = *(itPpLayer-1);
                pInVar->data = pPrevLayer->pOutData;
                pInVar->devia = pPrevLayer->pOutDevia;
            }

            //
            // 如果是最后一层，拷贝偏导数
            //
            if(iLayer==nLayers-1) {
                PSolveVar * pOutVar = pVars + pLayer->iOutVar;
                memcpy(pOutVar->devia, pOutDevia, sizeof(Q)*m_nOutputTensorSize);
            }

            //
            // 如果是BATCH或SEQUENCE模式，由于输出的时候是指向第一个，但计算的时候需要从最后一个开始计算，
            //  所以，所有op的指针向后移一个偏置
            //
            if( pLayer->eMode == ENnLayerMode::EMODE_BATCH ||
                pLayer->eMode == ENnLayerMode::EMODE_SEQUENCE ) {
                for(int i=0; i<nVars; i++) {
                    switch(pVars[i].type) {
                    case ENnVariableType::EVInput:
                    case ENnVariableType::EVOperator:
                        {
                            int offset = pVars[i].size * (nBatchs-1);
                            pVars[i].data = ((Q*)pVars[i].data) + offset;
                            pVars[i].devia = ((Q*)pVars[i].devia) + offset;
                        }
                        break;
                    }
                }
            }

            //
            // 遍历所有批次，依次运算
            //
            for(int iBatch=0; iBatch<nBatchs; iBatch++) {
                //
                // 遍历计算序列并执行
                //
                for(int iSolver=nSolvers-1; iSolver>=0; iSolver--) {
                    //准备输入输出计算参数
                    PSolveInstruct instruct = pSolver[iSolver];
                    int nInVars = instruct.nInVar;
                    for(int j=0; j<nInVars; j++) {
                        int iVar = instruct.pInVarIndex[j];
                        evalIn[j].size = pVars[iVar].size;
                        evalIn[j].data = pVars[iVar].data;
                        evalIn[j].devia = pVars[iVar].devia;
                    }

                    if(instruct.iOutVar>=0){
                        int iOutVarIndex = instruct.iOutVar;
                        evalOut.size = pVars[iOutVarIndex].size;
                        evalOut.data = pVars[iOutVarIndex].data;
                        evalOut.devia = pVars[iOutVarIndex].devia;
                    }

                    //实际计算函数调用
                    (*instruct.solver.pDeviaFun)(instruct.solver.pParameter, nInVars, evalIn, evalOut);
                }

                //
                // 如果不是最后一个，则所有输入和运算的指针都要向前移动步长
                //
                if( pLayer->eMode == ENnLayerMode::EMODE_SEQUENCE ||
                    pLayer->eMode == ENnLayerMode::EMODE_BATCH) {
                    if(iBatch < nBatchs-1) {
                        for(int i=0; i<nVars; i++) {
                            switch(pVars[i].type) {
                            case ENnVariableType::EVInput:
                            case ENnVariableType::EVOperator:
                                {
                                    int size = pVars[i].size;
                                    pVars[i].data = ((Q*)pVars[i].data) - size;
                                    pVars[i].devia = ((Q*)pVars[i].devia) - size;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            itPpLayer--;
        }

        #ifdef _DEBUG
            VERIFY(pOpVarBuf-pOpSolvedBuffer==m_nOpSize)
            itPpLayer = ppLayers;
            pOpVarBuf = pOpSolvedBuffer;
            for( int iLayer=0; iLayer<nLayers; iLayer++) {
                PLayerContext* pLayer = *itPpLayer;
                int nBatchs = pLayer->nBatchs;
                int nVars = pLayer->arrVars.size();
                PSolveVar* pVars = pLayer->arrVars.data();

                for(int i=0; i<nVars; i++) {
                    switch(pVars[i].type) {
                    case ENnVariableType::EVOperator:
                        {
                            VERIFY(pVars[i].data == pOpVarBuf);
                            pOpVarBuf += pVars[i].size * nBatchs;
                        }
                        break;
                    }
                }
    
                itPpLayer++;
            }
        #endif//

        pInData -= m_nInputTensorSize;
        pInDevial -= m_nInputTensorSize;
        pOutData -= m_nOutputTensorSize;
        pOutDevia -= m_nOutputTensorSize;
        pOpSolvedBuffer -= m_nOpSize;
    }

    #ifdef _DEBUG
    itPpLayer = ppLayers;
    pItWeightDevia = pWeightDeviaBuffer;
    pItOpDevia = pOpDeviaBuffer;
    pItStateDevia = pOpDeviaBuffer+m_nOpSize;
    for(int iLayer=0; iLayer<nLayers; iLayer++) {
        PLayerContext* pLayer = *itPpLayer;

        int nBatchs = pLayer->nBatchs;
        int nVars = pLayer->arrVars.size();

        //
        // 设置所有的权重、运算、状态的偏导指针，注意：
        //  op的指针是指向它输出时的状态，便于接收下一层传递上来的偏导，所以：
        //      SEQUENCE模式，指向最后一个
        //      其它模式指向第一个
        //
        PSolveVar* pVars = pLayer->arrVars.data();
        for(int i=0; i<nVars; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVWeight:
                VERIFY(pVars[i].devia == pItWeightDevia);
                pItWeightDevia += pVars[i].size;
                break;

            case ENnVariableType::EVState:
                VERIFY(pVars[i].devia == pItStateDevia);
                pItStateDevia += pVars[i].size;
                break;

            case ENnVariableType::EVOperator:
                VERIFY(pVars[i].devia == pItOpDevia);
                pItOpDevia += pVars[i].size*nBatchs;
                break;
            }
        }
        itPpLayer++;
    }
    #endif//

    m_spOptimizer->updateDeviation(m_nBatchIns);

    //更新权重值
    {
        Q* pItWeightDevia = pWeightDeviaBuffer;
        itPpLayer = ppLayers;
        for(int iLayer=0; iLayer<nLayers; iLayer++) {
            PLayerContext* pLayer = *itPpLayer;

            int nVars = pLayer->arrVars.size();
            PSolveVar* pVars = pLayer->arrVars.data();
            for(int i=0; i<nVars; i++) {
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
            itPpLayer++;
        }
        VERIFY(pItWeightDevia-pWeightDeviaBuffer==m_nWeightSize)
    }

    m_spBatchInDeviation.updateVer();
    spBatchInDeviation = m_spBatchInDeviation;
    return sCtx.success();
}

void CNnLayerNetwork::releaseCtx() {
    vector<PLayerContext*>::iterator it = m_arrLayerCtx.begin();
    while(it != m_arrLayerCtx.end()) {
        delete *it;
        it++;
    }
    m_arrLayerCtx.clear();
}


int CNnLayerNetwork::toArchive(const SIoArchive& ar) {
    ar.visitObjectArray("layers", m_arrLayers);
    ar.visitObject("inputDimension", m_spInDimension);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnLayerNetwork, CNnLayerNetwork::__getClassKey())