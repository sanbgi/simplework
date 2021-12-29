#include "CDenseNetwork.h"

int CDenseNetwork::createNetwork(int nCells, SNeuralNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spDense->m_pActivator = CActivator::getSigmod();
    spNetwork.setPtr(spDense.getPtr());
    return SError::ERRORTYPE_SUCCESS;
}

int CDenseNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {
    if(m_nInputCells == 0) {
        if( initWeights(inputTensor.nData) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
    }

    double pOutputs[m_nCells];
    {
        double* pInput = inputTensor.pDoubleArray;
        double* pWeights = m_spWeights;
        double* pBais = m_spBais;
        for(int iOutput=0; iOutput<m_nCells; iOutput++ ) {
            double& dOutout = *(pOutputs+iOutput);
            dOutout = 0;
            for(int iInput=0; iInput<m_nInputCells; iInput++) {
                dOutout += pWeights[iOutput*m_nInputCells+iInput] * pInput[iInput];
            }
            dOutout = dOutout - pBais[iOutput];
        }
        m_pActivator->activate(m_nCells, pOutputs, pOutputs);
    }
    PTensor tensorOutput;
    tensorOutput.idType = inputTensor.idType;
    tensorOutput.nData = m_nCells;
    tensorOutput.pData = pOutputs;
    tensorOutput.nDims = 1;
    tensorOutput.pDimSizes = &tensorOutput.nData;
    return pOutputReceiver->visit(tensorOutput);
}

int CDenseNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx, PTensor* pInputDeviation) {

    struct COutputReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& outputTensor) {

            int nOutputData = outputTensor.nData;
            double pOutputDelta[nOutputData];
            PTensor outputDeviation = outputTensor;
            outputDeviation.pDoubleArray = pOutputDelta;
            if( pLearnCtx->getOutputDeviation(outputTensor, outputDeviation) != SError::ERRORTYPE_SUCCESS ) {
                return SError::ERRORTYPE_FAILURE;
            }

            return pNetwork->learn(*pInputTensor, outputTensor, outputDeviation, pInputDeviation);
        }

        CDenseNetwork* pNetwork;
        double dInputWeight;
        SNeuralNetwork::ILearnCtx* pLearnCtx;
        const PTensor* pInputTensor;
        PTensor* pInputDeviation;
    }outputReceiver;
    outputReceiver.pNetwork = this;
    outputReceiver.pLearnCtx = pLearnCtx;
    outputReceiver.pInputTensor = &inputTensor;
    outputReceiver.pInputDeviation = pInputDeviation;
    return eval(inputTensor, &outputReceiver);
}

int CDenseNetwork::initWeights(int nInputCells) {
    //
    // 检查细胞数量是否合法
    //
    if( m_nCells <= 0 || nInputCells <= 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    //
    // 如果当前权重数量大于等于目标权重数量，则无需调整
    //
    if( m_nInputCells >= nInputCells ) {
        m_nInputCells = nInputCells;
        return SError::ERRORTYPE_SUCCESS;
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
    for(int i=0; i<nWeight; i++) {
        *pWeight = (i+1)/(double)nWeight;
    }
    for(int i=0; i<m_nCells; i++ ){
        *pBais = 0;
    }

    m_nInputCells = nInputCells;
    return SError::ERRORTYPE_SUCCESS;
}

int CDenseNetwork::learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& outputDeviation, PTensor* pInputDeviation) {
    double fakeArray[inputTensor.nData];
    double* pExpectInputDelta = fakeArray;
    if(pInputDeviation) {
        double* pExpectInputDelta = pInputDeviation->pDoubleArray;
        memset(pExpectInputDelta,0,sizeof(double)*inputTensor.nData);
    }

    //
    // 学习率先固定
    //
    double dLearnRate = 1;

    //
    //  计算目标函数相对于Y值的偏导数
    //
    double* pDeltaArray = outputDeviation.pDoubleArray;
    double* pOutputArray = outputTensor.pDoubleArray;
    double* pInputArray = inputTensor.pDoubleArray;
    double* pWeights = m_spWeights;
    double* pBais = m_spBais;
    double pDerivationZ[m_nCells];
    m_pActivator->deactivate(m_nCells, pOutputArray, pDeltaArray, pDerivationZ);

    //
    //  调整权重
    //
    for(int iOutput=0; iOutput<m_nCells; iOutput++ ) {

        //
        //  计算目标函数对当前输出值的偏导数
        //      X = 输入
        //      Y = 权重*X-偏置
        //      F = activation(Y)
        //      delta = 目标 - F
        //      E = delta*delta/2 目标函数
        //      derivationZ = d(E)/d(Y) = d(E)/d(delta) * d(delta)/d(F) * d(F)/d(Y)
        //      其中：
        //          d(E)/d(delta) = pDeltaArray[iOutput]
        //          d(delta)/d(F) = -1
        //          d(F)/d(Y) = deactivate(Y)
        //
        double derivationZ = pDerivationZ[iOutput];

        //
        // 更新权重，权重值的偏导数=输出值偏导数*数入值
        //
        for(int iInput=0; iInput<m_nInputCells; iInput++ ) {

            int iWeight = iOutput * m_nInputCells + iInput;

            //
            // 希望输入的下降值为，数入的偏导数
            //
            // 注意：这里是否要乘以学习率？
            //      如果乘以学习率后，相当于向前传递的不是偏导数，而是偏导数 * 学习率，与现有神经网络BP算法不一致
            //      如果不乘以学习率，相当于直接向前传递偏导数，与现有神经网络BP算法一致，但含义上有点奇怪   
            //
            //pExpectInputDelta[iInput] -= derivationZ * pWeights[iWeight] * dLearnRate;
            pExpectInputDelta[iInput] -= derivationZ * pWeights[iWeight];

            pWeights[iWeight] -= derivationZ * pInputArray[iInput] * dLearnRate;
        }

        //
        // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
        //
        pBais[iOutput] -= (-derivationZ) * dLearnRate;
    }
    
    return SError::ERRORTYPE_SUCCESS;
}
