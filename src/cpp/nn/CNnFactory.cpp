
#include "nn.h"
#include "CNnNetwork.h"
#include "CDenseNetwork.h"
#include "CConvolutionNetwork.h"
#include "CRotConvNetwork.h"
#include "CSequenceNetwork.h"
#include "CParallelNetwork.h"
#include "CPoolNetwork.h"
#include "CGlobalPoolNetwork.h"
#include "CRnnNetwork.h"
#include "CGruNetwork.h"
#include "CIdxFileReader.h"
#include "CNormallizePipe.h"
#include "CTensorSolver.h"
#include "CNnWeight.h"
#include "CNnState.h"
#include "CNnOperator.h"
#include "CDenseUnit.h"
#include "CConvUnit.h"
#include "CPoolUnit.h"
#include "CSequenceUnit.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CNnFactory : public CObject, public INnFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createDense(int nCells, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork) {
        return CDenseNetwork::createNetwork(nCells, dDropoutRate, szActivator, spNetwork);
    }

    int createConvolution(int nWidth, int nHeight, int nLayers, int nShiftConvs, int nStride, const char* szPadding, const char* szActivator, SNnNetwork& spNetwork) {
        return CConvolutionNetwork::createNetwork(nWidth, nHeight, nLayers, nShiftConvs, nStride, szPadding, szActivator, spNetwork);
    }

    int createRotConvolution(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork) {
        return CRotConvNetwork::createNetwork(nWidth, nHeight, nConv, dWidthRotAngle, dHeightRotAngle, szActivator, spNetwork);
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

    int createGlobalPool(const char* szMode, const char* szActivitor, SNnNetwork& spNetwork) {
        return CGlobalPoolNetwork::createNetwork(szMode, szActivitor, spNetwork);
    }

    int createRnn(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivitor, SNnNetwork& spNetwork) {
        return CRnnNetwork::createNetwork(nCells, bKeepGroup, dDropoutRate, szActivitor, spNetwork);
    }

    int createGru(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivitor, SNnNetwork& spNetwork) {
        return CGruNetwork::createNetwork(nCells, bKeepGroup, dDropoutRate, szActivitor, spNetwork);
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
    
    int createWeightVariable(const SDimension& spDimension, SNnVariable& spVar) {
        return CNnWeight::createWeightVariable(spDimension, spVar);
    }

    int createStateVariable(const SDimension& spDimension, SNnVariable& spVar){
        return CNnState::createStateVariable(spDimension, spVar);
    }

    int createWeightVariable(int nDims, const int pDimSizes[], SNnVariable& spVar) {
        return CNnWeight::createWeightVariable(nDims, pDimSizes, spVar);
    }

    int createStateVariable(int nDims, const int pDimSizes[], SNnVariable& spVar){
        return CNnState::createStateVariable(nDims, pDimSizes, spVar);
    }

    int createOpVariable(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
        return CNnOperator::createOperatorVariable(szOp, nInVars, pInVars, spOutVar);
    }

    //int createPoolVariable(const SNnVariable& spIn, int nWidth, int nHeight, int nStride, SNnVariable& spOutVar) {
    //    return CNnOperator::createPoolVariable(nullptr, nWidth, nHeight, nStride, 1, &spIn, spOutVar);
    //}

    //int createConvVariable(const SNnVariable& spIn, int nWidth, int nHeight, int nConvs, const char* szPadding, SNnVariable& spOutVar) {
    //    return CNnOperator::createConvVariable(spIn, nWidth, nHeight, nConvs, szPadding, spOutVar);
    //}

    int createDenseUnit(int nCells, double dDropoutRate, const char* szActivator, SNnUnit& spUnit) {
        return CDenseUnit::createUnit(nCells, dDropoutRate, szActivator, spUnit);
    }
    int createConvUnit(int nWidth, int nHeight, int nLayers, int nShiftConvs, const char* szPaddingMode, const char* szActivator, SNnUnit& spUnit){
        return CConvUnit::createUnit(nWidth, nHeight, nLayers, nShiftConvs, szPaddingMode, szActivator, spUnit);
    }
    int createPoolUnit(int nWidth, int nHeight, int nStride, const char* szPaddingMode, SNnUnit& spUnit) {
        return CPoolUnit::createUnit(nWidth, nHeight, nStride, szPaddingMode, spUnit);
    }
    int createSequenceUnit(int nUnits, const SNnUnit pUnits[], SNnUnit& spUnit) {
        return CSequenceUnit::createUnit(nUnits, pUnits, spUnit);
    }
    int createNetwork(const SNnUnit& spUnit, const SDimension& spInDimVector, SNnNetwork& spNet) {
        return CNnNetwork::createNetwork(spUnit, spInDimVector, spNet);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnFactory, SNnFactory::__getClassKey())