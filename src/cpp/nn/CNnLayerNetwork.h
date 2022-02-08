#ifndef __SimpleWork_NN_CNnLayerNetwork_H__
#define __SimpleWork_NN_CNnLayerNetwork_H__

#include "nn.h"
#include "COptimizer.h"
#include <string>

using namespace sw;
using namespace std;

class PLayerContext;
class CNnLayerNetwork : public CObject, public INnNetwork, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    //
    // 计算函数
    //
    typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "LayerNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.LayerNetwork"; }
    static int createNetwork(const SNnUnit& spUnit, const SDimension& spInDimVector, SNnNetwork& spNet);

public://INnLayerNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private:
    template<typename Q>
    int evalT(const STensor& spBatchIn, STensor& spBatchOut);

    template<typename Q>
    int learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private:
    SNnUnit m_spUnit;
    SDimension m_spInDimension;
    string m_strOptimizer;

    bool m_bInitialized;
    CTaker<PLayerContext*> m_spContext;

    //单元求解后的变量
    int m_nInputTensorSize;
    int m_nOutputTensorSize;
    SDimension m_spOutDimension;

    int m_nBatchs;
    int m_nBatchInSize;
    unsigned int m_idType;
    SOptimizer m_spOptimizer;

    STensor m_spOpSolveBuffer;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    int m_nBatchOutVer;

public:
    int initNetwork();
    int prepareNetwork(const STensor& spBatchIn);
    void releaseCtx();

    CNnLayerNetwork() {
        m_bInitialized = false;
        m_nBatchInSize = 0;
        m_idType = 0;
    }
    ~CNnLayerNetwork() {
        releaseCtx();
    }
};

#endif//__SimpleWork_NN_CNnLayerNetwork_H__
