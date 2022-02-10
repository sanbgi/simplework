
#include "nn.h"
#include "CNnNetwork.h"
#include "CIdxFileReader.h"
#include "CNormallizePipe.h"
#include "CTensorSolver.h"
#include "units/CSequenceUnit.h"
#include "CNnLayerNetwork.h"

using namespace sw;

class CNnFactory : public CObject, public INnFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
   /*
    int createDense(int nCells, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork) {
        return CDenseNetwork::createNetwork(nCells, dDropoutRate, szActivator, spNetwork);
    }
    int createRnn(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivitor, SNnNetwork& spNetwork) {
        return CRnnNetwork::createNetwork(nCells, bKeepGroup, dDropoutRate, szActivitor, spNetwork);
    }
 

    int createConvolution(int nWidth, int nHeight, int nLayers, int nShiftConvs, int nStride, const char* szPadding, const char* szActivator, SNnNetwork& spNetwork) {
        return CConvolutionNetwork::createNetwork(nWidth, nHeight, nLayers, nShiftConvs, nStride, szPadding, szActivator, spNetwork);
    }

    int createRotConvolution(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork) {
        return CRotConvNetwork::createNetwork(nWidth, nHeight, nConv, dWidthRotAngle, dHeightRotAngle, szActivator, spNetwork);
    }

    int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNnNetwork& spNetwork) {
        return CPoolNetwork::createNetwork(nWidth, nHeight, nStrideWidth, nStrideHeight, spNetwork);
    }

    int createGlobalPool(const char* szMode, const char* szActivitor, SNnNetwork& spNetwork) {
        return CGlobalPoolNetwork::createNetwork(szMode, szActivitor, spNetwork);
    }

    int createGru(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivitor, SNnNetwork& spNetwork) {
        return CGruNetwork::createNetwork(nCells, bKeepGroup, dDropoutRate, szActivitor, spNetwork);
    }

    int createSequence(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
        return CSequenceNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }

    int createParallel(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
        return CParallelNetwork::createNetwork(nNetworks, pNetworks, spNetwork);
    }
    */


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
    
    int createSequenceUnit(int nUnits, const SNnUnit pUnits[], SNnUnit& spUnit) {
        return CSequenceUnit::createUnit(nUnits, pUnits, spUnit);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnFactory, SNnFactory::__getClassKey())