#ifndef __SimpleWork_NN_CPoolNetwork_H__
#define __SimpleWork_NN_CPoolNetwork_H__

#include "nn.h"
#include "CActivator.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CPoolNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork);

private:
    STensor m_spInTensor;
    STensor m_spOutTensor;
    STensor m_spOutDimVector;

    int m_nWidth;
    int m_nHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;

    int m_nTensor;
    int m_nInputTensorSize;
    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayer;

    int m_nOutWidth;
    int m_nOutHeight;
    int m_nOutTensorSize;
    bool m_isTransparent;

    static SCtx sCtx;

protected:
    CPoolNetwork() {
        m_nInputWidth = 0;
        m_nInputHeight = 0;
        m_isTransparent = false;
    }

    int initNetwork(const STensor& inputTensor);
};

#endif//__SimpleWork_NN_CPoolNetwork_H__
