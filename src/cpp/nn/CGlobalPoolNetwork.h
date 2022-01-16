#ifndef __SimpleWork_NN_CGlobalPoolNetwork_H__
#define __SimpleWork_NN_CGlobalPoolNetwork_H__

#include "nn.h"
#include "CActivator.h"
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
class CGlobalPoolNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);

private://IIoArchivable
    int getVer() { return 220116; }
    const char* getName() { return "GlobalPoolNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GlobalPoolNetwork"; }
    static int createNetwork(const char* szMode, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& spBatchIn);

private:
    //基础参数
    string m_strActivator;
    string m_strMode;

    //缓存参数
    int m_nInputSize;
    unsigned int m_idDataType;
    int m_nBatchs;
    int m_nLayers;
    int m_nLayerSize;
    CActivator* m_pActivator;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;

public:
    CGlobalPoolNetwork() {
        m_nBatchs = 0;
        //激活函数未初始化
        m_pActivator = nullptr;
    }
};

#endif//__SimpleWork_NN_CGlobalPoolNetwork_H__
