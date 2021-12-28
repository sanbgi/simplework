#ifndef __SimpleWork_NN_CDenseNetwork_H__
#define __SimpleWork_NN_CDenseNetwork_H__

#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CDenseNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int getCellNumber();
    int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver);
    int learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);
    int learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& deltaTener, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx);

public://Factory
    static int createNetwork(int nCells, SNeuralNetwork& spNetwork);

private:
    int initWeights(int nInputCells);
    double activate(double v);
    double deactivate(double dOutput, double dDelta);

private:
    int m_nCells;
    int m_nInputCells;
    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;

public:
    CDenseNetwork() {
        m_nInputCells = 0;
    }

};

#endif//__SimpleWork_NN_CDenseNetwork_H__
