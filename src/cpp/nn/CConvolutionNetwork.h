#ifndef __SimpleWork_NN_CConvNetwork_H__
#define __SimpleWork_NN_CConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include "CSize.h"
#include <string>

using namespace sw;
using namespace std;

class CConvolutionNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

private://IIoArchivable
    int getVer() { return 220112; }
    const char* getName() { return "ConvNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nWeights, int nConvs);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ConvNetwork"; }
    static int createNetwork(int nWidth, int nHeight, int nConvs, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& inputTensor);

private:
    //基础参数
    CBatchSize3D m_sizeConv;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dDropoutRate;
    string m_strActivator;
    string m_strOptimizer;
    string m_strPadding;

    //网络参数
    int m_nLayers;
    unsigned int m_idDataType;
    CTaker<char*> m_spWeights;
    CTaker<char*> m_spBais;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;

    //缓存参数
    int m_nInputSize;

    //输入、输出、卷积尺寸
    CBatchSize3D m_sizeIn;
    CBatchSize3D m_sizeOut;

    //输入、输出、卷积步长
    CBatchSize2D m_stepInMove;
    CBatchSize2D m_stepInConv;
    CBatchSize2D m_stepOut;
    CBatchSize2D m_stepConv;

    STensor m_spBatchIn;
    STensor m_spBatchOut;
    STensor m_spBatchInDeviation;

public:
    CConvolutionNetwork(){
        //卷积核数
        m_sizeConv.batch = 0;
        //丢弃率
        m_dDropoutRate = 0;
        //卷积层数
        m_nLayers = 0;
        //输入尺寸
        m_nInputSize = 0;
        //
        m_idDataType = 0;
    }
};

#endif//__SimpleWork_NN_CConvNetwork_H__
