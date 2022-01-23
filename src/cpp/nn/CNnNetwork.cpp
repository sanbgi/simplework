#include "CNnNetwork.h"
#include "CNnInput.h"
#include "CNnOperator.h"
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

int CNnNetwork::createNetwork(const SNnUnit& spUnit, const SDimension spInDimVector, SNnNetwork& spNet) {
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

    //
    // 创建输入变量
    //
    SNnVariable spInput;
    if( CNnInput::createVariable(m_spInDimVector, spInput) != sCtx.success()) {
        return sCtx.error("创建输入变量失败");
    }

    //
    // 求解网络单元，生成网络计算图
    //
    SNnVariable spOutVariable;
    if(m_spUnit->eval(spInput, spOutVariable) != sCtx.success()) {
        return sCtx.error("网络单元求解失败");
    }

    SNnInternalVariable spOutVar = spOutVariable;
    if( !spOutVar ) {
        return sCtx.error("网络单元求解结果无效");
    }

    //
    // 深度遍历网络计算图，生成计算需要的变量及步骤
    // 
    vector<PSolveVar> arrVars;
    vector<PSolveInstruct> arrSolvers;
    map<INnInternalVariable*, int> arrSolvedVars;
    int iInputVarIndex = -1;
    SNnInternalVariable spInputVar;
    vector<int> arrWeightVarIndex;
    vector<SNnInternalVariable> arrToSolveVars;
    arrToSolveVars.push_back(spOutVar);
    while(arrToSolveVars.size() > 0) {
        SNnInternalVariable spToSolveVar = arrToSolveVars.at(arrToSolveVars.size()-1);
        switch(spToSolveVar->getVariableType()) {
            case ENnVariableType::EVInput:
            {
                if(iInputVarIndex >= 0) {
                    return sCtx.error("目前网络还不支持多输入模式");
                }
                PSolveVar solveVar;
                solveVar.size = spToSolveVar->getSize();
                solveVar.data = nullptr;
                solveVar.type = ENnVariableType::EVInput;
                solveVar.pVar = spToSolveVar->getVariablePtr();
                iInputVarIndex = arrVars.size();
                spInputVar = spToSolveVar;
                arrSolvedVars[spToSolveVar.getPtr()] = arrVars.size();
                arrVars.push_back(solveVar);
                arrToSolveVars.pop_back();
            }
            break;

            case ENnVariableType::EVState:
            {
                PSolveVar solveVar;
                solveVar.size = spToSolveVar->getSize();
                solveVar.pVar = spToSolveVar->getVariablePtr();
                solveVar.type = ENnVariableType::EVState;
                arrSolvedVars[spToSolveVar.getPtr()] = arrVars.size();
                arrVars.push_back(solveVar);
                arrToSolveVars.pop_back();
            }
            break;

            case ENnVariableType::EVWeight:
            {
                PSolveVar solveVar;
                solveVar.size = spToSolveVar->getSize();
                solveVar.pVar = spToSolveVar->getVariablePtr();
                solveVar.type = ENnVariableType::EVWeight;
                arrWeightVarIndex.push_back(arrVars.size());
                arrSolvedVars[spToSolveVar.getPtr()] = arrVars.size();
                arrVars.push_back(solveVar);
                arrToSolveVars.pop_back();
            }
            break;

            case ENnVariableType::EVOperator:
            {
                SNnInternalVariable pSubVars[4];
                int nSubVars = spToSolveVar->getSubVariables(pSubVars);

                PSolveInstruct solveParameter;
                solveParameter.nInVar = nSubVars;
                int nPushback = 0;
                for( int i=0; i<nSubVars; i++) {
                    SNnInternalVariable spSubVar = pSubVars[i];
                    map<INnInternalVariable*, int>::iterator it = arrSolvedVars.find(spSubVar.getPtr());
                    if(it != arrSolvedVars.end()) {
                        arrToSolveVars.push_back(spSubVar);
                        nPushback ++;
                    }else{
                        solveParameter.pInVarIndex[i] = it->second;
                    }
                }

                if(nPushback == 0) {
                    PSolveVar solveVar;
                    solveVar.size = spToSolveVar->getSize();
                    solveVar.type = ENnVariableType::EVOperator;
                    arrSolvedVars[spToSolveVar.getPtr()] = arrVars.size();

                    solveParameter.iOutVarIndex = arrVars.size();
                    solveParameter.pOperator = (CNnOperator*)spToSolveVar->getVariablePtr();

                    arrSolvers.push_back(solveParameter);
                    arrVars.push_back(solveVar);
                    arrToSolveVars.pop_back();
                }
            }
            break;

            default:{
                return sCtx.error("未知错误，发现不认识的结算变量类型");
            }
            break;
        }
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
    
    int nDimSize = m_spInDimVector.size();
    const int* pDimSize = m_spInDimVector.data();
    if(pDimSize == nullptr || nDimSize < 1) {
        return sCtx.error("网络输入张量维度定义无效");
    } 

    int nInputTensorSize = 1;
    while(nDimSize-->0) {
        nInputTensorSize *= *pDimSize;
        pDimSize--;
    }

    m_spOutVar = spOutVar;
    m_nOpVarSize = nOpSize;
    m_nStateSize = nStateSize;
    m_nWeightSize = nWeightSize;
    m_iInputVarIndex = iInputVarIndex;
    m_nInputTensorSize = nInputTensorSize;
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
        it->pOperator->getEvalFunAddress(idType, it->pFunEval, it->pFunDevia);
    }

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
    if(!m_spSolveBuffer || m_spSolveBuffer.size() != m_nOpVarSize * m_nBatchIns) {
        if( STensor::createVector<Q>(m_spSolveBuffer, m_nOpVarSize * m_nBatchIns) != sCtx.success()) {
            return sCtx.error("创建计算缓冲区失败");
        }
    }

    //
    // 准备输出张量
    //
    if(!m_spBatchOut || m_spBatchOut.size() != m_nOutputTensorSize * m_nBatchIns) {

        SDimension spOutDimVector = m_spOutVar.dimension();
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
    Q* pBuffer = m_spSolveBuffer.data<Q>();

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
        Q* pVarBuf = pBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVInput:
                pVars[i].data = pInData;
                break;

            case ENnVariableType::EVOperator:
                pVars[i].data = pVarBuf;
                pVarBuf += pVars[i].size;
                break;
            }
        }
        pVars[nVars-1].data = pOutData;

        //
        // 遍历计算序列并执行
        //
        for(int i=0; i<nSolvers; i++) {
            //准备输入输出计算参数
            PSolveInstruct instruct = pSolver[i];
            int nIns = instruct.nInVar;
            for(int j=0; j<nIns; j++) {
                int iVar = instruct.pInVarIndex[j];
                evalIn[i].size = pVars[iVar].size;
                evalIn[i].data = pVars[iVar].data;
            }
            int iOutVarIndex = instruct.iOutVarIndex;
            evalOut.size = pVars[iOutVarIndex].size;
            evalOut.data = pVars[iOutVarIndex].data;

            //实际计算函数调用
            (*instruct.pFunEval)(instruct.pOperator, instruct.nInVar, evalIn, evalOut);
        }

        pInData += m_nInputTensorSize;
        pOutData += m_nOutputTensorSize;
        pBuffer += m_nOpVarSize;
    }

    m_spBatchOut.updateVer();
    m_nBatchOutVer = m_spBatchOut.ver();
    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.error();
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
    int nDeviaBuffer = m_nOpVarSize + m_nStateSize;
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
    // 设置所有的权重偏导指针
    //
    {
        Q* pItWeightDevia = pWeightDeviaBuffer;
        Q* pItDevia = pDeviaBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVWeight:
                pVars[i].devia = pItWeightDevia;
                pItWeightDevia + pVars[i].size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVOperator:
                pVars[i].devia = pItDevia;
                pItDevia + pVars[i].size;
                break;
            }
        }
    }

    //
    // 准备计算参数
    //
    PDeviaVector evalIn[4], evalOut;
    Q* pBuffer = m_spSolveBuffer.data<Q>();

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
        Q* pVarBuf = pBuffer;
        for(int i=0; i<nVars-1; i++) {
            switch(pVars[i].type) {
            case ENnVariableType::EVInput:
                pVars[i].data = pInData;
                pVars[i].devia = pInData;
                break;

            case ENnVariableType::EVOperator:
                pVars[i].data = pVarBuf;
                pVarBuf += pVars[i].size;
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
            int nIns = instruct.nInVar;
            for(int j=0; j<nIns; j++) {
                int iVar = instruct.pInVarIndex[j];
                evalIn[i].size = pVars[iVar].size;
                evalIn[i].data = pVars[iVar].data;
                evalIn[i].devia = pVars[iVar].devia;
            }
            int iOutVarIndex = instruct.iOutVarIndex;
            evalOut.size = pVars[iOutVarIndex].size;
            evalOut.data = pVars[iOutVarIndex].data;
            evalOut.devia = pVars[iOutVarIndex].devia;

            //实际计算函数调用
            (*instruct.pFunDevia)(instruct.pOperator, instruct.nInVar, evalIn, evalOut);
        }

        pInData += m_nInputTensorSize;
        pInDevial += m_nInputTensorSize;
        pOutData += m_nOutputTensorSize;
        pOutDevia += m_nOutputTensorSize;
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
                    pItWeightDevia + nWeights;
                }
                break;
            }
        }
    }

    m_spBatchInDeviation.updateVer();
    spBatchInDeviation = m_spBatchInDeviation;
    return sCtx.error();
}
