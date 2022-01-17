#ifndef __SimpleWork_NN_CSequenceNetwork_H__
#define __SimpleWork_NN_CSequenceNetwork_H__

#include <vector>
#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CSequenceNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "SequenceNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.SequenceNetwork"; }
    static int createNetwork(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork);

public:
    std::vector<SNnNetwork> m_arrNetworks;
};

#endif//__SimpleWork_NN_CSequenceNetwork_H__
