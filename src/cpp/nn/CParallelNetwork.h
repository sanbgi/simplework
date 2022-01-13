#ifndef __SimpleWork_NN_CParallelNetwork_H__
#define __SimpleWork_NN_CParallelNetwork_H__

#include <vector>
#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

//
// 并行计算神经网络，其中，每一个子网络的最后一个维度是图层深度，所以，所有节点输出合并是在最后一个维度上合并
//
class CParallelNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);


private://IIoArchivable
    int getVer() { return 220112; }
    const char* getName() { return "ParrallelNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ParrallelNetwork"; }
    static int createNetwork(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork);

public:
    std::vector<SNnNetwork> m_arrNetworks;
    std::vector<STensor> m_arrOutTensors;
    int m_nOutLayers;
    int m_nOutLayerSize;
    STensor m_spBatchOut;
    STensor m_spBatchIn;
    STensor m_spInDeviation;
    static SCtx sCtx;
};

#endif//__SimpleWork_NN_CParallelNetwork_H__
