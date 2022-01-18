#ifndef __SimpleWork_NN_CRnnNetwork_H__
#define __SimpleWork_NN_CRnnNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include <string>

using namespace sw;
using namespace std;

//
//  全连接神经网络，注意数据格式：
//
//      输入数据：nBatchs(第一个维度) * nGroups * nCells(余下所有维度)
//      输出数据
//          保留组结构：nBatchs(第一个维度) * nGroups * nCells(第二个维度)
//          不保留组结构：nBatchs * nCells
//
//  说明：
//      循环网络的输入序列数量为：nBatchs * nGroups，每一个数据为一个nCells张
//  量（也可能是多维的）
//
class CRnnNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);

private://IIoArchivable
    int getClassVer() { return 220118; }
    const char* getClassName() { return "RnnNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nUWeights, int nVWeights, int nConvs);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RnnNetwork"; }
    static int createNetwork(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& spBatchIn);

private:
    //基础参数
    //神经元数量
    int m_nCells;
    //
    //  是否保留输入的数据组结构
    //      保留，输出数据结构为: nBatchs * nGroups * nCells
    //      不保留，输出数据结构为：nBatchs * nCells
    //
    bool m_bKeepGroup;
    double m_dDropoutRate;
    string m_strActivator;
    string m_strOptimizer;

    //运行参数
    int m_nInputCells;
    unsigned int m_idDataType;
    CTaker<char*> m_spUWeights;
    CTaker<char*> m_spVWeights;
    CTaker<char*> m_spState;
    CTaker<char*> m_spBais;

    //缓存参数
    int m_nInputSize;
    int m_nBatchs;
    int m_nGroups;
    int m_nInVer;
    int m_nOutVer;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;
    STensor m_spBatchIn;
    STensor m_spInternalBatchOut;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;
    CTaker<bool*> m_spDropout;
    int m_nEvalDropout;

public:
    CRnnNetwork() {
        m_nCells = 0;
        m_bKeepGroup = false;
        m_nInputCells = 0;
        m_nBatchs = 0;
        m_nInVer = 0;
        m_nOutVer = 0;
        m_nEvalDropout = 0;
        //激活函数未初始化
        m_pActivator = nullptr;
    }
};

#endif//__SimpleWork_NN_CRnnNetwork_H__
