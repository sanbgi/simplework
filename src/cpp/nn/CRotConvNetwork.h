#ifndef __SimpleWork_NN_CRotConvNetwork_H__
#define __SimpleWork_NN_CRotConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include "CSize.h"
#include <string>

using namespace sw;
using namespace std;

//
// 旋转卷积，相当于卷积核在平面每移动一个距离，就旋转一个角度进行卷积。模仿人类大脑
// 皮层的特征识别模式，比如：朝向识别等。
//      
//      在实际计算中，旋转后的卷积核难以计算，所以，实际上是，通过下面公式来
//  进行模拟计算的：
//      
//      conv(a) = conv(0度) * cos(a) * cos(a) + conv(90度) * sin(a) * sin(a);
//
//  疑问：
//      有没有可能其它公式更合适？目前从数据看起来，减少输出层数，并且使用旋转卷积，
//  还不能提高精度，可能与conv(a)的计算偏差大相关，毕竟两个90度卷积的加权结果，未必
//  能表示指定角度的卷积结果。
//
class CRotConvNetwork : public CObject, public INnNetwork, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "RotConvNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RotConvNetwork"; }
    static int createNetwork(int nWidth, int nHeight, int nConvs, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork);

private:
    template<typename Q> int evalT(const STensor& spInTensor, STensor& spOutTensor);
    template<typename Q> int learnT(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);
    template<typename Q> int initWeightT(int nWeights, int nConvs);

private:
    int prepareNetwork(const STensor& inputTensor);

private:
    //基础参数
    CBatchSize3D m_sizeConv;
    double m_dDropoutRate;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dWRotAngle;
    double m_dHRotAngle;
    string m_strActivator;
    string m_strOptimizer;
    string m_strPadding;

    //网络参数
    int m_nLayers;
    int m_nInVer;
    int m_nOutVer;
    unsigned int m_idDataType;
    CTaker<char*> m_spWeights;
    CTaker<char*> m_spBais;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;

    //缓存参数
    int m_nInputSize;
    //填充尺寸
    CRect2D m_padding;
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
    CRotConvNetwork(){
        //卷积核数
        m_sizeConv.batch = 0;
        //
        m_dDropoutRate = 0;
        //卷积层数
        m_nLayers = 0;
        //输入尺寸
        m_nInputSize = 0;
        //激活函数未初始化
        m_pActivator = nullptr;
        m_nInVer = m_nOutVer = 0;
    }
};

#endif//__SimpleWork_NN_CRotConvNetwork_H__
