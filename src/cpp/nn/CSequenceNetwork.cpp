#include "CSequenceNetwork.h"

int CSequenceNetwork::getCellNumber() {
    /*
    int nCell = 0;
    std::vector<SNeuralNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end()) {
        nCell += (*it)->getCellNumber();
        it++;
    }
    return nCell;
    */
    return 0;
}

int CSequenceNetwork::createNetwork(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
    CPointer<CSequenceNetwork> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nNetworks; i++) {
        spSequence->m_arrNetworks.push_back( *(pNetworks+i) );
    }
    spNetwork.setPtr(spSequence.getPtr());
    return SError::ERRORTYPE_SUCCESS;
}

int CSequenceNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {

    class CInteralReceiver : public IVisitor<const PTensor&> {
    public:
        int visit(const PTensor& rData) {
            if(iPipe == pArr->size()) {
                if( pFinalReceiver ) {
                    return pFinalReceiver->visit(rData);
                }
                return SError::ERRORTYPE_SUCCESS;
            }

            CInteralReceiver receiver;
            receiver.pFinalReceiver = pFinalReceiver;
            receiver.iPipe = iPipe+1;
            receiver.pArr = pArr;
            return pArr->at(iPipe)->eval(rData, &receiver);
        }
        int iPipe;
        std::vector<SNeuralNetwork>* pArr;
        IVisitor<const PTensor&>* pFinalReceiver;
    }receiver;
    receiver.pArr = &m_arrNetworks;
    receiver.iPipe = 0;
    receiver.pFinalReceiver = pOutputReceiver;
    return receiver.visit(inputTensor);
}

int CSequenceNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx) {
    class CInteralCtx : public SNeuralNetwork::ILearnCtx {
    public:
        int getOutputDelta(const PTensor& outputTensor, IVisitor<const PTensor&>* pDeltaReceiver){
            if(iPipe == pArr->size()) {
                return pFinalCtx->getOutputDelta(outputTensor, pDeltaReceiver);
            }

            CInteralCtx nextCtx;
            nextCtx.pArr = pArr;
            nextCtx.iPipe = iPipe+1;
            nextCtx.dInputWeight = dInputWeight;
            nextCtx.pFinalCtx = pFinalCtx;
            nextCtx.pDeltaReceiver = pDeltaReceiver;
            return (*pArr)[iPipe]->learn(outputTensor, &nextCtx);
        }

        int setInputDelta(const PTensor& inputDelta) {
            if(pDeltaReceiver) {
                return pDeltaReceiver->visit(inputDelta);
            }
            return pFinalCtx->setInputDelta(inputDelta);
        }
        int iPipe;
        double dInputWeight;
        std::vector<SNeuralNetwork>* pArr;
        SNeuralNetwork::ILearnCtx* pFinalCtx;
        IVisitor<const PTensor&>* pDeltaReceiver;
    }receiver;
    receiver.pArr = &m_arrNetworks;
    receiver.iPipe = 1;
    receiver.pFinalCtx = pLearnCtx;
    receiver.pDeltaReceiver = nullptr;
    if(m_arrNetworks.size() == 0) {
        return SError::ERRORTYPE_FAILURE;
    }
    return m_arrNetworks[0]->learn(inputTensor, &receiver);
}
