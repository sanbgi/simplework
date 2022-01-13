#include "CDenseNetwork.h"
#include <math.h>
#include <iostream>

SCtx CDenseNetwork::sCtx("CDenseNetwork");
int CDenseNetwork::createNetwork(int nCells, const char* szActivator, SNnNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    if( szActivator!=nullptr )
        spDense->m_strActivator = szActivator;
    //spDense->m_strOptimizer
    spNetwork.setPtr(spDense.getPtr());
    return sCtx.success();
}

int CDenseNetwork::prepareNetwork(const STensor& spBatchIn) {
    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn->getDataSize();
    if( nInputSize == m_nInputCells * m_nBatchs ) {
        return sCtx.success();
    }

    //
    // 计算详细维度尺寸
    //
    int nBatchs = 0;
    int nInputCells = 0;
    {
        //
        // 检查输入张量维度
        //
        STensor& spInDimTensor = spBatchIn->getDimVector();
        int nInputDims = spInDimTensor->getDataSize();
        if(nInputDims < 2) {
            return sCtx.error("输入张量维度需要大于1，其中第一个维度是批量张量个数");
        }

        //
        // 计算输入参数
        //
        int* pDimSizes = spInDimTensor->getDataPtr<int>();
        nBatchs = pDimSizes[0];
        nInputCells = pDimSizes[1];
        for( int i=2; i<nInputDims; i++) {
            nInputCells *= pDimSizes[i];
        }
        if(nBatchs * nInputCells != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 判断是否需要重新初始化网络
    //
    if(m_nInputCells==0) {
        //
        //
        //
        if( m_nCells <= 0 || nInputCells <= 0 ) {
            return sCtx.error("不允许全连接网络细胞数或输入细胞数为零");
        }
        
        m_pActivator = CActivator::getActivation(m_strActivator.c_str());
        if(m_pActivator == nullptr) {
            return sCtx.error((std::string("不支持的激活函数名: ") + m_strActivator).c_str());
        }
        if( COptimizer::getOptimizer(m_strOptimizer.c_str(), m_spOptimizer) != sCtx.success()) {
            return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
        }

        int nWeight = m_nCells*nInputCells;
        double* pWeights = new double[nWeight];
        double* pBais = new double[m_nCells];
        m_spWeights.take(pWeights, [](double* pWeights){
            delete[] pWeights;
        });
        m_spBais.take(pBais, [](double* pBais){
            delete[] pBais;
        });

        //
        // 初始化权重值，从[0-1]均匀分布（注意不是随机值）
        //
        /*
        double xWeight = 1.0/nInputCells;
        for(int i=0; i<nWeight; i++) {
            *(pWeights+i) = (rand() % 10000 / 10000.0) * xWeight;
        }*/
        double xWeight = 0.1;//sqrt(1.0/nInputCells);
        for(int i=0; i<nWeight; i++) {
            pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
        }
        for(int i=0; i<m_nCells; i++ ){
            pBais[i] = 0;
        }

        m_nBatchs = 0; //通过这个值的设置，实现之后的运行时参数必须重新初始化
        m_nInputCells = nInputCells;
    }else if(m_nInputCells != nInputCells) {
        return sCtx.error("当前输入的参数，与神经网络需要的参数不符");
    }

    if(m_nBatchs != nBatchs) {
        //
        // 检查细胞数量是否合法
        //
        int pOutDimSizes[2] = { nBatchs, m_nCells };
        if( int errCode = STensor::createVector(m_spOutDimVector, 2, pOutDimSizes) != sCtx.success() ) {
            return sCtx.error(errCode, "创建神经网络输出张量维度向量失败");
        }

        if( int errCode = STensor::createTensor<double>(m_spBatchOut, m_spOutDimVector, nBatchs * m_nCells) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输出张量失败");
        }

        m_nBatchs = nBatchs;
    }
    return sCtx.success();
}


int CDenseNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( int errCode = prepareNetwork(spBatchIn) != sCtx.success() ) {
        return errCode;
    }

    int nBatchs = m_nBatchs;
    int nOutCells = m_nCells;
    int nWeights = m_nInputCells*m_nCells;
    int nInCells = m_nInputCells;
    
    struct CItOutVariables {
        double* pIn;
        double* pOut;
        double* pWeight;
        double* pBais;
    }it = {
        spBatchIn->getDataPtr<double>(),
        m_spBatchOut->getDataPtr<double>(),
        m_spWeights,
        m_spBais
    };
    for(int iTensor=0; iTensor<nBatchs; iTensor++) {
        CItOutVariables varTBackup = {
            it.pIn,
            it.pOut,
            it.pWeight,
            it.pBais,
        };

        //
        //  调整权重
        //
        for(int iOutput=0; iOutput<nOutCells; iOutput++) {

            CItOutVariables varOBackup = {
                it.pIn,
                it.pOut,
                it.pWeight,
            };

            double dOut = 0;
            for(int iInput=0; iInput<nInCells; iInput++ ) {
                dOut += (*it.pWeight) * (*it.pIn);
                it.pIn++;
                it.pWeight++;
            }

            (*it.pOut) = dOut - (*it.pBais);

            //  更新迭代参数
            it.pOut++;
            it.pBais++;
            it.pIn = varOBackup.pIn;
            it.pWeight = varOBackup.pWeight + nInCells;
        }

        m_pActivator->activate(nOutCells, varTBackup.pOut, varTBackup.pOut);

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInCells;
        it.pOut = varTBackup.pOut + nOutCells;
        it.pWeight = varTBackup.pWeight;
        it.pBais = varTBackup.pBais;
    }

    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CDenseNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    spBatchIn = m_spBatchIn;
    if( int errCode = STensor::createTensor<double>(spBatchInDeviation, spBatchIn->getDimVector(), spBatchIn->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    int nBatchs = m_nBatchs;
    int nInCells = m_nInputCells;
    int nOutputTensorSize = m_nCells;
    int nWeights = m_nCells * m_nInputCells;
    
    double* pWeightDerivationArray = m_spOptimizer->getDeviationPtr(nWeights+m_nCells);
    double* pBaisDeviationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(double)*(nWeights+m_nCells));

    struct CItOutVariables {
        double* pIn;
        double* pInDeviation;
        double* pOut;
        double* pOutDeviation;
        double* pWeight;
        double* pWeightDeviation;
        double* pBaisDeviation;
        double* pZDeviatioin;
    }it = {
        spBatchIn->getDataPtr<double>(),
        spBatchInDeviation->getDataPtr<double>(),
        spBatchOut->getDataPtr<double>(),
        spBatchOutDeviation->getDataPtr<double>(),
        m_spWeights,
        pWeightDerivationArray,
        pBaisDeviationArray
    };
    for(int iTensor=0; iTensor<nBatchs; iTensor++) {
        CItOutVariables varTBackup = {
            it.pIn,
            it.pInDeviation,
            it.pOut,
            it.pOutDeviation,
            it.pWeight,
            it.pWeightDeviation,
            it.pBaisDeviation,
        };

        //
        //  计算目标函数相对于Y值的偏导数
        //
        double pZDerivationArray[m_nCells];
        m_pActivator->deactivate(m_nCells, it.pOut, it.pOutDeviation, pZDerivationArray);
        it.pZDeviatioin = pZDerivationArray;

        //
        //  调整权重
        //
        for(int iOutput=0; iOutput<m_nCells; iOutput++) {

            CItOutVariables varOBackup = {
                it.pIn,
                it.pInDeviation,
                it.pOut,
                it.pOutDeviation,
                it.pWeight,
                it.pWeightDeviation,
            };

            //
            //  计算目标函数对当前输出值的偏导数
            //      X = 输入
            //      Y = 权重*X-偏置
            //      F = activation(Y)
            //      delta = F - F0(目标)
            //      E = delta*delta/2 目标函数
            //      derivationZ = d(E)/d(Y) = d(E)/d(delta) * d(delta)/d(F) * d(F)/d(Y)
            //      其中：
            //          d(E)/d(delta) = pOutDeviation[iOutput]
            //          d(delta)/d(F) = 1
            //          d(F)/d(Y) = deactivate(Y)
            //
            double derivationZ = *(it.pZDeviatioin);

            //
            // 更新权重，权重值的偏导数=输出值偏导数*数入值
            //
            for(int iInput=0; iInput<m_nInputCells; iInput++ ) {

                //
                // 输入对实际目标的偏差值，反向传递给上一层，其实就是相对于输入的偏导数
                //
                // 注意：这里是否要乘以学习率？
                //      如果乘以学习率后，相当于向前传递的不是偏导数，而是偏导数 * 学习率，与现有神经网络BP算法不一致
                //      如果不乘以学习率，相当于直接向前传递偏导数，与现有神经网络BP算法一致，但含义上有点奇怪   
                //
                //pInDeviation[iInput] += derivationZ * pWeights[iWeight] * dLearnRate;
                (*it.pInDeviation) += derivationZ * (*it.pWeight);
                (*it.pWeightDeviation) += derivationZ * (*it.pIn);

                it.pIn++;
                it.pInDeviation++;
                it.pWeight++;
                it.pWeightDeviation++;
            }

            //
            // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
            //
            *it.pBaisDeviation += (-derivationZ);

            //  更新迭代参数
            it.pBaisDeviation++;
            it.pZDeviatioin++;
            it.pIn = varOBackup.pIn;
            it.pInDeviation = varOBackup.pInDeviation;
            it.pWeight = varOBackup.pWeight + nInCells;
            it.pWeightDeviation = varOBackup.pWeightDeviation + nInCells;
        }

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInCells;
        it.pInDeviation = varTBackup.pInDeviation + nInCells;
        it.pOut = varTBackup.pOut + nOutputTensorSize;
        it.pOutDeviation = varTBackup.pOutDeviation + nOutputTensorSize;
        it.pWeight = varTBackup.pWeight;
        it.pWeightDeviation = varTBackup.pWeightDeviation;
        it.pBaisDeviation = varTBackup.pBaisDeviation;
    }

    m_spOptimizer->updateDeviation(nBatchs);

    double* pWeights = m_spWeights;
    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        *pWeights -= *pWeightDerivationArray;
        pWeights++;
        pWeightDerivationArray++;
    }
    double* pBais = m_spBais;
    for(int iBais=0; iBais<m_nCells; iBais++) {
        *pBais -= *pBaisDeviationArray;
        pBais++;
        pBaisDeviationArray++;
    }
    return sCtx.success();
}

int CDenseNetwork::toArchive(const SIoArchive& ar) {
    ar.visit("nCells", m_nCells);
    ar.visitString("activator", m_strActivator);
    ar.visitString("optimizer", m_strOptimizer);

    ar.visit("nInputCells", m_nInputCells);
    if(m_nInputCells) {
        ar.visitTaker("weights", m_nCells*m_nInputCells, m_spWeights);
        ar.visitTaker("bais", m_nCells, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseNetwork, CDenseNetwork::__getClassKey())