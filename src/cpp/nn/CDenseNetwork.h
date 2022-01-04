#ifndef __SimpleWork_NN_CDenseNetwork_H__
#define __SimpleWork_NN_CDenseNetwork_H__

#include "nn.h"
#include "CActivator.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

//
//  网络定义：
//      X = inputTensor
//      Z = WX-B
//      Y = activate(Z)
//      outputTensor = Y
//
class CDenseNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver);
    int learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx, PTensor* pInputDeviation);
    int learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& deltaTener, PTensor* pInputDeviation);

public://Factory
    static int createNetwork(int nCells, SNeuralNetwork::EACTIVATION eActivation, SNeuralNetwork& spNetwork);

private:
    int initWeights(const PTensor& inputTensor);

private:
    int m_nCells;
    int m_nInputCells;
    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;
    CActivator* m_pActivator;
    static SCtx sCtx;
    
public:
    CDenseNetwork() {
        m_nInputCells = 0;
    }

};

#endif//__SimpleWork_NN_CDenseNetwork_H__
