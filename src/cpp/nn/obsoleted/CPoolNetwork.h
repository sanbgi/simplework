#ifndef __SimpleWork_NN_CPoolNetwork_H__
#define __SimpleWork_NN_CPoolNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include <string>

using namespace sw;
using namespace std;

class CPoolNetwork : public CObject, public INnNetwork, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "PoolNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static const char* __getClassKey() { return "sw.nn.PoolNetwork"; }
    static int createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNnNetwork& spNetwork);

private:
    int m_nPoolWidth;
    int m_nPoolHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;
    string m_strPadding;

    int m_nBatchInSize;
    unsigned int m_idDataType;
    int m_nInVer;
    int m_nOutVer;
    int m_nBatchs;
    int m_nInputTensorSize;
    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayer;

    int m_nOutWidth;
    int m_nOutHeight;
    int m_nOutTensorSize;

    STensor m_spBatchIn;
    STensor m_spBatchOut;
    SDimension m_spOutDimVector;

protected:
    CPoolNetwork() {
        m_nPoolWidth = -1;
        m_nPoolHeight = -1;
        m_nBatchInSize = -1;
        m_idDataType = -1;
        m_nInVer = m_nOutVer = 0;
    }

    int prepareNetwork(const STensor& inputTensor);
};

#endif//__SimpleWork_NN_CPoolNetwork_H__
