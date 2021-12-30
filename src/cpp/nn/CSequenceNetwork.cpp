#include "CSequenceNetwork.h"

SCtx CSequenceNetwork::sCtx("CSequenceNetwork");
int CSequenceNetwork::createNetwork(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
    CPointer<CSequenceNetwork> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nNetworks; i++) {
        spSequence->m_arrNetworks.push_back( *(pNetworks+i) );
    }
    spNetwork.setPtr(spSequence.getPtr());
    return sCtx.Success();
}

int CSequenceNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {
    class CInteralReceiver : public IVisitor<const PTensor&> {
    public:
        int visit(const PTensor& rData) {
            if(iPipe == pArr->size()) {
                if( pFinalReceiver ) {
                    return pFinalReceiver->visit(rData);
                }
                return sCtx.Success();
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

int CSequenceNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx, PTensor* pInputDeviation) {
    class CNextCtx : public SNeuralNetwork::ILearnCtx {
    public:
        int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeviation){
            if(iPipe == pArr->size()) {
                return pFinalCtx->getOutputDeviation(outputTensor, outputDeviation);
            }

            CNextCtx nextCtx;
            nextCtx.pArr = pArr;
            nextCtx.iPipe = iPipe+1;
            nextCtx.pFinalCtx = pFinalCtx;
            return (*pArr)[iPipe]->learn(outputTensor, &nextCtx, &outputDeviation);
        }
        int iPipe;
        std::vector<SNeuralNetwork>* pArr;
        SNeuralNetwork::ILearnCtx* pFinalCtx;
    }ctx;
    ctx.pArr = &m_arrNetworks;
    ctx.iPipe = 1;
    ctx.pFinalCtx = pLearnCtx;
    if(m_arrNetworks.size() == 0) {
        return sCtx.Error();
    }
    return m_arrNetworks[0]->learn(inputTensor, &ctx, pInputDeviation );
}
