#include "CDenseNetwork.h"

int CDenseNetwork::getCellNumber() {
    return m_nCells;
}

int CDenseNetwork::createNetwork(int nCells, SNeuralNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spDense->m_pActivator = CActivator::getReLU();
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
            dOutout = m_pActivator->activate(dOutout - pBais[iOutput]);
        }
    }
    PTensor tensorOutput;
    tensorOutput.idType = inputTensor.idType;
    tensorOutput.nData = m_nCells;
    tensorOutput.pData = pOutputs;
    tensorOutput.nDims = 1;
    tensorOutput.pDimSizes = &tensorOutput.nData;
    return pOutputReceiver->visit(tensorOutput);
}

int CDenseNetwork::learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx) {

    struct COutputReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {

            struct CDeltaReceiver : IVisitor<const PTensor&> {
                int visit(const PTensor& t) {
                    return pNetwork->learn(*pInputTensor, *pOutputTensor, t, dInputWeight, pLearnCtx);
                }

                CDenseNetwork* pNetwork;
                double dInputWeight;
                SNeuralNetwork::ILearnCtx* pLearnCtx;
                const PTensor* pOutputTensor;
                const PTensor* pInputTensor;
            }deltaReceiver;
            deltaReceiver.pNetwork = this->pNetwork;
            deltaReceiver.dInputWeight = this->dInputWeight;
            deltaReceiver.pLearnCtx = this->pLearnCtx;
            deltaReceiver.pOutputTensor = &t;
            deltaReceiver.pInputTensor = this->pInputTensor;
            return pLearnCtx->getOutputDelta(t, &deltaReceiver);
        }

        CDenseNetwork* pNetwork;
        double dInputWeight;
        SNeuralNetwork::ILearnCtx* pLearnCtx;
        const PTensor* pInputTensor;
    }outputReceiver;
    outputReceiver.pNetwork = this;
    outputReceiver.dInputWeight = dInputWeight;
    outputReceiver.pLearnCtx = pLearnCtx;
    outputReceiver.pInputTensor = &inputTensor;
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

int CDenseNetwork::learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& deltaTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx) {

    double pExpectInputDelta[inputTensor.nData];
    memset(pExpectInputDelta,0,sizeof(double)*inputTensor.nData);

    //
    // 将执行体放在括号中，达到节约栈内存目的(因为括号中的内存可以在回调之前释放)
    //
    {
        //
        // 学习率先固定
        //
        double dLearnRate = 0.1;

        //
        //  计算目标函数相对于Y值的偏导数
        //
        double* pDeltaArray = deltaTensor.pDoubleArray;
        double* pOutputArray = outputTensor.pDoubleArray;
        double* pInputArray = inputTensor.pDoubleArray;
        double* pWeights = m_spWeights;
        double* pBais = m_spBais;

        //
        //  调整权重
        //
        for(int iOutput=0; iOutput<m_nCells; iOutput++ ) {

            //
            // 计算极小化目标函数对输出值的偏导数，极小化目标函数为 ((expectOutput-output) ^ 2) / 2
            //
            double derivationOutput = m_pActivator->deactivate(pOutputArray[iOutput], pDeltaArray[iOutput]); 


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
                //pExpectInputDelta[iInput] -= derivationOutput * pWeights[iWeight] * dLearnRate;
                pExpectInputDelta[iInput] -= derivationOutput * pWeights[iWeight];

                pWeights[iWeight] -= derivationOutput * pInputArray[iInput] * dLearnRate;
            }

            //
            // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
            //
            pBais[iOutput] -= (-derivationOutput) * dLearnRate;
        }
    }

    PTensor expectInputDeltaTensor = inputTensor;
    expectInputDeltaTensor.pDoubleArray = pExpectInputDelta;
    return pLearnCtx->pubInputDelta(expectInputDeltaTensor);
}