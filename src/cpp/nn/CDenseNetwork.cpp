#include "CDenseNetwork.h"
#include <math.h>
#include <iostream>

SCtx CDenseNetwork::sCtx("CDenseNetwork");
int CDenseNetwork::createNetwork(int nCells, SNeuralNetwork::EACTIVATION eActivation, SNeuralNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spDense->m_pActivator = CActivator::getActivation(eActivation);
    spNetwork.setPtr(spDense.getPtr());
    return sCtx.Success();
}

int CDenseNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    if( int errCode = initWeights(spInTensor) != sCtx.Success() ) {
        return errCode;
    }

    int nInData = spInTensor->getDataSize();
    if(nInData != m_nInputCells * m_nInputTensor) {
        return sCtx.Error("不支持输入张量尺寸与第一次的张量尺寸不同");
    }

    if( int errCode = STensor::createTensor<double>(spOutTensor, m_spOutDimVector, m_nInputTensor*m_nCells) != sCtx.Success() ) {
        return sCtx.Error(errCode, "创建输出张量失败");
    }

    int nTensor = m_nInputTensor;
    int nWeights = m_nInputCells*m_nCells;
    int nWWsize = m_nInputCells;
    int nInputTensorSize = m_nInputCells;
    double* pWeights = m_spWeights;
    double* pBais = m_spBais;
    double* pInput = spInTensor->getDataPtr<double>();
    double* pOutput = spOutTensor->getDataPtr<double>();
    for(int iTensor=0; iTensor<nTensor; iTensor++) {
        for(int iOutput=0, iWWAt=0; iOutput<m_nCells; iOutput++, iWWAt+=nWWsize ) {
            double& dOutout = DVV(pOutput,iOutput,m_nCells);
            dOutout = 0;
            for(int iInput=0; iInput<m_nInputCells; iInput++) {
                dOutout += DVV(pWeights, iWWAt+iInput,nWeights) * DVV(pInput, iInput, nInputTensorSize);
            }
            dOutout -= DVV(pBais,iOutput, m_nCells);
        }
        m_pActivator->activate(m_nCells, pOutput, pOutput);
        pInput+=nInputTensorSize;
        pOutput+=m_nCells;
    }

    m_spInTensor = spInTensor;
    m_spOutTensor = spOutTensor;
    return sCtx.Success();
}

int CDenseNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.Error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.Success() ) {
        return sCtx.Error(errCode, "创建输入偏差张量失败");
    }

    int nTensor = m_nInputTensor;
    int nWWsize = m_nInputCells;
    int nInputTensorSize = m_nInputCells;
    int nOutputTensorSize = m_nCells;

    double* pWeightArray = m_spWeights;
    double* pBaisArray = m_spBais;

    //
    // 学习率先固定
    //
    double dLearnRate = 5.0/m_nInputCells;

    int nWeights = m_nCells * m_nInputCells;
    CTaker<double*> spWeightDeviationArray(new double[nWeights], [](double* ptr) {
        delete[] ptr;
    });
    double* pWeightDerivationArray = spWeightDeviationArray;
    memset(pWeightDerivationArray, 0 ,sizeof(double)*nWeights);

    double* pOutputArray = spOutTensor->getDataPtr<double>();
    double* pOutputDeviationArray = spOutDeviation->getDataPtr<double>();
    double* pInputArray = spInTensor->getDataPtr<double>();
    double* pInputDeviationArray = spInDeviation->getDataPtr<double>();
    for(int iTensor=0; iTensor<nTensor; iTensor++) {
        //
        //  计算目标函数相对于Y值的偏导数
        //
        double pZDerivationArray[m_nCells];
        m_pActivator->deactivate(m_nCells, pOutputArray, pOutputDeviationArray, pZDerivationArray);

        //
        //  调整权重
        //
        for(int iOutput=0, iWWAt=0; iOutput<m_nCells; iOutput++, iWWAt+=nWWsize ) {

            //
            //  计算目标函数对当前输出值的偏导数
            //      X = 输入
            //      Y = 权重*X-偏置
            //      F = activation(Y)
            //      delta = F - F0(目标)
            //      E = delta*delta/2 目标函数
            //      derivationZ = d(E)/d(Y) = d(E)/d(delta) * d(delta)/d(F) * d(F)/d(Y)
            //      其中：
            //          d(E)/d(delta) = pOutputDeviationArray[iOutput]
            //          d(delta)/d(F) = 1
            //          d(F)/d(Y) = deactivate(Y)
            //
            double derivationZ = DVV(pZDerivationArray, iOutput, m_nCells);

            //
            // 更新权重，权重值的偏导数=输出值偏导数*数入值
            //
            for(int iInput=0; iInput<m_nInputCells; iInput++ ) {

                int iWeight = iWWAt + iInput;

                //
                // 输入对实际目标的偏差值，反向传递给上一层，其实就是相对于输入的偏导数
                //
                // 注意：这里是否要乘以学习率？
                //      如果乘以学习率后，相当于向前传递的不是偏导数，而是偏导数 * 学习率，与现有神经网络BP算法不一致
                //      如果不乘以学习率，相当于直接向前传递偏导数，与现有神经网络BP算法一致，但含义上有点奇怪   
                //
                //pInputDeviationArray[iInput] += derivationZ * pWeights[iWeight] * dLearnRate;
                DVV(pInputDeviationArray, iInput, nInputTensorSize) += derivationZ * DVV(pWeightArray, iWeight, nWeights);

                DVV(pWeightDerivationArray, iWeight, nWeights) += derivationZ * DVV(pInputArray, iInput, nInputTensorSize);
            }

            //
            // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
            //
            DVV(pBaisArray, iOutput, m_nCells) -= (-derivationZ) * dLearnRate;
        }

        pOutputArray += nOutputTensorSize;
        pOutputDeviationArray += nOutputTensorSize;
        pInputArray += nInputTensorSize;
        pInputDeviationArray += nInputTensorSize;
    }

    double avgWeight = 0;
    double maxW = -100000;
    double minW = 100000;
    double avgDerivation = 0;
    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        DVV(pWeightArray, iWeight, nWeights) -= DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate;
        avgWeight += DVV(pWeightArray,iWeight,nWeights) / nWeights;
        avgDerivation += abs(DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate) / nWeights;
        if(maxW < DVV(pWeightArray,iWeight,nWeights)) {
            maxW = DVV(pWeightArray,iWeight,nWeights);
        }
        if(minW > DVV(pWeightArray,iWeight,nWeights)) {
            minW = DVV(pWeightArray,iWeight,nWeights);
        }
    }

    static int t = 0;
    if( (t++ / 2) % 20 == 0) {
        std::cout << "Dense " << nWeights << " ,Weight: " << minW << " ," << avgWeight <<" ," << maxW <<" , AD: " << avgDerivation << "\n";
    }

    /*
    //
    //  检查梯度下降后的值是否降低
    //
    struct CEvalCheck : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {
            double delta0 = 0;
            for(int i=0; i<pOutputDeviation->nData; i++) {
                delta0 += pOutputDeviation->pDoubleArray[i]*pOutputDeviation->pDoubleArray[i];
            }

            double delta = 0;
            for(int i=0; i<pOutputDeviation->nData; i++) {
                delta += (t.pDoubleArray[i] - pOutput->pDoubleArray[i] + pOutputDeviation->pDoubleArray[i]) * (t.pDoubleArray[i] - pOutput->pDoubleArray[i] + pOutputDeviation->pDoubleArray[i]);
            }

            if( delta > delta0) {
                std::cout << ", Dense" << pOutput->nData << ": " << delta0 << " --> " << delta << ", ";
            }
            return sCtx.Success();
        }
        const PTensor* pOutput;
        const PTensor* pOutputDeviation;
    }evalCheck;
    evalCheck.pOutput = &outputTensor;
    evalCheck.pOutputDeviation = &outputDeviation;

    return eval(inputTensor, &evalCheck);
    */
    
    return sCtx.Success();
}

int CDenseNetwork::initWeights(const STensor& spInTensor) {
    if(m_spOutDimVector) {
        return sCtx.Success();
    }

    STensor& spDimVector = spInTensor->getDimVector();
    int nDims = spDimVector->getDataSize();
    int* pDimSizes = spDimVector->getDataPtr<int>();
    if(nDims < 2) {
        return sCtx.Error("输入张量维度需要大于1，其中第一个维度是张量个数");
    }

    int nTensor = pDimSizes[0];
    int nInputCells = pDimSizes[1];
    for( int i=2; i<nDims; i++) {
        nInputCells *= pDimSizes[i];
    }

    //
    // 检查细胞数量是否合法
    //
    if( m_nCells <= 0 || nInputCells <= 0 ) {
        return sCtx.Error("不允许全连接网络细胞数或输入细胞数为零");
    }

    int pOutDimSizes[2] = { nTensor, m_nCells };
    if( int errCode = STensor::createVector(m_spOutDimVector, 2, pOutDimSizes) != sCtx.Success() ) {
        return sCtx.Error(errCode, "创建神经网络输出张量维度向量失败");
    }

    //
    // 如果当前权重数量大于等于目标权重数量，则无需调整
    //
    if( m_nInputCells >= nInputCells ) {
        m_nInputCells = nInputCells;
        return sCtx.Success();
    }

    if(m_nInputCells != 0) {
        m_nInputCells = 0;
        m_spWeights.release();
    }

    int nWeight = m_nCells*nInputCells;
    double* pWeight = new double[nWeight];
    double* pBais = new double[m_nCells];
    m_spWeights.take(pWeight, [](double* pWeight){
        delete[] pWeight;
    });
    m_spBais.take(pBais, [](double* pBais){
        delete[] pBais;
    });

    //
    // 初始化权重值，从[0-1]均匀分布（注意不是随机值）
    //
    double xWeight = 1.0/nInputCells;
    for(int i=0; i<nWeight; i++) {
        *(pWeight+i) = (rand() % 10000 / 10000.0) * xWeight;
    }
    for(int i=0; i<m_nCells; i++ ){
        *(pBais+i) = 0;
    }

    m_nInputCells = nInputCells;
    m_nInputTensor = nTensor;
    return sCtx.Success();
}
