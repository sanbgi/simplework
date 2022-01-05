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
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork);

public:
    std::vector<SNeuralNetwork> m_arrNetworks;
    static SCtx sCtx;
};

#endif//__SimpleWork_NN_CSequenceNetwork_H__
