
#include "nn.h"
#include "CDenseNetwork.h"
#include "CConvolutionNetwork.h"
#include "CSequenceNetwork.h"
#include "CPoolNetwork.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CNeuralNetworkFactory : public CObject, public SNeuralNetwork::INeuralNetworkFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SNeuralNetwork::INeuralNetworkFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createDense(int nCells, SNeuralNetwork& spNetwork) {
        return CDenseNetwork::createNetwork(nCells, spNetwork);
    }

    int createConvolution(int nWidth, int nHeight, int nConv, SNeuralNetwork& spNetwork) {
        return CConvolutionNetwork::createNetwork(nWidth, nHeight, nConv, spNetwork);
    }

    int createSequence(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
        return CSequenceNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) {
        return CPoolNetwork::createNetwork(nWidth, nHeight, nStrideWidth, nStrideHeight, spNetwork);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CNeuralNetworkFactory, SNeuralNetwork::SNeuralNetworkFactory::getClassKey())