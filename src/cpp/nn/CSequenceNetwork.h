#ifndef __SimpleWork_NN_CSequenceNetwork_H__
#define __SimpleWork_NN_CSequenceNetwork_H__

#include <vector>
#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CSequenceNetwork : public CObject, public INeuralNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INeuralNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INeuralNetwork
    int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver);
    int learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx);
    int learn(const PTensor& inputTensor, const PTensor& expectTensor);
    
public://Factory
    static int createNetwork(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork);

public:
    std::vector<SNeuralNetwork> m_arrNetworks;
};

#endif//__SimpleWork_NN_CSequenceNetwork_H__
