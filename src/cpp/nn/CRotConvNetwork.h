#ifndef __SimpleWork_NN_CRotConvNetwork_H__
#define __SimpleWork_NN_CRotConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include "CSize.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

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
class CRotConvNetwork : public CObject, public INnNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nConvs, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork);

private:
    int initNetwork(const STensor& inputTensor);

private:
    STensor m_spInTensor;
    STensor m_spOutTensor;
    STensor m_spOutDimVector;

    //基础参数
    int m_nPadingWidth;
    int m_nPadingHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dWRotAngle;
    double m_dHRotAngle;

    //数据计算参数
    int m_nInData;

    //输入、输出、卷积尺寸
    CBatchSize3D m_sizeIn;
    CBatchSize3D m_sizeOut;
    CBatchSize3D m_sizeConv;

    //输入、输出、卷积步长
    CBatchSize2D m_stepInMove;
    CBatchSize2D m_stepInConv;
    CBatchSize2D m_stepOut;
    CBatchSize2D m_stepConv;

    CTaker<double*> m_spWeights;
    CTaker<double*> m_spBais;
    CActivator* m_pActivator;
    SOptimizer m_spOptimizer;
};

#endif//__SimpleWork_NN_CRotConvNetwork_H__
