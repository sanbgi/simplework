#ifndef __SimpleWork_NN_CNnLayerNetwork_H__
#define __SimpleWork_NN_CNnLayerNetwork_H__

#include "nn.h"
#include "COptimizer.h"
#include <vector>
#include <string>

using namespace sw;
using namespace std;

class CNnVariable;
class PSolveContext;
class PLayerContext;
class CNnLayerNetwork : public CObject, public INnNetwork{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    //
    // 计算函数
    //
    typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

public:
    static int createNetwork(int nLayers, const SNnLayer pLayers[], const SDimension& spInDimension, SNnNetwork& spNet);

public://INnLayerNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private:
    template<typename Q>
    int evalT(const STensor& spBatchIn, STensor& spBatchOut);

    template<typename Q>
    int learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private:
    vector<SNnLayer> m_arrLayers;
    SDimension m_spInDimension;
    string m_strOptimizer;

    bool m_bInitialized;
    vector<PLayerContext*> m_arrLayerCtx;

    //单元求解后的变量
    int m_nInputTensorSize;
    int m_nOutputTensorSize;
    SDimension m_spOutDimension;

    //
    //  注意其中区别
    //      m_nOpSize 是所有运算过程中输出的大小（包括nBatchs)
    ///     m_nStateSize 和 m_nWeightSize 只是计算单元中状态和权重大小
    //
    int m_nOpSize;
    int m_nStateSize;
    int m_nWeightSize;

    int m_nBatchIns;
    int m_nBatchInSize;
    unsigned int m_idType;
    SOptimizer m_spOptimizer;

    STensor m_spOpSolveBuffer;
    STensor m_spDeviaBuffer;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;
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
