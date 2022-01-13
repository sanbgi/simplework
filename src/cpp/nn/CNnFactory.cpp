
#include "nn.h"
#include "CNnNetwork.h"
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

class CNnFactory : public CObject, public INnFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createDense(int nCells, const char* szActivator, SNnNetwork& spNetwork) {
        return CDenseNetwork::createNetwork(nCells, szActivator, spNetwork);
    }

    int createTwoPoleDense(int nCells, const char* szActivator, SNnNetwork& spNetwork) {
        return CTwoPoleDenseNetwork::createNetwork(nCells, szActivator, spNetwork);
    }

    int createConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNnNetwork& spNetwork) {
        return CConvolutionNetwork::createNetwork(nWidth, nHeight, nConv, szActivator, spNetwork);
    }

    int createRotConvolution(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork) {
        return CRotConvNetwork::createNetwork(nWidth, nHeight, nConv, dWidthRotAngle, dHeightRotAngle, szActivator, spNetwork);
    }

    int createTwoPoleConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNnNetwork& spNetwork) {
        return CTwoPoleConvNetwork::createNetwork(nWidth, nHeight, nConv, szActivator, spNetwork);
    }

    int createSequence(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
        return CSequenceNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createParallel(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
        return CParallelNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNnNetwork& spNetwork) {
        return CPoolNetwork::createNetwork(nWidth, nHeight, nStrideWidth, nStrideHeight, spNetwork);
    }

    int readIdxFile(const char* szFileName, STensor& spData) {
        return CIdxFileReader::readFile(szFileName, spData);
    }

    int openIdxFileReader(const char* szFileName, SNnPipe& spPipe) {
        return CIdxFileReader::createReader(szFileName, spPipe);
    }

    int createNormallizePipe(SNnPipe& spPipe) {
        return CNormalizePipe::createPipe(spPipe);
    }

    int normalizeTensor(const STensor& spIn, STensor& spOut) {
        return CTensorSolver::normalize(spIn, spOut);
    }

    int classifyTensor(int nClassify, const STensor& spIn, STensor& spOut) {
        return CTensorSolver::classify(nClassify, spIn, spOut);
    }

    int saveNetwork(const char* szFileName, const SNnNetwork& spNet) {
        return CNnNetwork::saveNetwork(szFileName, spNet);
    }

    int loadNetwork(const char* szFileName, SNnNetwork& spNet) {
        return CNnNetwork::loadNetwork(szFileName, spNet);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnFactory, SNnFactory::__getClassKey())