#include "CSequenceNetwork.h"

int CSequenceNetwork::getCellNumber() {
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
    return SError::ERRORTYPE_FAILURE;
}

int CSequenceNetwork::learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx) {
    return SError::ERRORTYPE_FAILURE;
}
