#ifndef __SimpleWork_NeuralNetwork_h__
#define __SimpleWork_NeuralNetwork_h__

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NeuralNetwork)

public:
    //
    // 神经网络学习需要的上下文，提供给神经网络学习需要的反馈信息，以及接收神经网络对输入的期待偏量
    //
    struct ILearnCtx {
        //
        // 前向计算，同时查询输出量与目标量的偏差量(注意不是偏导数)
        //
        virtual int forward(const PTensor& outputTensor, IVisitor<const PTensor&>* pDeltaReceiver) = 0;

        //
        // 反向梯度，返回期望输入量对目标的偏差量(注意不是偏导数)
        //
        virtual int backward(const PTensor& inputDelta) { return SError::ERRORTYPE_SUCCESS; }
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INeuralNetwork", 211223)

        //
        //  计算
        //      @inputTensor，输入张量
        //      @pReceiver，输出数据接收回调接口
        //
        virtual int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) = 0;

        //
        //  学习
        //      @inputTensor，输入张量
        //      @expectTensor，期望的输出张量
        //
        virtual int learn(const PTensor& inputTensor, const PTensor& expectTensor) = 0;

        //
        //  学习(可实现级联)
        //      @inputTensor，输入数据
        //      @pLearnCtx，计算所需要的上下文
        //
        virtual int learn(const PTensor& inputTensor, ILearnCtx* pLearnCtx) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(NeuralNetworkFactory, "sw.nn.NeuralNetworkFactory")
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INeuralNetworkFactory", 211223)

            //
            //  创建池化神经网络
            //
            virtual int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建直连神经网络
            //
            virtual int createDense(int nCells, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建卷积神经网络
            //
            virtual int createConvolution(int nWidth, int nHeight, int nConv, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建顺序处理神经网络
            //
            virtual int createSequence(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) = 0;


        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetworkFactory)

public:
    static SNeuralNetworkFactory& getFactory() {
        static SNeuralNetworkFactory g_factory = SObject::createObject<SNeuralNetworkFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetwork)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NeuralNetwork_h__