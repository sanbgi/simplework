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
    int getCellNumber();
    int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver);
    int learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);
    int learn(const PTensor& inputTensor, const PTensor& deltaTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork);

private:
    int m_nWidth;
    int m_nHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;

    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayer;
    bool m_isTransparent;

protected:
    CPoolNetwork() {
        m_nInputWidth = 0;
        m_nInputHeight = 0;
        m_isTransparent = false;
    }

    int initNetwork(const PTensor& inputTensor);
};

#endif//__SimpleWork_NN_CPoolNetwork_H__
