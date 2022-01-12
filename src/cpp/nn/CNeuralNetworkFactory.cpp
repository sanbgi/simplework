
#include "nn.h"
#include "CDenseNetwork.h"
#include "CTwoPoleDenseNetwork.h"
#include "CConvolutionNetwork.h"
#include "CRotConvNetwork.h"
#include "CTwoPoleConvNetwork.h"
#include "CSequenceNetwork.h"
#include "CParallelNetwork.h"
#include "CPoolNetwork.h"
#include "CIdxFileReader.h"
#include "CNormallizePipe.h"
#include "CTensorSolver.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CNeuralNetworkFactory : public CObject, public SNeuralNetwork::INeuralNetworkFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SNeuralNetwork::INeuralNetworkFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createDense(int nCells, const char* szActivator, SNeuralNetwork& spNetwork) {
        return CDenseNetwork::createNetwork(nCells, szActivator, spNetwork);
    }

    int createTwoPoleDense(int nCells, const char* szActivator, SNeuralNetwork& spNetwork) {
        return CTwoPoleDenseNetwork::createNetwork(nCells, szActivator, spNetwork);
    }

    int createConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNeuralNetwork& spNetwork) {
        return CConvolutionNetwork::createNetwork(nWidth, nHeight, nConv, szActivator, spNetwork);
    }

    int createRotConvolution(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNeuralNetwork& spNetwork) {
        return CRotConvNetwork::createNetwork(nWidth, nHeight, nConv, dWidthRotAngle, dHeightRotAngle, szActivator, spNetwork);
    }

    int createTwoPoleConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNeuralNetwork& spNetwork) {
        return CTwoPoleConvNetwork::createNetwork(nWidth, nHeight, nConv, szActivator, spNetwork);
    }

    int createSequence(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
        return CSequenceNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createParallel(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
        return CParallelNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) {
        return CPoolNetwork::createNetwork(nWidth, nHeight, nStrideWidth, nStrideHeight, spNetwork);
    }

    int readIdxFile(const char* szFileName, STensor& spData) {
        return CIdxFileReader::readFile(szFileName, spData);
    }

    int openIdxFileReader(const char* szFileName, SNeuralPipe& spPipe) {
        return CIdxFileReader::createReader(szFileName, spPipe);
    }

    int createNormallizePipe(SNeuralPipe& spPipe) {
        return CNormalizePipe::createPipe(spPipe);
    }

    int normalizeTensor(const STensor& spIn, STensor& spOut) {
        return CTensorSolver::normalize(spIn, spOut);
    }

    int classifyTensor(int nClassify, const STensor& spIn, STensor& spOut) {
        return CTensorSolver::classify(nClassify, spIn, spOut);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNeuralNetworkFactory, SNeuralNetwork::SNeuralNetworkFactory::__getClassKey())