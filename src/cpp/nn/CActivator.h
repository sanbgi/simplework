#ifndef __SimpleWork_NN_CActivation_H__
#define __SimpleWork_NN_CActivation_H__

#include <vector>
#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

//
//  我们定义：
//      Z = net(X)          神经网络计算
//      Y = activate(Z)     激活计算结果
//
class CActivator{
public:
    //
    //  激活与求导
    //  @nData 数据数量，代表神经网络输出数据的个数
    //  @pZArray 神经网络初步计算结果（还未通过激活函数计算）
    //  @pYArray 神经网络激活输出结果
    //  @pYDeltaArray 神经网络总体输出离期望的差值
    //  @pDzArray 神经网络学习目标函数对网络输出（未激活）的偏导数，其中目标函数定义为：
    //      对于大部分()：
    //          E = 求和(Delta*Delta)/2
    //      对于Softmax(交叉熵):
    //          E = -求和((Yi-Delta)*log(Yi))
    //
    virtual void activate( int nData, double* pZArray, double* pYArray);
    virtual void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray);

public:
    static CActivator* getActivation(SNeuralNetwork::EACTIVATION eActivation = SNeuralNetwork::ACTIVATION_Default);

public:
    static CActivator* getReLU();
    static CActivator* getLeakyReLU();
    static CActivator* getELU();
    static CActivator* getSigmod();
    static CActivator* getTanh();
    static CActivator* getSoftmax();
    static CActivator* getNeuralActivator();
};

#endif//__SimpleWork_NN_CActivation_H__
