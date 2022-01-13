#ifndef __SimpleWork_NN_CTwoPoleConvNetwork_H__
#define __SimpleWork_NN_CTwoPoleConvNetwork_H__

#include "nn.h"
#include "CActivator.h"
#include "COptimizer.h"
#include "CSize.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CTwoPoleConvNetwork : public CObject, public INnNetwork{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnNetwork
    int eval(const STensor& spInTensor, STensor& spOutTensor);
    int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation);

public://Factory
    static int createNetwork(int nWidth, int nHeight, int nConvs, const char* szActivator, SNnNetwork& spNetwork);

private:
    int prepareNetwork(const STensor& inputTensor);

private:
    STensor m_spBatchInTensor;
    STensor m_spBatchOut;
    STensor m_spOutDimVector;

    //基础参数
    int m_nPadingWidth;
    int m_nPadingHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;

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

#endif//__SimpleWork_NN_CTwoPoleConvNetwork_H__
