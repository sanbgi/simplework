#include "CDenseNetwork.h"

int CDenseNetwork::getCellNumber() {
    return m_nCells;
}

int CDenseNetwork::createNetwork(int nCells, SNeuralNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spNetwork.setPtr(spDense.getPtr());
    return SError::ERRORTYPE_SUCCESS;
}

void CDenseNetwork::activate(double arrY[], double arrOutput[]) {
    for(int i=0; i<m_nCells; i++) {
        arrOutput[i] = arrY[i];
    }
}

void CDenseNetwork::deactivate(double arrOutput[], double arrDelta[], double arrDeirvatioY[]) {
    for(int i=0; i<m_nCells; i++) {
        arrDeirvatioY[i] = -arrDelta[i];
    }
}

int CDenseNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {
    if(m_nInputCells == 0) {
        if( initWeights(inputTensor.nData) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
    }

    PTensor in;
    in.idType = inputTensor.idType;
    in.nData = inputTensor.nData;
    in.pData = inputTensor.pData;
    in.nDims = 1;
    in.pDimSizes = &in.nData;

    PTensor in2;
    in2.idType = SData::getBasicTypeIdentifier<double>();
    in2.nData = m_nCells*m_nInputCells;
    in2.pDoubleArray = m_spWeights;
    in2.nDims = 2;
    int arrDimSize[] = { m_nInputCells, m_nCells };
    in2.pDimSizes = arrDimSize;

    struct CInternalReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {
            double r[t.nData];
            for(int i=0; i<t.nData; i++) {
                r[i] = t.pDoubleArray[i] - pBais[i];
            }
            pNetwork->activate(r, r);
            PTensor rtTensor;
            rtTensor.idType = t.idType;
            rtTensor.nData = t.nData;
            rtTensor.pData = r;
            rtTensor.nDims = 1;
            rtTensor.pDimSizes = &rtTensor.nData;
            return pReceiver->visit(rtTensor);
        }

        double* pBais;
        IVisitor<const PTensor&>* pReceiver;
        CDenseNetwork* pNetwork;
    }receiver;
    receiver.pBais = m_spBais;
    receiver.pReceiver = pOutputReceiver;
    receiver.pNetwork = this;
    return STensorSolver::getSolver()->multiply(in, in2, &receiver);
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
            return pLearnCtx->getOutputTarget(t, &deltaReceiver);
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

    double expectInputDelta[inputTensor.nData];
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
        double derivationY[outputTensor.nData];
        deactivate(outputTensor.pDoubleArray, deltaTensor.pDoubleArray, derivationY);
        
        //
        //  计算期望的输入偏差值
        //
        for(int iInput = 0; iInput<m_nInputCells; iInput++) {

            //
            //  数入值的偏导数 = 所有子项的和：Y的偏导数 * Y对数入的偏导数（及权重值m_spWeights[iInput*m_nInputCells+iOutput])
            //
            double derivationInput = 0;
            for(int iOutput=0; iOutput < outputTensor.nData; iOutput++) {
                derivationInput += derivationY[iOutput] * m_spWeights[iInput*m_nInputCells+iOutput];
            }

            //
            // 希望输入的下降值为，数入的偏导数
            //
            // 注意：这里是否要乘以学习率？
            //      如果乘以学习率后，相当于向前传递的不是偏导数，而是偏导数 * 学习率，与现有神经网络BP算法不一致
            //      如果不乘以学习率，相当于直接向前传递偏导数，与现有神经网络BP算法一致，但含义上有点奇怪   
            //
            //expectInputDelta[iInput] = -derivationInput * dLearnRate;
            expectInputDelta[iInput] = -derivationInput;
        }

        //
        //  调整权重
        //
        for(int iOutput=0; iOutput<m_nCells; iOutput++ ) {

            //
            //  极小化目标函数对输出值的偏导数，极小化目标函数为 ((expectOutput-output) ^ 2) / 2
            //
            double drivationY = derivationY[iOutput]; 

            //
            // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
            //
            m_spBais[iOutput] = m_spBais[iOutput] - (-drivationY) * dLearnRate;

            //
            // 更新权重，权重值的偏导数=输出值偏导数*数入值
            //
            for(int iInput=0; iInput<m_nInputCells; iInput++ ) {
                int iWeight = iInput*m_nInputCells+iOutput;
                m_spWeights[iWeight] = m_spWeights[iWeight] - drivationY * inputTensor.pDoubleArray[iInput] * dLearnRate;
            }
        }
    }

    PTensor expectInputDeltaTensor = inputTensor;
    expectInputDeltaTensor.pDoubleArray = expectInputDelta;
    return pLearnCtx->pubInputDelta(expectInputDeltaTensor);
}