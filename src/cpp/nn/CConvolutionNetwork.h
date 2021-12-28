#ifndef __SimpleWork_NN_CConvNetwork_H__
#define __SimpleWork_NN_CConvNetwork_H__

#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CConvolutionNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int getCellNumber();
    int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver);
    int learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);
    int learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& deltaTener, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nConvs, SNeuralNetwork& spNetwork);

private:
    int initWeights(const PTensor& inputTensor);
    double activate(double v);
    double deactivate(double dOutput, double dDelta);

private:
    int m_nConvWidth;
    int m_nConvHeight;
    int m_nConvs;
    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayers;
    int m_nInputData;
    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;

public:
    CConvolutionNetwork() {
        m_nInputWidth = 0;
    }
};

#endif//__SimpleWork_NN_CConvNetwork_H__
