#ifndef __SimpleWork_NN_CDenseNetwork_H__
#define __SimpleWork_NN_CDenseNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include <string>

using namespace sw;
using namespace std;

//
//  网络定义：
//      X = inputTensor
//      Z = WX-B
//      Y = activate(Z)
//      outputTensor = Y
//
class CDenseNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);

private://IIoArchivable
    int getVer() { return 220112; }
    const char* getName() { return "DenseNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseNetwork"; }
    static int createNetwork(int nCells, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& spBatchIn);

private:
    //基础参数
    int m_nCells;
    string m_strActivator;
    string m_strOptimizer;

    //运行参数
    int m_nInputCells;
    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;

    //缓存参数
    int m_nBatchs;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spOutDimVector;

    //环境变量
    static SCtx sCtx;
    
public:
    CDenseNetwork() {
        m_nCells = 0;
        m_nInputCells = 0;
        m_nBatchs = 0;
    }
};

#endif//__SimpleWork_NN_CDenseNetwork_H__
