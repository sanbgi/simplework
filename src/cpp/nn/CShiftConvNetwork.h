#ifndef __SimpleWork_NN_CShiftConvNetwork_H__
#define __SimpleWork_NN_CShiftConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include "CSize.h"
#include <string>

using namespace sw;
using namespace std;
//
//
// 轮换卷积网络（模拟人脑视觉V1皮层对朝向等识别的模式）
//
//      网络输出层为 ：nLayers
//      每一层使用的轮换卷积核数为：nShiftConvs
//
// 具体模式：
//
//      每走一步（比如：x方向），则换一个卷积核，直到卷积核用完，再从第一个卷积核开始，如
// 果nShiftConvs = 1, 则退化为普通的卷积网络。
//
//
class CShiftConvNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);

private://IIoArchivable
    int getVer() { return 220114; }
    const char* getName() { return "ShiftConvNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private:
    template<typename Q> int evalT(const STensor& spBatchIn, STensor& spBatchOut);
    template<typename Q> int learnT(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nWeights, int nConvs);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ShiftConvNetwork"; }
    static int createNetwork(int nWidth, int nHeight, int nLayers, int nLayerShiftConvs, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& inputTensor);

private:
    //基础参数
    int m_nLayers;
    CBatchSize3D m_sizeConv;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dDropoutRate;
    string m_strActivator;
    string m_strOptimizer;
    string m_strPadding;

    //网络参数
    int m_nInputLayers;
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
    CShiftConvNetwork(){
        //卷积核数
        m_sizeConv.batch = 0;
        //丢弃率
        m_dDropoutRate = 0;
        //卷积层数
        m_nInputLayers = 0;
        //输入尺寸
        m_nInputSize = 0;
        //
        m_idDataType = 0;
    }
};

#endif//__SimpleWork_NN_CShiftConvNetwork_H__
