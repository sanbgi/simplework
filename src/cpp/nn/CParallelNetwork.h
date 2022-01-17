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
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ParrallelNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ParrallelNetwork"; }
    static int createNetwork(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork);
    
private:
    int prepareNetwork(const STensor& spBatchIn);

public:
    std::vector<SNnNetwork> m_arrNetworks;
    std::vector<STensor> m_arrOutTensors;
    int m_nOutLayers;
    int m_nOutLayerSize;
    int m_nInVer;
    int m_nOutVer;
    unsigned int m_idDataType;
    STensor m_spBatchOut;
    STensor m_spBatchIn;
    STensor m_spInDeviation;

public:
    CParallelNetwork() {
        m_nInVer = m_nOutVer = 0;
        m_idDataType = 0;
    }
};

#endif//__SimpleWork_NN_CParallelNetwork_H__
