#ifndef __SimpleWork_NN_CConvNetwork_H__
#define __SimpleWork_NN_CConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CConvolutionNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nConvs, SNeuralNetwork& spNetwork);

private:
    int initWeights(const STensor& inputTensor);

private:
    STensor m_spInTensor;
    STensor m_spOutTensor;
    STensor m_spOutDimVector;

    int m_nConvWidth;
    int m_nConvHeight;
    int m_nConvs;

    int m_nTensor;
    int m_nInputLayers;

    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputTensorSize;
    int m_nInputData;

    int m_nOutWidth;
    int m_nOutHeight;
    int m_nOutTensorSize;

    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;
    CActivator* m_pActivator;
    static SCtx sCtx;

public:
    CConvolutionNetwork() {
        m_nInputWidth = 0;
    }
};

#endif//__SimpleWork_NN_CConvNetwork_H__
