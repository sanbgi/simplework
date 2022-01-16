#include "CSequenceNetwork.h"

static SCtx sCtx("CSequenceNetwork");
int CSequenceNetwork::createNetwork(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
    CPointer<CSequenceNetwork> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nNetworks; i++) {
        spSequence->m_arrNetworks.push_back( *(pNetworks+i) );
    }
    spNetwork.setPtr(spSequence.getPtr());
    return sCtx.success();
}

int CSequenceNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    STensor spIn = spInTensor;
    std::vector<SNnNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor spOut;
        if( int errCode = (*it)->eval(spIn, spOut) != sCtx.success() ){
            return errCode;
        }
        spIn = spOut;
        it++;
    }
    spOutTensor = spIn;
    return sCtx.success();
}

int CSequenceNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    STensor spOut = spOutTensor;
    STensor spOutDev = spOutDeviation;
    std::vector<SNnNetwork>::reverse_iterator it = m_arrNetworks.rbegin();
    while(it != m_arrNetworks.rend() ) {
        STensor spIn;
        STensor spInDev;
        if( int errCode = (*it)->learn(spOut, spOutDev, spIn, spInDev) != sCtx.success() ) {
            return errCode;
        }
        spOut = spIn;
        spOutDev = spInDev;
        it++;
    }
    spInTensor = spOut;
    spInDeviation = spOutDev;
    return sCtx.success();
}

int CSequenceNetwork::toArchive(const SIoArchive& ar) {
    ar.visitObjectArray("nodes", m_arrNetworks);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CSequenceNetwork, CSequenceNetwork::__getClassKey())