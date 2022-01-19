#ifndef __SimpleWork_NN_CGruNetwork_H__
#define __SimpleWork_NN_CGruNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include <string>

using namespace sw;
using namespace std;

//
//  GRU循环神经网络（最简单版本）
//      
//      参考：  https://www.jianshu.com/p/9dc9f41f0b29
//             https://blog.csdn.net/weixin_37621229/article/details/80245449
//
class CGruNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);

private://IIoArchivable
    int getClassVer() { return 220118; }
    const char* getClassName() { return "GruNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nWeights, int nConvs);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GruNetwork"; }
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
    CTaker<char*> m_spWeightsZ;
    CTaker<char*> m_spWeightsR;
    CTaker<char*> m_spWeightsH;
    CTaker<char*> m_spState;

    //缓存参数
    int m_nInputSize;
    int m_nBatchs;
    int m_nGroups;
    int m_nInVer;
    int m_nOutVer;
    CActivator* m_pSigmodActivator;
    CActivator* m_pTanhActivator;
    SOptimizer m_spOptimizer;
    SVectorSolver m_spSolver;
    STensor m_spBatchIn;
    STensor m_spInternalBatchOut;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;
    CTaker<bool*> m_spDropout;
    int m_nEvalDropout;

public:
    CGruNetwork() {
        m_nCells = 0;
        m_bKeepGroup = false;
        m_nInputCells = 0;
        m_nBatchs = 0;
        m_nInVer = 0;
        m_nOutVer = 0;
        m_nEvalDropout = 0;
        //激活函数未初始化
        m_pSigmodActivator = m_pTanhActivator = nullptr;
    }
};

#endif//__SimpleWork_NN_CGruNetwork_H__
