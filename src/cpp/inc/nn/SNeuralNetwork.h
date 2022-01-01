#ifndef __SimpleWork_NeuralNetwork_h__
#define __SimpleWork_NeuralNetwork_h__

#include "nn.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NeuralNetwork)

public:
    //
    // 神经网络学习需要的上下文，提供给神经网络学习需要的目标偏差值
    //
    struct ILearnCtx {
        //
        // 获得输出的偏差量，在前向计算中，Ctx可以在这个回调函数中，将输出作为下一级神经网络的输入，进行学习
        //
        virtual int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeviation) = 0;
    };

    //
    // 激活函数
    //
    enum EACTIVATION {
        ACTIVATION_ReLU,
        ACTIVATION_Softmax
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INeuralNetwork", 211223)

        //
        //  计算
        //      @inputTensor，输入张量
        //      @pReceiver，输出数据接收回调接口
        //
        virtual int eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) = 0;

        //
        //  学习(可实现级联)
        //      @inputTensor，输入数据
        //      @pLearnCtx，计算所需要的上下文
        //      @pInputDeviation，输入量的偏差量（希望输入能够调整）
        //
        virtual int learn(const PTensor& inputTensor, ILearnCtx* pLearnCtx, PTensor* pInputDeviation) = 0;

        //
        //  学习
        //      @inputTensor，输入张量
        //      @expectTensor，期望的输出张量
        //
        int learn(const PTensor& inputTensor, const PTensor& expectTensor) {
            struct CLearnCtx : ILearnCtx {
                int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeviation) {
                    for(int i=0; i<outputTensor.nData; i++) {
                        outputDeviation.pDoubleArray[i] = pExpectTensor->pDoubleArray[i] - outputTensor.pDoubleArray[i];
                    }
                    return SError::ERRORTYPE_SUCCESS;
                }
                const PTensor* pExpectTensor;
            }ctx;
            ctx.pExpectTensor = &expectTensor;
            return learn(inputTensor, &ctx, nullptr);
        }

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
            virtual int createDense(int nCells, EACTIVATION eActivation, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建卷积神经网络
            //
            virtual int createConvolution(int nWidth, int nHeight, int nConv, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建顺序处理神经网络
            //
            virtual int createSequence(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) = 0;

            //
            //  读取整个IDX格式的文件，格式参考：http://yann.lecun.com/exdb/mnist/
            //
            virtual int readIdxFile(const char* szFileName, SData& spData) = 0;

            //
            //  打开IDX格式的文件读取器，格式参考：http://yann.lecun.com/exdb/mnist/
            //
            virtual int openIdxFileReader( const char* szFileName, SPipe& spPipe) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetworkFactory)

public:
    static SNeuralNetworkFactory& getFactory() {
        static SNeuralNetworkFactory g_factory = SObject::createObject<SNeuralNetworkFactory>();
        return g_factory;
    }

    static SNeuralNetwork createDense(int nCells, EACTIVATION eActivation=ACTIVATION_ReLU) {
        SNeuralNetwork nn;
        getFactory()->createDense(nCells,eActivation,nn);
        return nn;
    }

    static SNeuralNetwork createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight) {
        SNeuralNetwork nn;
        getFactory()->createPool(nWidth, nHeight, nStrideWidth, nStrideHeight, nn);
        return nn;
    }

    static SNeuralNetwork createConv(int nWidth, int nHeight, int nConv) {
        SNeuralNetwork nn;
        getFactory()->createConvolution(nWidth, nHeight, nConv, nn);
        return nn;
    }

    static SNeuralNetwork createSequence(int nNetworks, SNeuralNetwork* pNetworks) {
        SNeuralNetwork nn;
        getFactory()->createSequence(nNetworks, pNetworks, nn);
        return nn;
    }

    static SData loadIdxFile(const char* szFilename) {
        SData data;
        getFactory()->readIdxFile(szFilename, data);
        return data;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetwork)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NeuralNetwork_h__