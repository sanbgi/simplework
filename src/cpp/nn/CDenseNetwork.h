#ifndef __SimpleWork_NN_CDenseNetwork_H__
#define __SimpleWork_NN_CDenseNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"

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
class CDenseNetwork : public CObject, public INnNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nCells, const char* szActivator, SNnNetwork& spNetwork);

private:
    int initNetwork(const STensor& spDimTensor);

private:
    STensor m_spInTensor;
    STensor m_spOutTensor;
    STensor m_spOutDimVector;
    int m_nCells;
    int m_nInputTensor;
    int m_nInputCells;
    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;
    static SCtx sCtx;
    
public:
    CDenseNetwork() {
        m_nInputCells = 0;
    }
};

#endif//__SimpleWork_NN_CDenseNetwork_H__
