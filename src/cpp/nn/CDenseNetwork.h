#ifndef __SimpleWork_NN_CDenseNetwork_H__
#define __SimpleWork_NN_CDenseNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include <string>

using namespace sw;
using namespace std;

//
//  全连接神经网络，注意数据格式：
//
//      输入数据：nBatchs(第一个维度) * nCells(余下所有维度)
//      输出数据：nBatchs(第一个维度) * nCells(第二个维度)
//
//  比如：输入维度[10, 3, 1]，表示一共有十个输入数据，每一个数据数据包含3 X 1个输入神经元
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
    int getClassVer() { return 220112; }
    const char* getClassName() { return "DenseNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nWeights, int nConvs);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseNetwork"; }
    static int createNetwork(int nCells, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& spBatchIn);

private:
    //基础参数
    int m_nCells;
    double m_dDropoutRate;
    string m_strActivator;
    string m_strOptimizer;

    //运行参数
    int m_nInputCells;
    unsigned int m_idDataType;
    CTaker<char*> m_spWeights;
    CTaker<char*> m_spBais;

    //缓存参数
    int m_nBatchs;
    int m_nInVer;
    int m_nOutVer;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;
    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;
    CTaker<bool*> m_spDropout;
    int m_nEvalDropout;

public:
    CDenseNetwork() {
        m_nCells = 0;
        m_nInputCells = 0;
        m_nBatchs = 0;
        m_nInVer = 0;
        m_nOutVer = 0;
        m_nEvalDropout = 0;
        //激活函数未初始化
        m_pActivator = nullptr;
    }
};

#endif//__SimpleWork_NN_CDenseNetwork_H__
